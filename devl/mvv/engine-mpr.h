#ifndef MVV_ENGINE_MPR_H_
# define MVV_ENGINE_MPR_H_

# include "dynamic-resource.h"
# include "drawable.h"
# include "resource.h"
# include "resource-manager.h"
# include <nll/nll.h>

namespace mvv
{
   /**
    @ingroup mvv
    @brief Result of a reformated volume.
    */
   struct OrderMprRenderingResult : public OrderResult
   {
      typedef nll::imaging::InterpolatorNearestNeighbour<MedicalVolume>    InterpolatorNN;
      typedef nll::imaging::Mpr<MedicalVolume, InterpolatorNN>::Slice      Slice;

      OrderMprRenderingResult( Slice s ) : slice( s )
      {}

      Slice slice;
   };

   MVV_BIND_ORDER_RESULT( ORDER_MPR_RENDERING, OrderMprRenderingResult );

   /**
    @ingroup mvv
    @brief Order to asynchronously render a reformated slice of a volume
    */
   class OrderMprRendering : public Order
   {
      // basic types
      typedef nll::imaging::InterpolatorNearestNeighbour<MedicalVolume>   InterpolatorNN;
      typedef nll::imaging::InterpolatorTriLinear<MedicalVolume>          InterpolatorTrilinear;
      typedef nll::imaging::Mpr<MedicalVolume, InterpolatorNN>            MprNN;
      typedef nll::imaging::Mpr<MedicalVolume, InterpolatorTrilinear>     MprTrilinear;

   public:
      typedef nll::imaging::Mpr<MedicalVolume, InterpolatorNN>::Slice     Slice;

   public:
      enum Interpolator
      {
         NEAREST_NEIGHBOUR,
         TRILINEAR
      };

   public:
      OrderMprRendering( const MedicalVolume* vol,
                         ui32 sx,
                         ui32 sy,
                         double zoomx,
                         double zoomy,
                         nll::core::vector3d origin,
                         nll::core::vector3d v1,
                         nll::core::vector3d v2,
                         Interpolator interpolator ) : Order( ORDER_MPR_RENDERING, true, Predecessors() ),
         _volume( vol ), _sx( sx ), _sy( sy ), _zoomx( zoomx ), _zoomy( zoomy ),
         _origin( origin ), _v1( v1 ), _v2( v2 ), _interpolator( interpolator )
      {
         ensure( vol && sx && sy, "invalid parameters" );
      }

      virtual ~OrderMprRendering()
      {}

      virtual OrderResult* run()
      { 
         Slice slice;

         nll::core::Timer t1;

         switch ( _interpolator )
         {
         case NEAREST_NEIGHBOUR:
            {
               MprNN mpr( *_volume, _sx, _sy );
               slice = mpr.getSlice( _origin, _v1, _v2, nll::core::vector2d( _zoomx, _zoomy ) );
               break;
            }
         case TRILINEAR:
            {
               MprTrilinear mpr( *_volume, _sx, _sy );
               slice = mpr.getSlice( _origin, _v1, _v2, nll::core::vector2d( _zoomx, _zoomy ) );
               break;
            }
         default:
            ensure( 0, "interpolation not handled" );
         };

         return new OrderMprRenderingResult( slice );
      }

      const MedicalVolume* getVolume() const
      {
         return _volume;
      }

   protected:
      const MedicalVolume*    _volume;
      ui32                    _sx;
      ui32                    _sy;
      double                  _zoomx;
      double                  _zoomy;
      Interpolator            _interpolator;
      nll::core::vector3d     _origin;
      nll::core::vector3d     _v1;
      nll::core::vector3d     _v2;
   };

   /**
    @brief Combines all the slices within a MPR using transfer functions
    */
   class OrderCombineMpr : public Order
   {
   public:
      typedef std::vector<Order*>      Orders;
      typedef nll::core::Image<ui8>    Image;

   public:
      OrderCombineMpr( const Orders& predecessors, Image& slice, ResourceVolumes& volumes, ResourceLuts& luts, ResourceVolumeIntensities& intensities ) : Order( ORDER_MPR_RENDERING_COMBINE, true, makePredecessors( predecessors ) ),
         _tracked( predecessors ), _volumes( volumes ), _slice( slice ), _luts( luts ), _intensities( intensities )
      {
      }

      virtual OrderResult* run()
      {
         double ratioCheck = 0;
         typedef std::vector<OrderMprRendering::Slice::DirectionalIterator*>  Iterators;
         typedef std::vector<ResourceLut*>                                    Windowings;
         typedef std::vector<double>                                          Intensities;

         Iterators  iterators( _volumes.size() );
         Windowings windowings( _volumes.size() );
         Intensities intensities( _volumes.size() );

         ui32 n = 0;
         for ( ResourceVolumes::iterator it = _volumes.begin(); it != _volumes.end(); ++it, ++n )
         {
            const OrderMprRenderingResult* slice = dynamic_cast<const OrderMprRenderingResult*>( _tracked[ n ]->getResult() );
            iterators[ n ] = new OrderMprRendering::Slice::DirectionalIterator( slice->slice.beginDirectional() );
            intensities[ n ] = _intensities.getIntensity( *it );
            windowings[ n ] = _luts.getLut( *it );
            ratioCheck += intensities[ n ];
         }
         ensure( nll::core::equal<double>( ratioCheck, 1, 1e-2 ), "ratio must sum to 1" );

         for ( Image::DirectionalIterator itOut = _slice.beginDirectional(); itOut != _slice.endDirectional(); ++itOut )
         {
            double vala = 0;
            double valb = 0;
            double valc = 0;
            for ( ui32 nn = 0; nn < iterators.size(); ++nn )
            {
               const ui8* buf = windowings[ nn ]->transform( **( iterators[ nn ] ) );
               const double intensity = intensities[ nn ];
               vala += buf[ 0 ] * intensity;
               valb += buf[ 1 ] * intensity;
               valc += buf[ 2 ] * intensity;

               ++( *iterators[ nn ] );
            }
            itOut.pickcol( 0 ) = static_cast<ui8>( vala );
            itOut.pickcol( 1 ) = static_cast<ui8>( valb );
            itOut.pickcol( 2 ) = static_cast<ui8>( valc );
         }

         for ( Iterators::iterator it = iterators.begin(); it != iterators.end(); ++it )
            delete *it;

         return new OrderResult();
      }

   private:
      static Order::Predecessors makePredecessors( const Orders& orders )
      {
         Order::Predecessors predecessors;
         for ( Orders::const_iterator it = orders.begin(); it != orders.end(); ++it )
            predecessors.insert( ( *it )->getId() );
         return predecessors;
      }

      // non copiyable
      OrderCombineMpr& operator=( const OrderCombineMpr& );
      OrderCombineMpr( const OrderCombineMpr& );


      const Orders&                       _tracked;
      ResourceVolumes&                    _volumes;
      ResourceLuts&                       _luts;
      ResourceVolumeIntensities&          _intensities;
      Image&                              _slice;
   };

   class EngineMprCombiner : public EngineRunnable
   {
      typedef std::vector<Order*>   Orders;

   public:
      ResourceImageRGB outFusedMPR;

   public:
      EngineMprCombiner( OrderProvider& orderProvider, ResourceOrderList& orders, ResourceVolumeIntensities& intensities, ResourceLuts& luts, ResourceVolumes& volumes ) : _orderProvider( orderProvider ), _orders( orders ),
         _intensities( intensities ), _luts( luts ), _volumes( volumes ), _current( 0 )
      {
         attach( _orders );
         attach( _intensities );
         attach( _luts );
         _needToRecompute = false;
         _idle = true;
      }

      virtual void consume( Order* o )
      {
         if ( !_current || ( o->getOrderClassId() != ORDER_MPR_RENDERING_COMBINE ) )
            return;
         if ( o != _current )
            return;
         assert( o->getResult() );


         // the order has correctly executed: every volume MPR has been computed as well
         // as their fusion. Just publish the result and destroy the orders
         outFusedMPR.notifyChanges();
         for ( Orders::const_iterator it = _renderingOrders.begin(); it != _renderingOrders.end(); ++it )
            _orderProvider.pushOrderToDestroy( *it );
         _current = 0;
         _idle = true;

         //_needToRecompute = false;
         std::cout << "combiner.consume() idle=true" << std::endl;

         static int nbFps = 0;
         static unsigned last = clock();
         ++nbFps;

         if ( ( clock() - last ) / (double)CLOCKS_PER_SEC >= 1 )
         {
            std::cout << "---------------fps=" << nbFps << std::endl;
            nbFps = 0;
            last = clock();
         }
         //std::cout << "---saving file in c:\tmp---" << std::endl;
         //nll::core::writeBmp( outFusedMPR.image, "c:/tmp/out.bmp" );
      }

      /**
       @return true if all orders have been processed
       */
      const bool& isIdle() const
      {
         return _idle;
      }

   private:
      virtual bool _run()
      {
         if ( !_volumes.size() || !_orders.getOrders().size() )
         {
            // if no volume or havent received any order->nothing to do!
            return true;
         }
         if ( _current )
         {
            // first finish our current order
            return false;
         }

         // track these orders as we need to destroy them
         _renderingOrders = _orders.getOrders();
         const OrderMprRenderingResult* renderingResult = dynamic_cast<const OrderMprRenderingResult*>( _renderingOrders[ 0 ]->getResult() );
         ensure( renderingResult, "wrong order..." );
         ensure( _renderingOrders.size(), "we need to have orders to fuse!" );

         // reset the frame
         outFusedMPR.image = ResourceImageRGB::Image( renderingResult->slice.sizex(),
                                                      renderingResult->slice.sizey(),
                                                      3,
                                                      true );

         // create the order
         OrderCombineMpr* order = new OrderCombineMpr( _renderingOrders, outFusedMPR.image, _volumes, _luts, _intensities );
         //std::cout << "--create fusion order[" << order->getId() << "]:" << _renderingOrders[ 0 ]->getId() << " " << _renderingOrders[ 1 ]->getId() << std::endl;
         _orderProvider.pushOrder( order );
         _current = order;
         _idle = false;
         std::cout << "combiner.run() idle=false" << std::endl;
         return true;
      }

   private:
      // non copiable
      EngineMprCombiner& operator=( const EngineMprCombiner& );
      EngineMprCombiner( const EngineMprCombiner& );

   private:
      OrderProvider&                _orderProvider;
      ResourceOrderList&            _orders;
      ResourceVolumeIntensities&    _intensities;
      ResourceLuts&                 _luts;
      ResourceVolumes&              _volumes;

      Orders   _renderingOrders;
      Order*   _current;
      bool     _idle;
   };

   /**
    @brief This engine computes the MPR
    */
   class EngineMprComputation : public EngineRunnable
   {
      typedef std::vector<Order*>   Orders;

   public:
      EngineMprComputation( OrderProvider& orderProvider,
                            const bool& canRecomputeMpr,
                            ResourceVolumes& volumes,
                            ResourceVector3d& origin,
                            ResourceVector3d& vector1,
                            ResourceVector3d& vector2,
                            ResourceVector2d& zoom,
                            ResourceVector2ui& renderingSize ) : _orderProvider( orderProvider ), _canRecomputeMpr( canRecomputeMpr ), _volumes( volumes ),
         _origin( origin ), _vector1( vector1 ), _vector2( vector2 ), _zoom( zoom ), _renderingSize( renderingSize )
      {
         attach( volumes );
         attach( origin );
         attach( vector1 );
         attach( vector2 );
         attach( zoom );
         _needToRecompute = false;
      }

      ~EngineMprComputation()
      {
         for ( Orders::iterator it = _tracked.begin(); it != _tracked.end(); ++it )
               _orderProvider.pushOrderToDestroy( *it );
      }

   private:
      /**
       @brief Create the orders to compute the MPRs of every volume for this 'MPR view'
       */
      virtual bool _run()
      {
         std::cout << "engine.computation::run()" << std::endl;
         if ( !_volumes.size() )
            return true;
         if ( _tracked.size() )
         {
            std::cout << "waiting..." << _tracked.size()<< std::endl;
            return false;
         }
         if ( _canRecomputeMpr )
         {
            // we delete the orders that are not needed anymore.
            for ( Orders::iterator it = _tracked.begin(); it != _tracked.end(); ++it )
               _orderProvider.pushOrderToDestroy( *it );

         } else {
            std::cout << "engine.computation::run() cannot recompute" << std::endl;
            // we need to wait for the MPR fusion
            return false;
         }

         _tracked = Orders( _volumes.size() );

         ui32 n = 0;
         for ( ResourceVolumes::iterator it = _volumes.begin(); it != _volumes.end(); ++it, ++n )
         {
            OrderMprRendering* order = new OrderMprRendering( *it, _renderingSize[ 0 ], _renderingSize[ 1 ], _zoom[ 0 ], _zoom[ 1 ],
                                                              nll::core::vector3d( _origin[ 0 ],
                                                                                   _origin[ 1 ],
                                                                                   _origin[ 2 ] ),
                                                              nll::core::vector3d( _vector1[ 0 ],
                                                                                   _vector1[ 1 ],
                                                                                   _vector1[ 2 ] ),
                                                              nll::core::vector3d( _vector2[ 0 ],
                                                                                   _vector2[ 1 ],
                                                                                   _vector2[ 2 ] ),
                                                                                   OrderMprRendering::TRILINEAR );
            _tracked[ n ] = order;
            _orderProvider.pushOrder( order );
         }

         // MPR rendering orders have been created
         return true;
      }

   public:
      /**
       @brief Consume an order
       */
      virtual void consume( Order* o )
      {
         if ( !_tracked.size() || ( o->getOrderClassId() != ORDER_MPR_RENDERING ) )
            return;

         // it is a MPR_RENDERING order so we need to check it is one of our order
         for ( ui32 n = 0; n < _tracked.size(); ++n )
            if ( !_tracked[ n ]->getResult() )
            {
               // one order has not finished yet so we need to wait
               return;
            }

         // all the orders are finished, just fuse them and update the result
         ensure( _tracked.size() == _volumes.size(), "error size doesn't match!" );

         // orders have been rendered, update the resource
         outOrdersToFuse.setOrders( _tracked );

         _tracked.clear();
      }

   private:
      EngineMprComputation& operator=( const EngineMprComputation& );
      EngineMprComputation( const EngineMprComputation& );

   public:
      ResourceOrderList outOrdersToFuse;

   private:
      OrderProvider&                      _orderProvider;
      ResourceVolumes&                    _volumes;
      ResourceVector3d&                   _origin;
      ResourceVector3d&                   _vector1;
      ResourceVector3d&                   _vector2;
      ResourceVector2d&                   _zoom;
      ResourceVector2ui&                  _renderingSize;

      const bool&                         _canRecomputeMpr;

      Orders                              _tracked;
   };

   /**
    @brief Render a fused MPR with LUT. Uses 2 engines: 1 for rendering the MPRs and another
           one to fuse them allowing to minimize the computations when just lut or intensity
           is updated

           By default the engine is not computing until the attached resources are changing
           
           A fused MPR is published.
    */
   class EngineMprImpl : public EngineRunnable
   {
      typedef std::vector<Order*>   Orders;

   public:
      // resource that export the computed fused MPR
      ResourceImageRGB  outFusedMpr;

   public:
      enum Orientation
      {
         FRONTAL,       // (1 0 0) (0 0 1)
         CORONAL,       // (0 1 0) (0 0 1)
         TRANSVERSE     // (1 0 0) (0 1 0)
      };

   public:
      EngineMprImpl( OrderProvider& orderProvider,
                     ResourceVolumes& volumes,
                     ResourceVector3d& origin,
                     ResourceVector3d& vector1,
                     ResourceVector3d& vector2,
                     ResourceVector2d& zoom,
                     ResourceVector2ui& renderingSize,
                     ResourceVolumeIntensities& intensities,
                     ResourceLuts& luts ) : _orderProvider( orderProvider ), _volumes( volumes ),
                     _origin( origin ), _vector1( vector1 ), _vector2( vector2 ), _zoom( zoom ),
                     _renderingSize( renderingSize ), _intensities( intensities ), _luts( luts ),
                     _mprComputation( orderProvider, _mprCombiner.isIdle(), volumes, origin, vector1, vector2, zoom, renderingSize ),
                     _mprCombiner( orderProvider, _mprComputation.outOrdersToFuse, intensities, luts, volumes )

      {
         attach( volumes );
         attach( origin );
         attach( vector1 );
         attach( vector2 );
         attach( zoom );
         attach( renderingSize );
         attach( intensities );
         attach( luts );

         // we are using this resource to force the update when new orders have been created.
         // If new Orders, the combiner should wait for the orders to execute and then
         // execute itself
         attach( _dummySynchResource );

         // we don't attach it as the resource will die before the engine does which will cause
         // problems when this object is deleted
         _mprComputation.outOrdersToFuse.setFather( &_dummySynchResource );
         _needToRecompute = false;
      }

      virtual void notify()
      {
         _mprComputation.notify();
         _mprCombiner.notify();
         _needToRecompute = true;
      }

      virtual void consume( Order* o )
      {
         _mprComputation.consume( o );
         _mprCombiner.consume( o );

         // update out resource if necesary
         if ( _mprCombiner.outFusedMPR.image.getBuf() != outFusedMpr.image.getBuf() && _mprCombiner.isIdle() )
         {
            outFusedMpr.image = _mprCombiner.outFusedMPR.image;
            outFusedMpr.notifyChanges();
         }
      }

      virtual bool _run()
      {
         _mprComputation.run();
         _mprCombiner.run();
         return !_mprCombiner.isNotified();
      }

      /**
       @brief Set zoom, orientation, position of the bigggest volume to be the center of the MPR
       */
      void autoFindPosition( Orientation orientation )
      {
         // find the biggest volume
         ui32 size = 0;
         ResourceVolumes::iterator choice;
         for ( ResourceVolumes::iterator it = _volumes.begin(); it != _volumes.end(); ++it )
         {
            ui32 volSize = (*it)->getSize()[ 0 ] * (*it)->getSize()[ 1 ] * (*it)->getSize()[ 2 ];
            if ( volSize > size )
            {
               size = volSize;
               choice = it;
            }
         }

         // select the orientation
         if ( orientation == CORONAL )
         {
            _zoom.setValue( 0, 1 );
            _zoom.setValue( 1, 1 );

            // we have to normalize the vectors
            nll::core::vector3d value1( (*choice)->getPst()( 0, 1 ),
                                        (*choice)->getPst()( 1, 1 ),
                                        (*choice)->getPst()( 2, 1 ) );
            value1.div( value1.norm2() );

            nll::core::vector3d value2( (*choice)->getPst()( 0, 2 ),
                                        (*choice)->getPst()( 1, 2 ),
                                        (*choice)->getPst()( 2, 2 ) );
            value2.div( value2.norm2() );

            _vector1.setValue( value1 );
            _vector2.setValue( value2 );
            nll::core::vector3d pos = (*choice)->indexToPosition( nll::core::vector3d( (*choice)->getSize()[ 0 ] / 2 + 0.5,
                                                                                       (*choice)->getSize()[ 1 ] / 2 + 0.5,
                                                                                       (*choice)->getSize()[ 2 ] / 2 + 0.5 ) );
            _origin.setValue( pos );
            return;
         }

         if ( orientation == TRANSVERSE )
         {
            _zoom.setValue( 0, 1 );
            _zoom.setValue( 1, 1 );

            // we have to normalize the vectors
            nll::core::vector3d value1( (*choice)->getPst()( 0, 0 ),
                                        (*choice)->getPst()( 1, 0 ),
                                        (*choice)->getPst()( 2, 0 ) );
            value1.div( value1.norm2() );

            nll::core::vector3d value2( -(*choice)->getPst()( 0, 1 ),
                                        -(*choice)->getPst()( 1, 1 ),
                                        -(*choice)->getPst()( 2, 1 ) );
            value2.div( value2.norm2() );

            _vector1.setValue( value1 );
            _vector2.setValue( value2 );

            nll::core::vector3d pos = (*choice)->indexToPosition( nll::core::vector3d( (*choice)->getSize()[ 0 ] / 2 + 0.5,
                                                                                       (*choice)->getSize()[ 1 ] / 2 + 0.5,
                                                                                       (*choice)->getSize()[ 2 ] / 2 + 0.5 ) );
            _origin.setValue( pos );
            return;
         }

         if ( orientation == FRONTAL )
         {
            _zoom.setValue( 0, 1 );
            _zoom.setValue( 1, 1 );

            // we have to normalize the vectors
            nll::core::vector3d value1( (*choice)->getPst()( 0, 0 ),
                                        (*choice)->getPst()( 1, 0 ),
                                        (*choice)->getPst()( 2, 0 ) );
            value1.div( value1.norm2() );

            nll::core::vector3d value2( (*choice)->getPst()( 0, 2 ),
                                        (*choice)->getPst()( 1, 2 ),
                                        (*choice)->getPst()( 2, 2 ) );
            value2.div( value2.norm2() );

            _vector1.setValue( value1 );
            _vector2.setValue( value2 );
            nll::core::vector3d pos = (*choice)->indexToPosition( nll::core::vector3d( (*choice)->getSize()[ 0 ] / 2 + 0.5,
                                                                                       (*choice)->getSize()[ 1 ] / 2 + 0.5,
                                                                                       (*choice)->getSize()[ 2 ] / 2 + 0.5 ) );
            _origin.setValue( pos );
            return;
         }
      }

   protected:
      OrderProvider&                _orderProvider;
      ResourceVolumes&              _volumes;
      ResourceVector3d&             _origin;
      ResourceVector3d&             _vector1;
      ResourceVector3d&             _vector2;
      ResourceVector2d&             _zoom;
      ResourceVector2ui&            _renderingSize;
      ResourceVolumeIntensities&    _intensities;
      ResourceLuts&                 _luts;

      DynamicResource               _dummySynchResource;

      EngineMprComputation          _mprComputation;
      EngineMprCombiner             _mprCombiner;
   };
}

#endif
