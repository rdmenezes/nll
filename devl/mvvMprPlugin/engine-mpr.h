#ifndef MVV_MPR_ENGINE_MPR_H_
# define MVV_MPR_ENGINE_MPR_H_

#include <mvvPlatform/engine-order.h>
#include <mvvPlatform/resource-vector.h>
#include <mvvPlatform/resource-volumes.h>
#include <mvvPlatform/resource-typedef.h>
#include <mvvPlatform/resource-barrier.h>
#include <mvvPlatform/types.h>
#include "types.h"

/// order to create MPR from a volume
# define MVV_PLATFORM_ORDER_CREATE_SLICE  OrderClassId::create( "MVV_PLATFORM_ORDER_CREATE_SLICE" )

/// order to blend several slices into 1 RGB slice
# define MVV_PLATFORM_ORDER_BLEND_SLICE  OrderClassId::create( "MVV_PLATFORM_ORDER_BLEND_SLICE" )

/// when constructed, the segment's position will be defined as below
# define MVV_PLATFORM_UNINITIALIZED_POSITION           nll::core::vector3f( -10000, -10000, -10000 )

namespace mvv
{
namespace platform
{
   enum InterpolationMode
   {
      LINEAR,
      NEAREST
   };

   typedef ResourceValue<InterpolationMode>  ResourceInterpolationMode;

   namespace impl
   {
      class OrderSliceCreatorResult : public OrderResult
      {
      public:
         OrderSliceCreatorResult( Slice slice ) : _slice( slice )
         {
         }

         Slice& getSlice()
         {
            return _slice;
         }

      private:
         Slice _slice;
      };

      /**
       @brief Create a slice from position, orientation...

       @note We are copying the data so that the values are not changing while computing the MPR...
       */
      class OrderSliceCreator : public Order
      {
      public:
         OrderSliceCreator( clock_t time,
                            const nll::core::vector3f& position,
                            const nll::core::vector3f& dirx,
                            const nll::core::vector3f& diry,
                            const nll::core::vector3f& panning,
                            const nll::core::vector2f& zoom,
                            const nll::core::vector2ui& size,
                            InterpolationMode interpolation,
                            Volume& volume,
                            SymbolVolume volumeName ) : Order( MVV_PLATFORM_ORDER_CREATE_SLICE, Order::Predecessors(), true ), _time( time ), _volume( volume ), _position( position ), _dirx( dirx ), _diry( diry ), _panning( panning ), _zoom( zoom ), _size( size ), _interpolation( interpolation ), _volumeName( volumeName )
         {
         }

      protected:
         virtual OrderResult* _compute()
         {
            typedef nll::imaging::InterpolatorNearestNeighbour<Volume>  InterpolatorNN;
            typedef nll::imaging::InterpolatorTriLinear<Volume>         InterpolatorTrilinear;
            typedef nll::imaging::Mpr<Volume, InterpolatorNN>           MprNN;
            typedef nll::imaging::Mpr<Volume, InterpolatorTrilinear>    MprTrilinear;

            Slice slice( nll::core::vector3ui( _size[ 0 ], _size[ 1 ], 1 ),
                         _dirx,
                         _diry,
                         _position + _panning,
                         nll::core::vector2f( 1.0f / _zoom[ 0 ], 1.0f / _zoom[ 1 ] ) );

            switch ( _interpolation )
            {
            case LINEAR:
               {
                  MprTrilinear mpr( _volume );
                  mpr.getSlice( slice );
                  break;
               }
            case NEAREST:
               {
                  MprNN mpr( _volume );
                  mpr.getSlice( slice );
                  break;
               }
            default:
               throw std::exception( "interpolation mode not handled" );
            }
            return new OrderSliceCreatorResult( slice );
         }

      public:
         SymbolVolume getVolume() const
         {
            return _volumeName;
         }

         clock_t getTime() const
         {
            return _time;
         }

	  private:
		  // copy disabled
		  OrderSliceCreator& operator=( const OrderSliceCreator& );
		  OrderSliceCreator( const OrderSliceCreator& );

      protected:
         clock_t                 _time;
         nll::core::vector3f     _position;
         nll::core::vector3f     _dirx;
         nll::core::vector3f     _diry;
         nll::core::vector3f     _panning;
         nll::core::vector2f     _zoom;
         nll::core::vector2ui    _size;
         InterpolationMode       _interpolation;
         Volume&				      _volume;
         SymbolVolume            _volumeName;
      };

      /**
       @brief computes a MPR for each volume in the list
       */
      class EngineMprSlice : EngineOrder
      {
      protected:
         // input slots
         ResourceVolumes      volumes;       /// list of volume to display
         ResourceVector3f     position;      /// position in mm of the camera
         ResourceVector3f     directionx;    /// x-axis of the MPR
         ResourceVector3f     directiony;    /// y-axis of the MPR
         ResourceVector3f     panning;       /// panning position in mm
         ResourceVector2f     zoom;          /// zoom
         ResourceVector2ui    size;          /// size in pixel of the slice to render

         ResourceBool               isInteracting; /// set to true if MPR is being modified, so we can use less accurate algorithms to speed up the process
         ResourceInterpolationMode  interpolation; /// Interpolation used to produce the slices

      public:
         // output slots
         ResourceOrders       outOrdersComputed;   /// return the orders that have been computed

      public:
         ~EngineMprSlice()
         {
            _dispatcher.disconnect( this );
         }

         EngineMprSlice( bool& ready,
                         ResourceVolumes vvolumes,
                         ResourceVector3f vposition,
                         ResourceVector3f vdirectionx,
                         ResourceVector3f vdirectiony,
                         ResourceVector3f vpanning,
                         ResourceVector2f vzoom,
                         ResourceVector2ui vsize,
                         ResourceBool visInteracting,
                         ResourceInterpolationMode vinterpolation,
                         EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, bool fasterDisplayWhenInteracting ) : 
            volumes( vvolumes ), position( vposition ), directionx( vdirectionx ), directiony( vdirectiony ), panning( vpanning ), zoom( vzoom ), size( vsize ), isInteracting( visInteracting ), interpolation( vinterpolation ),
            EngineOrder( handler, provider, dispatcher ), _fasterDisplayWhenInteracting( fasterDisplayWhenInteracting ),
            _ready( ready )
         {
            _construct();
            dispatcher.connect( this );

            position.connect( this );
            directionx.connect( this );
            directiony.connect( this );
            panning.connect( this );
            zoom.connect( this );
            size.connect( this );
            volumes.connect( this );
            interpolation.connect( this );

            if ( fasterDisplayWhenInteracting )
            {
               isInteracting.connect( this );
            }
            interpolation.connect( this );
         }

         virtual void consume( Order* )
         {
            if ( _ordersCheck.size() == 0 )
            {
               // all orders have been consumed
               return;
            }

            // A rendering order has completed. Now check that all our order are run, and prepare the engine
            // for the next run
            for ( std::vector< RefcountedTyped<Order> >::iterator it = _ordersCheck.begin(); it != _ordersCheck.end(); )
            {
               if ( !(**it).getResult() )
                  return;
               //std::cout << "consume order:" << (**it).getId() << std::endl;
               _ordersCheck.erase( it );
               it = _ordersCheck.begin();
            }


            // all orders are complete! we must clear our oders to complete list, and update output slots
            // we don't want to notify each time the engines connected to the output slot, so create a barrier
            ResourceScopedBarrier barrier( outOrdersComputed );
            outOrdersComputed.clear();
            for ( std::vector< RefcountedTyped<Order> >::iterator it = _ordersSend.begin(); it != _ordersSend.end(); ++it )
            {
               outOrdersComputed.insert( *it );
            }

            // clear the waiting list so we can compute a new list!
            std::cout << "slice clear=" << &_ready << std::endl;
            _ordersSend.clear();
         }

         virtual const std::set<OrderClassId>& interestedOrder() const
         {
            return _interested;
         }

      protected:
         virtual bool _run()
         {
            if ( _ordersSend.size() /*|| !_ready*/  )
            {
               // if we already have an order or this order is still in the pipeline then wait
               return false;
            }

            std::vector< RefcountedTyped<Order> > orders;
            InterpolationMode currentInterpolation = _fasterDisplayWhenInteracting ? NEAREST : interpolation.getValue();
            for ( ResourceVolumes::Iterator it = volumes.begin(); it != volumes.end(); ++it )
            {
               _ready = false;
               OrderSliceCreator* slicer = new OrderSliceCreator( clock(),
                                                                  position.getValue(),
                                                                  directionx.getValue(),
                                                                  directiony.getValue(),
                                                                  panning.getValue(),
                                                                  zoom.getValue(),
                                                                  size.getValue(),
                                                                  currentInterpolation,
                                                                  **it,
                                                                  it.getName() );

               std::cout << "slicer=" << &_ready << " clock=" << slicer->getTime() << " pos=" << position.getValue()[ 0 ] << " " << position.getValue()[ 1 ] << " " << position.getValue()[ 2 ] << std::endl;
               RefcountedTyped<Order> order( slicer );
               orders.push_back( order );
               _orderProvider.pushOrder( &*order );
            }

            _ordersSend = orders;
            _ordersCheck = orders;
            return true;
         }

         void _construct()
         {
            _interested.insert( MVV_PLATFORM_ORDER_CREATE_SLICE );
         }

      protected:
         bool                    _fasterDisplayWhenInteracting;
         std::set<OrderClassId>  _interested;
         std::vector< RefcountedTyped<Order> >  _ordersSend;
         std::vector< RefcountedTyped<Order> >  _ordersCheck;
         bool& _ready;
      };


      class OrderSliceBlenderResult : public OrderResult
      {
      public:
         OrderSliceBlenderResult( Sliceuc s ) : blendedSlice( s )
         {}

         Sliceuc blendedSlice;
      };

      class OrderSliceBlender : public Order
      {
      public:
		  OrderSliceBlender( clock_t time,
                           std::set<Order*> orders,
                           ResourceMapTransferFunction& maplut,
                           ResourceFloats& intensities ) : Order( MVV_PLATFORM_ORDER_BLEND_SLICE, Order::Predecessors() ), _time( time ), _maplut( maplut ), _intensities( intensities ), _orders( orders )
         {
          
         }

        clock_t getTime() const
        {
           return _time;
        }

      private:
         // disable copy
         OrderSliceBlender( const OrderSliceBlender& );
         OrderSliceBlender& operator=( OrderSliceBlender& );

      protected:
         virtual OrderResult* _compute()
         {
            std::vector< nll::imaging::BlendSliceInfof<ResourceLut::lut_type> > sliceInfos;

            int n = 0;
			   for ( std::set<Order*>::iterator it = _orders.begin(); it != _orders.end(); ++it, ++n )
            {
               OrderSliceCreator* orderCreator = dynamic_cast<OrderSliceCreator*> ( *it );
               impl::OrderSliceCreatorResult* result = dynamic_cast<impl::OrderSliceCreatorResult*>( (**it).getResult() );
               ensure( result, "must nnot be null" );
               if ( !orderCreator )
                  throw std::exception( "unexpected type of order" );
               SymbolVolume volume = orderCreator->getVolume();

               float intensity = 0;
               ResourceLut lut;

               // we have to be careful here: we don't wan't to have side effects with
               // multithreading (i.e. unsafely increase/decrease refcount of a shared object...)
               // so we use refereces 
               bool res  = _maplut.find( volume, lut ) && _intensities.find( volume, intensity );
               if ( res )
               {
                  sliceInfos.push_back( nll::imaging::BlendSliceInfof<ResourceLut::lut_type>( result->getSlice(), intensity, lut.getValue().lut ) );
               }
            }
            
            if ( sliceInfos.size() )
            {
               Sliceuc result( nll::core::vector3ui( sliceInfos[ 0 ].slice.size()[ 0 ],
                                                     sliceInfos[ 0 ].slice.size()[ 1 ],
                                           3 ),
                               sliceInfos[ 0 ].slice.getAxisX(),
                               sliceInfos[ 0 ].slice.getAxisY(),
                               sliceInfos[ 0 ].slice.getOrigin(),
                               sliceInfos[ 0 ].slice.getSpacing() );

               nll::imaging::blendDummy( sliceInfos, result );

               //nll::core::writeBmp( result.getStorage(), "c:/tmp/test.bmp" );
               //std::cout << "export image" << std::endl;

               _orders.clear();
               return new OrderSliceBlenderResult( result );
            }
            _orders.clear();
            return new OrderSliceBlenderResult( Sliceuc() );
         }

      protected:
         clock_t                             _time;
		   std::set<Order*>                    _orders;
         ResourceMapTransferFunction&        _maplut;
         ResourceFloats&                     _intensities;
      };

      /**
       @ingroup platform
       @brief Blend slices
       */
      class EngineSliceBlender : public EngineOrder
      {
      protected:
         // input slots
         ResourceOrders                ordersToBlend; // orders to blend
         ResourceMapTransferFunction   lut;
         ResourceFloats                intensities;

      protected:
         // output slot but protected
         ResourceUi32 fps;

      public:
         // output slots
         ResourceSliceuc               blendedSlice;

      public:
         EngineSliceBlender( bool& ready, ResourceOrders vordersToBlend, ResourceMapTransferFunction vlut, ResourceFloats vintensities, ResourceUi32 vfps,
                             EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher ),
            ordersToBlend( vordersToBlend ), lut( vlut ), intensities( vintensities ),
            _ready( ready )
         {
            _construct();
            _orderSend.unref();

            dispatcher.connect( this );

            ordersToBlend.connect( this );
            lut.connect( this );
            intensities.connect( this );

            _fps = 0;
            _clock = clock();
            _lastTime = 0;
         }

         ~EngineSliceBlender()
         {
            _dispatcher.disconnect( this );
         }

      protected:
         virtual bool _run()
         {
            if ( !ordersToBlend.size() )
            {
               // we have nothing to wait for...
               return true;
            }
            if ( _orderSend.isEmpty() )
            {
               // we have been notified
                std::set<Order*> orders;
                _copy = ResourceOrders();
                clock_t maxClock = 0;
                for ( ResourceOrders::Iterator it = ordersToBlend.begin(); it != ordersToBlend.end(); ++it )
                {
                    orders.insert( &( **it ) );
                    _copy.insert( *it );

                    OrderSliceCreator* orderCreator = dynamic_cast<OrderSliceCreator*> ( &( **it ) );
                    maxClock = std::max( maxClock, orderCreator->getTime() );

                    std::cout << "create blend=" << &_ready << " clock=" << orderCreator->getTime() << std::endl;
                }
               _orderSend = RefcountedTyped<Order>( new OrderSliceBlender( maxClock, orders, lut, intensities ) );
               _orderProvider.pushOrder( &*_orderSend );
               return true;
            }

            // we need to wait for blending is finished to start next round...
            return false;
         }

         virtual void consume( Order* order )
         {
            if ( _orderSend.isEmpty() )
               return;
            if ( order == &*_orderSend )
            {
               OrderSliceBlenderResult* result = dynamic_cast<OrderSliceBlenderResult*>( (*order).getResult() );
               if ( !result )
                  throw std::exception( "unexpected order received!" );

               OrderSliceBlender* orderBlender = dynamic_cast<OrderSliceBlender*> ( order );
               ensure( orderBlender, "error can't be null..." );
               if ( orderBlender->getTime() >= _lastTime )
               {
                  _lastTime = orderBlender->getTime();
                  blendedSlice.setValue( result->blendedSlice );
                  std::cout << "blender=" << &_ready << " pos=" << result->blendedSlice.getOrigin()[ 0 ] << " " << result->blendedSlice.getOrigin()[ 1 ] << " " << result->blendedSlice.getOrigin()[ 2 ] << std::endl;
               } else {
                  std::cout << "error=" << std::endl;
               }

               _orderSend.unref();
               ordersToBlend.clear();
               //_ready = true;

               ++_fps;
               if ( ( clock() - _clock ) / (double)CLOCKS_PER_SEC >= 1 )
               {
                  std::cout << "----engine:" << this << " fps=" << _fps << std::endl;
                  fps.setValue( _fps );   
                  _fps = 0;
                  _clock = clock();
               }
            }
         }

         virtual const std::set<OrderClassId>& interestedOrder() const
         {
            return _interested;
         }

         void _construct()
         {
            _interested.insert( MVV_PLATFORM_ORDER_BLEND_SLICE );
         }

      protected:
         std::set<OrderClassId>  _interested;
         RefcountedTyped<Order>  _orderSend;
         ResourceOrders          _copy;

         ui32                    _fps;
         ui32                    _clock;
         bool&                   _ready;
         clock_t                 _lastTime;
      };
   }

   /**
    @ingroup platform
    @brief Compute a fused MPR of a set of volume
    */
   class EngineMpr : EngineOrder
   {
   public:
      // public output slots
      ResourceSliceuc   blendedSlice;
      ResourceUi32      fps;

   public:
      ~EngineMpr()
      {
         std::cout << "destroy engine MPR" << std::endl;
         _dispatcher.disconnect( this );
      }

      EngineMpr( ResourceVolumes vvolumes,
                 ResourceVector3f vposition,
                 ResourceVector3f vdirectionx,
                 ResourceVector3f vdirectiony,
                 ResourceVector3f vpanning,
                 ResourceVector2f vzoom,
                 ResourceVector2ui vsize,
                 ResourceMapTransferFunction vlut,
                 ResourceFloats vintensities,
                 ResourceBool visInteracting,
                 ResourceInterpolationMode vinterpolation,
                 EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, bool fasterDisplayWhenInteracting = false ) : 
      EngineOrder( handler, provider, dispatcher ),
      _mprSlicer( _ready,
                  vvolumes,
                  vposition,
                  vdirectionx,
                  vdirectiony,
                  vpanning,
                  vzoom,
                  vsize,
                  visInteracting,
                  vinterpolation,
                  handler, provider, dispatcher, fasterDisplayWhenInteracting ),
      _sliceBlender( _ready, _mprSlicer.outOrdersComputed, vlut, vintensities, fps, handler,  provider, dispatcher )
      {
         _ready = true;
         dispatcher.connect( this );
         blendedSlice = _sliceBlender.blendedSlice;
      }

      virtual bool _run()
      {
         // we don't do any actions
         return true;
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
      std::set<OrderClassId>        _interested;
      impl::EngineMprSlice          _mprSlicer;
      impl::EngineSliceBlender      _sliceBlender;
      bool                          _ready;
   };
}
}

#endif