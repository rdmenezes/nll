#ifndef MVV_LAYOUT_3_MPR_H_
# define MVV_LAYOUT_3_MPR_H_

# include "layout.h"
# include "drawable.h"
# include "resource.h"
# include "engine-mpr.h"
# include "order-provider.h"

namespace mvv
{
   /**
    @brief Display 3 MPR & 3d view
    */
   class Display3Mpr
   {
      typedef std::set<Engine*>           Engines;

   public:
      typedef std::vector<Order*>         Orders;

   public:
      Display3Mpr( OrderProvider& orderProvider )
      {
         // create the MPRs
         _mpr00 = new EngineMpr( orderProvider,
                                 _volumes,
                                 _origin00,
                                 _v100,
                                 _v010,
                                 _zoom );
         _mpr10 = new EngineMpr( orderProvider,
                                 _volumes,
                                 _origin10,
                                 _v100,
                                 _v001,
                                 _zoom );
         _mpr01 = new EngineMpr( orderProvider,
                                 _volumes,
                                 _origin01,
                                 _v010,
                                 _v001,
                                 _zoom );

         // create the layout
         PaneListHorizontal* root = new PaneListHorizontal( nll::core::vector2ui( 0, 0 ),
                                                            nll::core::vector2ui( 10, 10 ) );
         PaneListVertical* vertical_l = new PaneListVertical( nll::core::vector2ui( 0, 0 ),
                                                              nll::core::vector2ui( 10, 10 ) );
         PaneListVertical* vertical_r = new PaneListVertical( nll::core::vector2ui( 0, 0 ),
                                                              nll::core::vector2ui( 10, 10 ) );
         PaneDrawable* mpr00 = new PaneDrawable( *_mpr00, nll::core::vector2ui( 0, 0 ),
                                                          nll::core::vector2ui( 10, 10 ) );
         PaneDrawable* mpr10 = new PaneDrawable( *_mpr10, nll::core::vector2ui( 0, 0 ),
                                                          nll::core::vector2ui( 10, 10 ) );
         PaneDrawable* mpr01 = new PaneDrawable( *_mpr01, nll::core::vector2ui( 0, 0 ),
                                                          nll::core::vector2ui( 10, 10 ) );
         PaneDrawableEmpty* empty = new PaneDrawableEmpty( nll::core::vector2ui( 0, 0 ),
                                                           nll::core::vector2ui( 10, 10 ) );
         vertical_l->addChild( mpr00, 0.5 );
         vertical_l->addChild( mpr01, 0.5 );
         vertical_r->addChild( mpr10, 0.5 );
         vertical_r->addChild( empty, 0.5 );

         root->addChild( vertical_l, 0.5 );
         root->addChild( vertical_r, 0.5 );
         _rootLayout = root;

         // the default vectors for MPR
         _v100.setValue( 0, 1 );
         _v100.setValue( 1, 0 );
         _v100.setValue( 2, 0 );
         
         _v010.setValue( 0, 0 );
         _v010.setValue( 1, 1 );
         _v010.setValue( 2, 0 );

         _v001.setValue( 0, 0 );
         _v001.setValue( 1, 0 );
         _v001.setValue( 2, 1 );

         _zoom.setValue( 0, 0.5 );
         _zoom.setValue( 1, 0.5 );

         // registers the engines contained by this object
         _engines.insert( _mpr00 );
         _engines.insert( _mpr10 );
         _engines.insert( _mpr01 );
      }

      /**
       @brief Return the volume list (and their lifecycle) attached to the MPR
       */
      ResourceVolumes& getVolumes()
      {
         return _volumes;
      }

      /**
       @brief Returns the layout. This layout should be appended to another layout where it will be held.
       */
      Pane* getLayout() const
      {
         return _rootLayout;
      }

      /**
       @brief Set the MPR position
       */
      void setMprPosition( const nll::core::vector3d& pos )
      {
         _origin01.setValue( 0, pos[ 0 ] );
         _origin10.setValue( 1, pos[ 1 ] );
         _origin00.setValue( 2, pos[ 2 ] );
      }

      /**
       @brief Get the MPR position
       */
      nll::core::vector3d getMprPosition() const
      {
         return nll::core::vector3d( _origin01.getValue( 0 ),
                                     _origin10.getValue( 1 ),
                                     _origin00.getValue( 2 ) );
      }

      /**
       @brief Consume orders
       */
      void consume( Orders& orders )
      {
         for ( Engines::iterator it = _engines.begin(); it != _engines.end(); ++it )
         {
            for ( Orders::iterator oit = orders.begin(); oit != orders.end(); ++oit )
               (*it)->consume( *oit );
         }
      }

      /**
       @brief Check the status of the MPR. If anything changed, this method will start updating them
              (i.e. starting orders)
       */
      void checkStatus()
      {
         for ( Engines::iterator it = _engines.begin(); it != _engines.end(); ++it )
         {
            (*it)->run();
         }
      }

      /**
       @brief Return the zoom shared by the 3 MPRs
       */
      nll::core::vector2d getZoom() const
      {
         return nll::core::vector2d( _zoom[ 0 ], _zoom[ 1 ] );
      }

      /**
       @brief Return the zoom shared by the 3 MPRs
       */
      void setZoom( const nll::core::vector2d& z )
      {
         ensure( z[ 0 ] > 0 && z[ 1 ] > 0, "only positive numbers" );

         //_origin00.setValue( 0, _origin00[ 0 ] + ( z[ 0 ] - _zx ) * _originDev00[ 0 ] );
         //_origin00.setValue( 1, _origin00[ 1 ] + ( z[ 1 ] - _zy ) * _originDev00[ 1 ] );
         //_origin00.setValue( 2, -itmax->volume->getOrigin()[ 2 ] + _originDev00[ 2 ] );
/*
         _origin10.setValue( 0, -itmax->volume->getOrigin()[ 0 ] + _originDev10[ 0 ] );
         _origin10.setValue( 1, -itmax->volume->getOrigin()[ 1 ] + _originDev10[ 1 ] );
         _origin10.setValue( 2, -itmax->volume->getOrigin()[ 2 ] + _originDev10[ 2 ] );

         _origin01.setValue( 0, -itmax->volume->getOrigin()[ 0 ] + _originDev01[ 0 ] );
         _origin01.setValue( 1, -itmax->volume->getOrigin()[ 1 ] + _originDev01[ 1 ] );
         _origin01.setValue( 2, -itmax->volume->getOrigin()[ 2 ] + _originDev01[ 2 ] );
*/
         _zoom.setValue( 0, z[ 0 ] );
         _zoom.setValue( 1, z[ 1 ] );
      }

      /**
       @brief Center the MPRs on the biggest volume, and adjust the zoom so that the bigger MPR fits the screen
       */
      void autoAdjustSize()
      {
         // look for the biggest volume and display its middle
         double max = 0;
         ResourceVolumes::const_iterator itmax;
         for ( ResourceVolumes::const_iterator it = _volumes.begin(); it != _volumes.end(); ++it )
         {

            double val = it->volume->getSize()[ 0 ] *
                         it->volume->getSize()[ 1 ] *
                         it->volume->getSize()[ 2 ] *
                         it->volume->getSpacing()[ 0 ] *
                         it->volume->getSpacing()[ 1 ] *
                         it->volume->getSpacing()[ 2 ];
            if ( val > max )
            {
               max = val;
               itmax = it;
            }
         }

         // select the new base vector : we want to be in the same base than the biggest volume
         _v100.setValue( 0, itmax->volume->getPst()( 0, 0 ) );
         _v100.setValue( 1, itmax->volume->getPst()( 1, 0 ) );
         _v100.setValue( 2, itmax->volume->getPst()( 2, 0 ) );
         _v100.normalize();

         _v010.setValue( 0, itmax->volume->getPst()( 0, 1 ) );
         _v010.setValue( 1, itmax->volume->getPst()( 1, 1 ) );
         _v010.setValue( 2, itmax->volume->getPst()( 2, 1 ) );
         _v010.normalize();

         _v001.setValue( 0, itmax->volume->getPst()( 0, 2 ) );
         _v001.setValue( 1, itmax->volume->getPst()( 1, 2 ) );
         _v001.setValue( 2, itmax->volume->getPst()( 2, 2 ) );
         _v001.normalize();

         // compute a ratio so that the biggest side on MPR is fully displayed
         double sx0 = itmax->volume->getSize()[ 0 ] * itmax->volume->getSpacing()[ 0 ];
         double sy0 = itmax->volume->getSize()[ 1 ] * itmax->volume->getSpacing()[ 1 ];

         double sx1 = itmax->volume->getSize()[ 0 ] * itmax->volume->getSpacing()[ 0 ];
         double sy1 = itmax->volume->getSize()[ 2 ] * itmax->volume->getSpacing()[ 2 ];

         double sx2 = itmax->volume->getSize()[ 1 ] * itmax->volume->getSpacing()[ 1 ];
         double sy2 = itmax->volume->getSize()[ 2 ] * itmax->volume->getSpacing()[ 2 ];

         const nll::core::vector2ui size = _rootLayout->getSize();

         // we want zoom to be 1 to the largest border
         double zx = ( size[ 0 ] / 2 ) / sx0;
         double zy = ( size[ 1 ] / 2 ) / sy0;
         double zoomFactor = std::min( zx, zy ) * 2;

         // compute the initial deviation
         double spx1 = size[ 0 ] / 2 - sx0 * zoomFactor;
         double spy1 = size[ 1 ] / 2 - sy0 * zoomFactor;

         double spx2 = size[ 0 ] / 2 - sx1 * zoomFactor;
         double spy2 = size[ 1 ] / 2 - sy1 * zoomFactor;

         double spx3 = size[ 0 ] / 2 - sx2 * zoomFactor;
         double spy3 = size[ 1 ] / 2 - sy2 * zoomFactor;

         // save the new deviation (in case a volume is unloaded we want to keep track of the original position)
         _originDev00[ 0 ] = - spx1 / ( 2 * zoomFactor );
         _originDev00[ 1 ] = - spy1 / ( 2 * zoomFactor );
         _originDev00[ 2 ] = + sy1 / 2;

         _originDev10[ 0 ] = - spx2 / ( 2 * zoomFactor );
         _originDev10[ 1 ] = + sy0 / 2;
         _originDev10[ 2 ] = - spy2 / ( 2 * zoomFactor );

         _originDev01[ 0 ] = + sx0 / 2;
         _originDev01[ 1 ] = - spx3 / ( 2 * zoomFactor );
         _originDev01[ 2 ] = - spy3 / ( 2 * zoomFactor );

         // set the updated center to the center of the largest volume
         _origin00.setValue( 0, -itmax->volume->getOrigin()[ 0 ] + _originDev00[ 0 ] );
         _origin00.setValue( 1, -itmax->volume->getOrigin()[ 1 ] + _originDev00[ 1 ] );
         _origin00.setValue( 2, -itmax->volume->getOrigin()[ 2 ] + _originDev00[ 2 ] );

         _origin10.setValue( 0, -itmax->volume->getOrigin()[ 0 ] + _originDev10[ 0 ] );
         _origin10.setValue( 1, -itmax->volume->getOrigin()[ 1 ] + _originDev10[ 1 ] );
         _origin10.setValue( 2, -itmax->volume->getOrigin()[ 2 ] + _originDev10[ 2 ] );

         _origin01.setValue( 0, -itmax->volume->getOrigin()[ 0 ] + _originDev01[ 0 ] );
         _origin01.setValue( 1, -itmax->volume->getOrigin()[ 1 ] + _originDev01[ 1 ] );
         _origin01.setValue( 2, -itmax->volume->getOrigin()[ 2 ] + _originDev01[ 2 ] );

         _zx = zoomFactor;
         _zy = zoomFactor;

         _zoom.setValue( 0, zoomFactor );
         _zoom.setValue( 1, zoomFactor );
      }

   protected:
      ResourceVolumes   _volumes;
      ResourceVector3d  _origin00;
      ResourceVector3d  _origin01;
      ResourceVector3d  _origin10;
      ResourceVector3d  _v100;
      ResourceVector3d  _v010;
      ResourceVector3d  _v001;
      ResourceVector2d  _zoom;

      nll::core::vector3d  _originDev00;
      nll::core::vector3d  _originDev10;
      nll::core::vector3d  _originDev01;

      EngineMpr*        _mpr00;
      EngineMpr*        _mpr10;
      EngineMpr*        _mpr01;
      Engines           _engines;
      Pane*             _rootLayout;

      double sx0, sy0, sx1, sy1, sx2, sy2;
      double _zx, _zy;
   };
}

#endif
