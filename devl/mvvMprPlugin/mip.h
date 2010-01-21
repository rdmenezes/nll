#ifndef MVV_PLATFORM_MIP_H_
# define MVV_PLATFORM_MIP_H_

# include "mvvMprPlugin.h"
# include <mvvPlatform/resource-typedef.h>

/// order to create MPR from a volume
# define MVV_PLATFORM_ORDER_PRECOMPUTE_MIP  OrderClassId::create( "MVV_PLATFORM_ORDER_CREATE_SLICE" )

namespace mvv
{
namespace platform
{
   namespace impl
   {
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
         OrderMipPrecompute( const Volume& volume, ui32 nbMips, const nll::core::vector2ui& size ) : Order( MVV_PLATFORM_ORDER_PRECOMPUTE_MIP, Order::Predecessors(), true ), _volume( volume ), _nbMips( nbMips ), _size( size )
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

            float sx = _volume.getSpacing()[ 0 ] * _volume.size()[ 0 ];
            float sy = _volume.getSpacing()[ 0 ] * _volume.size()[ 0 ];
            float sz = _volume.getSpacing()[ 0 ] * _volume.size()[ 0 ];
            float minSpacing = 1 / std::min( _size[ 0 ] / std::max( sx, sy ), _size[ 1 ] / sz );

            nll::imaging::MaximumIntensityProjection<Volume> mip( _volume );
            for ( ui32 n = 0; n < _nbMips; ++n )
            {
               const f32 angle =  static_cast<f32>( 2 * nll::core::PI / _nbMips * n );
               result->slices.push_back( mip.getAutoOrientedMip< nll::imaging::InterpolatorTriLinear<Volume> >( angle, _size[ 0 ], _size[ 1 ], minSpacing, minSpacing ) );
               result->progress = static_cast<float>( n ) / _nbMips;
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
      Mip( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, ui32 nbMips = 30 ) : EngineOrder( handler, provider, dispatcher ), volumes( storage ), _nbMips( nbMips )
      {
         _dispatcher.connect( this );
         _interested.insert( MVV_PLATFORM_ORDER_PRECOMPUTE_MIP );

         size.connect( this );
         lut.connect( this );
         anglex.connect( this );
         volumes.connect( this );
         fps.connect( this );
      }

      ~Mip()
      {
         _dispatcher.disconnect( this );
      }

      virtual bool _run()
      {
         const bool isMipComputed = !_order.isEmpty() || (*_order).getResultInProgress()->progress >= 1;

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
            outImage.getValue().getStorage() = nll::core::Image< nll::ui8>( size.getValue()[ 0 ], size.getValue()[ 1 ], 3, !isMipComputed );
            outImage.notify();
            _order = RefcountedTyped<impl::OrderMipPrecompute>( new impl::OrderMipPrecompute( **volumes.begin(), _nbMips, size.getValue() ) );
            _orderProvider.pushOrder( &*_order );
            _displayProgression( 0 );
            _cacheOldVolume = (*volumes.begin()).getDataPtr();
            return false;
         }

         float progress = (*_order).getResultInProgress()->progress;
         if ( progress >= 1 )
         {
            // everything else has triggered an update of the precomputed MIP, just update the slice

         } else {
            // just update the progress
            _displayProgression( progress );
         }


         return !fps.getValue();
      }

      virtual void consume( Order* )
      {
         // we don't handle any orders
      }

      virtual const std::set<OrderClassId>& interestedOrder() const
      {
         // no insteresting orders
         return _interested;
      }

   private:
      void _displayProgression( float )
      {
      }

      void _refreshDisplay()
      {
      }

   private:
      std::set<OrderClassId>  _interested;
      ui32                    _nbMips;
      RefcountedTyped<impl::OrderMipPrecompute> _order;

      Volume*                 _cacheOldVolume;
   };
}
}

#endif