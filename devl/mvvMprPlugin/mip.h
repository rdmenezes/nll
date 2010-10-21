#ifndef MVV_PLATFORM_MIP_H_
# define MVV_PLATFORM_MIP_H_

# include "mvvMprPlugin.h"
# include <mvvPlatform/resource-volumes.h>
# include <mvvPlatform/resource-typedef.h>
# include "mip-tool.h"
# include "mip-tool-annotations.h"
# include "mip-tool-sorter.h"

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
         OrderMipDisplay( const Slice& sliceOrig, Slice& slice, const ResourceLut& lut, const nll::core::vector2ui& size, float zoom, float& oldZoom ) : Order( MVV_PLATFORM_ORDER_DISPLAY_MIP, Order::Predecessors(), true ), _slice( slice ), _lut( lut ), _size( size ), _sliceOrig( sliceOrig ), _zoom( zoom ), _oldZoom( oldZoom )
         {
         }

         virtual OrderResult* _compute()
         {
            OrderMipDisplayResult* result = new OrderMipDisplayResult();

            if ( _size[ 0 ] > 0 && _size[ 1 ] > 0 )
            {
               // we display only if we need to...
               if ( _slice.size()[ 0 ] != _size[ 0 ] || _slice.size()[ 1 ] || _size[ 1 ] || _zoom != _oldZoom )
               {
                  float sx = static_cast<f32>(_size[ 0 ] ) / _sliceOrig.size()[ 0 ] * _sliceOrig.getSpacing()[ 0 ];
                  float sy = static_cast<f32>(_size[ 1 ] ) / _sliceOrig.size()[ 1 ] * _sliceOrig.getSpacing()[ 1 ];
                  float s = std::min( sx, sy ) / 3;
                  Slice slice( nll::core::vector3ui( _size[ 0 ], _size[ 1 ], 1 ),
                               _sliceOrig.getAxisX(),
                               _sliceOrig.getAxisY(),
                               _sliceOrig.getOrigin(),
                               nll::core::vector2f( _sliceOrig.getSpacing()[ 0 ] / s * _zoom,
                                                    _sliceOrig.getSpacing()[ 1 ] / s * _zoom ) );
                  if ( slice.size()[ 0 ] && slice.size()[ 1 ] && slice.size()[ 2 ] )
                  {
                     // if the output slice size is (0, 0, 0), it means a mip segment is created, but not displayed
                     nll::imaging::resampling<f32, Slice::BilinearInterpolator>( _sliceOrig, slice );
                  }

                  // cache the result for future usage...
                  _slice = slice;
                  _oldZoom = _zoom;
               }

               result->slice = Sliceuc( nll::core::vector3ui( _size[ 0 ], _size[ 1 ], 3 ),
                                        _slice.getAxisX(),
                                        _slice.getAxisY(),
                                        _slice.getOrigin(),
                                        _slice.getSpacing() );

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
            }
            return result;
         }

      private:
         // copy disabled
         OrderMipDisplay();
         OrderMipDisplay& operator=( OrderMipDisplay& );

      private:
         Slice&                        _slice;
         const Slice&                  _sliceOrig;
         const ResourceLut&            _lut;
         nll::core::vector2ui          _size;
         float                         _zoom;
         float&                        _oldZoom;
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
         std::vector<Slice>   slicesOrig;
         std::vector<f32>     currentZoomFactor;
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

               Slice cpy;
               cpy.clone( result->slices[ n ] );
               result->slicesOrig.push_back( cpy );
               result->currentZoomFactor.push_back( 1.0f );

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

   class MVVMPRPLUGIN_API Mip : public EngineOrder,  public LinkableDouble<MipTool*, Mip*>
   {
   public:
      typedef LinkableDouble<MipTool*, Mip*>  ToolLinks;

   private:
      typedef std::vector< MipTool* >     ToolsStorage;

   public:
      class MipToolWrapper : public Engine
      {
      private:
         // output of one tool is connected to the input of the following output
         ResourceSliceuc   inputMip;     /// must never be modified directly

      public:
         ResourceSliceuc   outputMip;

      public:
         MipToolWrapper( ResourceSliceuc& vinputMip,
                         Mip& mip,
                         MipTool* tool,
                         EngineHandler& handler ) : Engine( handler ), inputMip( vinputMip ), _tool( tool ), _mip( mip ), outputMip( true )
         {
            ensure( tool, "must not be zero" );

            inputMip.connect( this );
            ResourceBool* rtool = tool; rtool->connect( this );

            handler.connect( *this );

            if ( !tool->isSavingMprImage() )
            {
               outputMip = inputMip;
            }
         }

         virtual ~MipToolWrapper()
         {
         }

      private:
         // update the segment as soon as it is changed
         virtual bool _run()
         {
            //std::cout << "size=" << inputSegment.getValue().size()[ 0 ] << " " << inputSegment.getValue().size()[ 1 ] << std::endl;

            if ( !inputMip.getValue().size()[ 0 ] ||
                 !inputMip.getValue().size()[ 1 ] ||
                 !inputMip.getValue().size()[ 2 ] )
            {
               // we have no segment to display, then just do nothing
               return true;
            }

            if ( _tool->isModifyingMprImage() )
            {
               if ( _tool->isSavingMprImage() )
               {
                  ensure( inputMip.getValue().getStorage().begin() !=
                          outputMip.getValue().getStorage().begin(), "we can't share the same buffer if it modifies the mip..." );

                  // if we modify the image & the dimenstion are not good, just deep copy the input
                  if ( inputMip.getValue().getStorage().size() != outputMip.getValue().getStorage().size() )
                  {
                     Sliceuc slice;
                     slice.setGeometry( inputMip.getValue().getAxisX(),
                                        inputMip.getValue().getAxisY(),
                                        inputMip.getValue().getOrigin(),
                                        inputMip.getValue().getSpacing() );

                     slice.getStorage().clone( inputMip.getValue().getStorage() );
                     outputMip.setValue( slice );
                  } else {
                     // quick copy, we know memory is continuous
                     Sliceuc::iterator inputBegin = inputMip.getValue().getStorage().begin();
                     Sliceuc::iterator outputBegin = outputMip.getValue().getStorage().begin();                
                     memcpy( &*outputBegin, &*inputBegin, inputMip.getValue().getStorage().size() );

                     // update the geometry in case it is different
                     outputMip.getValue().setGeometry( inputMip.getValue().getAxisX(),
                                                       inputMip.getValue().getAxisY(),
                                                       inputMip.getValue().getOrigin(),
                                                       inputMip.getValue().getSpacing() );
                  }

                  // notify the changes
                  outputMip.notify();
               }

               //do the changes on the output
               _tool->updateMip( outputMip, _mip );
            }
            return true;
         }

      private:
         MipTool*      _tool;
         Mip&          _mip;
      };

   // output slots
   public:
      /// output public slot holding the resampled & annoted MIP
      ResourceSliceuc   outImage;

   private:
      /// output private slot holding the resampled MIP
      ResourceSliceuc   _outImage;

   // input slots
   public:
      ResourceVector2ui       size;
      ResourceLut             lut;
      ResourceFloat           anglex;
      ResourceVolumes         volumes;    // we are expecting only 1 volume
      ResourceUi32            fps;
      ResourceFloat           zoom;

   public:
      Mip( ResourceStorageVolumes& storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, ui32 nbMips = 48 ) : EngineOrder( handler, provider, dispatcher ), volumes( storage ), _nbMips( nbMips ), _outImage( true ), outImage( true )
      {
         _interested.insert( MVV_PLATFORM_ORDER_DISPLAY_MIP );
         dispatcher.connect( this );

         fps.setValue( 17 );
         zoom.setValue( 1.0f );

         size.connect( this );
         lut.connect( this );
         anglex.connect( this );
         volumes.connect( this );
         fps.connect( this );
         zoom.connect( this );

         _orderDisplay.unref();
         _outImage = outImage;

         _sorter = RefcountedTyped<MipToolSorter>( new MipToolSorterPriorityQueue() );
      }

      ~Mip()
      {
         _dispatcher.disconnect( this );
         ToolLinks::removeConnections();
      }

      virtual void connect( MipTool* tool );

      virtual void disconnect( MipTool* tool );

      virtual bool _run()
      {
         //const bool isMipComputed = !_order.isEmpty() || (*_order).getResultInProgress()->progress >= 1;
         if ( volumes.begin() == volumes.end() )
         {
            return false;
         }
         if ( volumes.size() > 1 )
            throw std::runtime_error( "Unexpected number of volumes (only zero or one)" );
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
            if ( f && _timer.getCurrentTime() > 1.0f / ( 60 * f ) )
            {
               float newAngle = anglex.getValue() + (float)f / 360;
               newAngle = ( newAngle > 2 * nll::core::PI ) ? 0 : newAngle;
               anglex.setValue( newAngle );
               _timer.start();
            }
            if ( _orderDisplay.isEmpty() )
            {
               // everything else has triggered an update of the precomputed MIP, just update the slice
               float newAngle = anglex.getValue();
               while ( newAngle > nll::core::PI * 2 )
               {
                  newAngle -= static_cast<float>( nll::core::PI * 2 );
               }
               while ( newAngle < 0 )
               {
                  newAngle += static_cast<float>( nll::core::PI * 2 );
               }
               anglex.setValue( newAngle );
               ui32 sliceId = static_cast<ui32>( newAngle / ( nll::core::PI * 2 ) * order->slices.size() );
               ensure( sliceId < order->slicesOrig.size(), "out of bound slice..." );
               _orderDisplay = RefcountedTyped<impl::OrderMipDisplay>( new impl::OrderMipDisplay( order->slicesOrig[ sliceId ], order->slices[ sliceId ], lut, size.getValue(), zoom.getValue(), order->currentZoomFactor[ sliceId ] ) );
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

      template <class T>
      std::set<T*> getTools()
      {
         std::set<T*> tools;
         for ( ToolsStorage::iterator it = _tools.begin(); it != _tools.end(); ++it )
         {
            T* tool = dynamic_cast<T*>( *it );
            if ( tool )
               tools.insert( tool );
         }
         return tools;
      }

      void refreshTools()
      {
         _outImage.notify();
      }

      virtual void receive( const EventMouse& event, const nll::core::vector2ui& windowOrigin );

   protected:
      void updateToolsList();

   private:
      void _displayProgression( float progress )
      {
         ResourceSliceuc::value_type& slice = outImage.getValue();
         if ( slice.size()[ 0 ] < 2 || !slice.size()[ 1 ] )
         {
            // in case the display is not ready
            return;
         }

         typedef ResourceSliceuc::value_type Slice;
         typedef Slice::value_type           Image;
         Image::DirectionalIterator it1 = slice.getStorage().getIterator( 0, 0, 0 );
         Image::DirectionalIterator it2 = slice.getStorage().getIterator( 1, 0, 0 );

         ui32 nb = static_cast<ui32>( slice.size()[ 1 ] * progress );
         for ( ui32 n = 0; n < nb; ++n )
         {
            *it1 = 255;
            it1.addy();

            *it2 = 255;
            it2.addy();
         }
      }

   private:
      typedef RefcountedTyped<MipToolWrapper>      Wrapper;
      typedef std::vector<Wrapper>                 Wrappers;


      std::set<OrderClassId>  _interested;
      ui32                    _nbMips;
      RefcountedTyped<impl::OrderMipPrecompute> _order;
      RefcountedTyped<impl::OrderMipDisplay>    _orderDisplay;

      Volume*                 _cacheOldVolume;
      nll::core::vector2ui    _cacheOldSize;
      nll::core::Timer        _timer;
      ToolsStorage            _tools;

      RefcountedTyped<MipToolSorter>      _sorter;
      Wrappers                            _wrappers;
   };
}
}

#endif