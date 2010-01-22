#ifndef MVV_PLATFORM_MIP_H_
# define MVV_PLATFORM_MIP_H_

# include "mvvMprPlugin.h"
# include <mvvPlatform/resource-typedef.h>

/// order to create MPR from a volume
# define MVV_PLATFORM_ORDER_PRECOMPUTE_MIP   OrderClassId::create( "MVV_PLATFORM_ORDER_PRECOMPUTE_MIP" )
# define MVV_PLATFORM_ORDER_DISPLAY_MIP      OrderClassId::create( "MVV_PLATFORM_ORDER_DISPLAY_MIP" )

namespace mvv
{
namespace platform
{
   namespace impl
   {
      class OrderMipDisplayResult : public OrderResult
      {
      public:
         Sliceuc  slice;
      };

      class OrderMipDisplay : public Order
      {
      public:
         OrderMipDisplay( Slice& slice, const ResourceLut& lut, const nll::core::vector2ui& size ) : Order( MVV_PLATFORM_ORDER_DISPLAY_MIP, Order::Predecessors(), true ), _slice( slice ), _lut( lut ), _size( size )
         {
         }

         virtual OrderResult* _compute()
         {
            OrderMipDisplayResult* result = new OrderMipDisplayResult();
            result->slice = Sliceuc( nll::core::vector3ui( _size[ 0 ], _size[ 1 ], 3 ),
                                        _slice.getAxisX(),
                                        _slice.getAxisY(),
                                        _slice.getOrigin(),
                                        _slice.getSpacing() );

            if ( _slice.size()[ 0 ] != _size[ 0 ] || _slice.size()[ 1 ] != _size[ 1 ] )
            {
               float sx = _size[ 0 ] / _slice.size()[ 0 ] * _slice.getSpacing()[ 0 ];
               float sy = _size[ 1 ] / _slice.size()[ 1 ] * _slice.getSpacing()[ 1 ];
               float s = std::min( sx, sy );
               Slice slice( nll::core::vector3ui( _size[ 0 ], _size[ 1 ], 1 ),
                            _slice.getAxisX(),
                            _slice.getAxisY(),
                            _slice.getOrigin(),
                            nll::core::vector2f( _slice.getSpacing()[ 0 ] / s,
                                                 _slice.getSpacing()[ 1 ] / s ) );
               nll::imaging::resampling<f32, Slice::BilinearInterpolator>( _slice, slice );
               _slice = slice;
            }

            Sliceuc::Storage::DirectionalIterator out = result->slice.getIterator( 0, 0 );
            for ( Slice::Storage::ConstDirectionalIterator it = _slice.getStorage().getIterator( 0, 0, 0 );
                  it != _slice.getStorage().endDirectional();
                  ++it, ++out )
            {
               const float* col = _lut.transform( *it );
               out.pickcol( 0 ) = static_cast<ui8>( col[ 0 ] );
               out.pickcol( 1 ) = static_cast<ui8>( col[ 1 ] );
               out.pickcol( 2 ) = static_cast<ui8>( col[ 2 ] );
            }
            return result;
         }

      private:
         // copy disabled
         OrderMipDisplay();
         OrderMipDisplay& operator=( OrderMipDisplay& );

      private:
         Slice&                        _slice;
         const ResourceLut&            _lut;
         nll::core::vector2ui          _size;
      };

      class OrderMipPrecomputeResult : public OrderResult
      {
      public:
         OrderMipPrecomputeResult()
         {
            progress = 0;
         }

         /// a value from 0 to 1, with 1 job is finished
         float                progress;
         std::vector<Slice>   slices;
      };

      class OrderMipPrecompute : public Order
      {
      public:
         OrderMipPrecompute( const Volume& volume, ui32 nbMips, const nll::core::vector2ui& size, impl::Resource& notifier ) : Order( MVV_PLATFORM_ORDER_PRECOMPUTE_MIP, Order::Predecessors(), true ), _volume( volume ), _nbMips( nbMips ), _size( size ), _notifier( notifier )
         {
         }

         virtual OrderMipPrecomputeResult* getResultInProgress()
         {
            return dynamic_cast<OrderMipPrecomputeResult*> ( getResult() );
         }

         virtual OrderResult* _compute()
         {
            // we preallocate the result so we can directly get the actual job status
            // beware that with preallocated result we can't have predecessors...
            OrderMipPrecomputeResult* result = new OrderMipPrecomputeResult();
            setResult( result );

            nll::imaging::MaximumIntensityProjection<Volume> mip( _volume );
            for ( ui32 n = 0; n < _nbMips; ++n )
            {
               const f32 angle =  static_cast<f32>( 2 * nll::core::PI / _nbMips * n ) + 0.01f;
               result->slices.push_back( mip.getAutoOrientedMip< nll::imaging::InterpolatorTriLinear<Volume> >( angle, std::max( _volume.size()[ 0 ], _volume.size()[ 1 ]), _volume.size()[ 2 ], _volume.getSpacing()[ 0 ], _volume.getSpacing()[ 1 ] ) );
               //result->slices.push_back( mip.getAutoOrientedMip< nll::imaging::InterpolatorTriLinear<Volume> >( angle, _size[ 0 ], _size[ 1 ], minSpacing, minSpacing ) );
               result->progress = static_cast<float>( n + 1 ) / _nbMips;
               _notifier.notify();
            }
            return result;
         }

      private:
         // disabled copies
         OrderMipPrecompute& operator=( const OrderMipPrecompute& );
         OrderMipPrecompute( const OrderMipPrecompute& );

      private:
         const Volume&        _volume;
         const ui32           _nbMips;
         nll::core::vector2ui _size;
         impl::Resource&      _notifier;
      };
   }

   class MVVMPRPLUGIN_API Mip : public EngineOrder
   {
   // output slots
   public:
      /// output public slot holding the resampled & annoted MIP
      ResourceSliceuc   outImage;

   // input slots
   public:
      ResourceVector2ui       size;
      ResourceLut             lut;
      ResourceFloat           anglex;
      ResourceVolumes         volumes;    // we are expecting only 1 volume
      ResourceUi32            fps;

   public:
      Mip( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, ui32 nbMips = 32 ) : EngineOrder( handler, provider, dispatcher ), volumes( storage ), _nbMips( nbMips )
      {
         _interested.insert( MVV_PLATFORM_ORDER_DISPLAY_MIP );
         dispatcher.connect( this );

         fps.setValue( 4 );

         size.connect( this );
         lut.connect( this );
         anglex.connect( this );
         volumes.connect( this );
         fps.connect( this );

         _orderDisplay.unref();
      }

      ~Mip()
      {
         _dispatcher.disconnect( this );
      }

      virtual bool _run()
      {
         //const bool isMipComputed = !_order.isEmpty() || (*_order).getResultInProgress()->progress >= 1;
         if ( volumes.begin() == volumes.end() )
            return false;
         if ( volumes.size() > 1 )
            throw std::exception( "Unexpected number of volumes (only zero or one)" );
         if ( volumes.size() == 1 && ( *volumes.begin() ).getDataPtr() != _cacheOldVolume )
         {
            // if the volume changed, then just clear the order
            _order.unref();
         }

         if ( volumes.size() != 1 )
         {
            // no volume, do nothing
            return true;
         }

         if ( volumes.size() == 1 && _order.isEmpty() )
         {
            // no previously computed MIP, just launch order, clear the frame and display status...
            outImage.getValue().getStorage() = nll::core::Image< nll::ui8>( size.getValue()[ 0 ], size.getValue()[ 1 ], 3, true );
            _cacheOldSize = size.getValue();
            outImage.notify();
            _order = RefcountedTyped<impl::OrderMipPrecompute>( new impl::OrderMipPrecompute( **volumes.begin(), _nbMips, size.getValue(), fps ) );
            _orderProvider.pushOrder( &*_order );
            _displayProgression( 0 );
            _cacheOldVolume = (*volumes.begin()).getDataPtr();
            return true;
         }

         if ( _cacheOldSize != size.getValue() )
         {
            outImage.getValue().getStorage() = nll::core::Image< nll::ui8>( size.getValue()[ 0 ], size.getValue()[ 1 ], 3, true );
            _cacheOldSize = size.getValue();
         }

         impl::OrderMipPrecomputeResult* order = (*_order).getResultInProgress();
         if ( !order )
            return false;
         float progress = order->progress;
         if ( progress >= 1 )
         {
            ui32 f = fps.getValue();
            if ( f && _timer.getCurrentTime() > 1 / f )
            {
               float newAngle = anglex.getValue() + (float)f / 360;
               newAngle = ( newAngle > 2 * nll::core::PI ) ? 0 : newAngle;
               anglex.setValue( newAngle );
            }
            if ( _orderDisplay.isEmpty() )
            {
               // everything else has triggered an update of the precomputed MIP, just update the slice
               ui32 sliceId = static_cast<ui32>( anglex.getValue() / ( nll::core::PI * 2 ) * order->slices.size() );
               _orderDisplay = RefcountedTyped<impl::OrderMipDisplay>( new impl::OrderMipDisplay( order->slices[ sliceId ], lut, size.getValue() ) );
               _orderProvider.pushOrder( &*_orderDisplay );
            }
         } else {
            // just update the progress
            _displayProgression( progress );
            return true;
         }


         return !fps.getValue();
      }

      virtual void consume( Order* o )
      {
         if ( _orderDisplay.isEmpty() )
            return;
         if ( o->getClassId() == MVV_PLATFORM_ORDER_DISPLAY_MIP && o->getId() == (*_orderDisplay).getId() )
         {
            impl::OrderMipDisplayResult* result = dynamic_cast<impl::OrderMipDisplayResult*>( o->getResult() );
            if ( result )
            {
               outImage.setValue( result->slice );
            }
            _orderDisplay.unref();
         }
      }

      virtual const std::set<OrderClassId>& interestedOrder() const
      {
         // no insteresting orders
         return _interested;
      }

   private:
      void _displayProgression( float progress )
      {
         ResourceSliceuc::value_type& slice = outImage.getValue();
         if ( !slice.size()[ 0 ] || !slice.size()[ 1 ] )
         {
            // in case the display is not ready
            return;
         }

         typedef ResourceSliceuc::value_type Slice;
         typedef Slice::value_type           Image;
         Image::DirectionalIterator it = slice.getStorage().getIterator( 0, 0, 0 );

         ui32 nb = static_cast<ui32>( slice.size()[ 1 ] * progress );
         for ( ui32 n = 0; n < nb; ++n )
         {
            *it = 255;
            it.addy();
         }
      }

      void _refreshDisplay()
      {
      }

   private:
      std::set<OrderClassId>  _interested;
      ui32                    _nbMips;
      RefcountedTyped<impl::OrderMipPrecompute> _order;
      RefcountedTyped<impl::OrderMipDisplay>    _orderDisplay;

      Volume*                 _cacheOldVolume;
      nll::core::vector2ui    _cacheOldSize;
      nll::core::Timer        _timer;
   };
}
}

#endif