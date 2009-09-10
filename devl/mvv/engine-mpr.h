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
      OrderCombineMpr( const Orders& predecessors, Image& slice, ResourceVolumes& volumes ) : Order( ORDER_MPR_RENDERING_COMBINE, true, makePredecessors( predecessors ) ),
         _tracked( predecessors ), _volumes( volumes ), _slice( slice )
      {
      }

      virtual OrderResult* run()
      {
         ui32 n = 0;
         double ratio = 0;
         for ( ResourceVolumes::const_iterator it = _volumes.begin(); it != _volumes.end(); ++it, ++n )
         {
            const OrderMprRenderingResult* slice = dynamic_cast<const OrderMprRenderingResult*>( _tracked[ n ]->getResult() );
            OrderMprRendering::Slice::DirectionalIterator itIn = slice->slice.beginDirectional();
            Image::DirectionalIterator itOut = _slice.beginDirectional();
            ResourceTransferFunctionWindowing* windowing = it->windowing;
            for ( ; itOut != _slice.endDirectional(); ++itIn, ++itOut )
            {
               const ui8* buf = windowing->transform( *itIn );
               itOut.pickcol( 0 ) += buf[ 0 ] * it->ratio;
               itOut.pickcol( 1 ) += buf[ 1 ] * it->ratio;
               itOut.pickcol( 2 ) += buf[ 2 ] * it->ratio;
            }
            ratio += it->ratio;
         }
         ensure( fabs( ratio - 1 ) < 1e-5, "ratio must sum to 1" );
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

      const Orders&                       _tracked;
      ResourceVolumes&                    _volumes;
      Image&                              _slice;
   };

   /**
    @ingroup mvv
    @brief A multiplanar reconstruction object
    */
   class EngineMpr : public Engine, public Drawable
   {
      typedef std::set<MedicalVolume*> Volumes;
      typedef std::vector<Order*>      Orders;

   public:
      EngineMpr( OrderProvider& orderProvider,
                 ResourceVolumes& volumes,
                 ResourceVector3d& origin,
                 ResourceVector3d& vector1,
                 ResourceVector3d& vector2,
                 ResourceVector2d& zoom ) : _orderProvider( orderProvider ), _sx( 0 ), _sy( 0 ), _volumes( volumes ),
                 _origin( origin ), _vector1( vector1 ), _vector2( vector2 ), _zoom( zoom )
      {
         attach( origin );
         attach( vector1 );
         attach( vector2 );
         attach( zoom );
      }

      /**
       @brief Consume an order
       */
      virtual void consume( Order* o )
      {
         if ( ( o->getOrderClassId() != ORDER_MPR_RENDERING && o->getOrderClassId() != ORDER_MPR_RENDERING_COMBINE ) || !_tracked.size() )
            return;

         // it is a MPR_RENDERING order so we need to check it is one of our order
         for ( ui32 n = 0; n < _tracked.size(); ++n )
            if ( !_tracked[ n ]->getResult() )
            {
               // one order has not finished yet so we need to wait
               return;
            }
            std::cout << (double)clock() / CLOCKS_PER_SEC << " MPR result found" << std::endl;

         // all the orders are finished, just fuse them and update the result
         ensure( _tracked.size() == _volumes.size() + 1, "error size doesn't match!" );

         for ( Orders::iterator it = _tracked.begin(); it != _tracked.end(); ++it )
         {
         }

         _tracked.clear();
         _slice.clone( _sliceTmp );
         std::cout << (double)clock() / CLOCKS_PER_SEC << " MPR MPR finished" << std::endl;
      }

      /**
       @brief Compute a MPR
       */
      virtual bool _run()
      {
         if ( _tracked.size() )
         {
            std::cout << "waiting..." << _tracked.size()<< std::endl;
            return false;
         }
         _sliceTmp = Image( _sx, _sy, 3, true );
         _tracked = Orders( _volumes.size() );
         std::cout << (double)clock() / CLOCKS_PER_SEC << " MPR STARTED RUN ORDER" << std::endl;

         ui32 n = 0;
         for ( ResourceVolumes::const_iterator it = _volumes.begin(); it != _volumes.end(); ++it, ++n )
         {
            OrderMprRendering* order = new OrderMprRendering( it->volume, _sx, _sy, _zoom[ 0 ], _zoom[ 1 ],
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
         OrderCombineMpr* order = new OrderCombineMpr( _tracked, _sliceTmp, _volumes );
         _orderProvider.pushOrder( order );
         _tracked.push_back( order );
         return true;
      }


      /**
       @brief Return a MPR fully processed and ready to be drawed
       */
      virtual const Image& draw()
      {
         if ( _slice.sizex() != _sx || _slice.sizey() != _sy )
         {            
            // for now just resample the image... we have to wait for the updated asynchronous order
            nll::core::rescaleBilinear( _slice, _sx, _sy );
         }
         return _slice;
      }

      /**
       @brief Specifies the size of the MPR in pixel
       */
      virtual void setImageSize( ui32 sx, ui32 sy )
      {
         _sx = sx;
         _sy = sy;

         _slice = Image( _sx, _sy, 3, true );
         _sliceTmp = Image( _sx, _sy, 3, true );
      }

   protected:
      OrderProvider& _orderProvider;
      Image          _slice;
      Image          _sliceTmp;
      ui32           _sx;
      ui32           _sy;

      ResourceVolumes&                    _volumes;
      ResourceVector3d&                   _origin;
      ResourceVector3d&                   _vector1;
      ResourceVector3d&                   _vector2;
      ResourceVector2d&                   _zoom;

      Orders   _tracked;
   };
}

#endif
