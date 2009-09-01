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
         std::cout << "--mpr started--" << std::endl;

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

         std::cout << "--mpr ended-- = " << t1.getCurrentTime() <<std::endl;
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
         if ( o->getOrderClassId() != ORDER_MPR_RENDERING || !_tracked.size() )
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
         
         _slice = Image( _sx, _sy, 3, true );
         for ( ui32 y = 0; y < _slice.sizey(); ++y )
            for ( ui32 x = 0; x < _slice.sizex(); ++x )
            {
               ui8 buf[ 4 ];
               ui8* out = _slice.point( x, y );
               ui32 n = 0;
               for ( ResourceVolumes::const_iterator it = _volumes.begin(); it != _volumes.end(); ++it, ++n )
               {
                  // it is assumed results' order is the same than volume's order
                  const OrderMprRenderingResult* slice = dynamic_cast<const OrderMprRenderingResult*>( _tracked[ n ]->getResult() );
                  const double val = slice->slice( x, y, 0 );
                  it->windowing->transform( val, buf );
                  for ( ui32 n = 0; n < 3; ++n )
                     out[ n ] += it->ratio * buf[ n ];
               }
            }

         // clear the orders, we don't need them
         _tracked.clear();

         // TODO REMOVE
         static int nbFps = 0;
         static unsigned last = clock();
         ++nbFps;

         if ( ( clock() - last ) / (double)CLOCKS_PER_SEC >= 1 )
         {
            std::cout << "----------------MPR_fps=" << nbFps << std::endl;
            nbFps = 0;
            last = clock();
         }
      }

      /**
       @brief Compute a MPR
       */
      virtual bool _run()
      {
         if ( _tracked.size() )
            return false;
         _tracked = Orders( _volumes.size() );

         ui32 n = 0;
         for ( ResourceVolumes::const_iterator it = _volumes.begin(); it != _volumes.end(); ++it, ++n )
         {
            std::cout << "mpr:order render" << std::endl;
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
      }

   protected:
      OrderProvider& _orderProvider;
      Image          _slice;
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
