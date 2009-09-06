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
         /*
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
                                 */
         
         
         _mpr00 = new EngineMpr( orderProvider,
                                 _volumes,
                                 _origin00,
                                 _v100,
                                 _v010,
                                 _zoom );
         _mpr10 = new EngineMpr( orderProvider,
                                 _volumes,
                                 _origin00,
                                 _v100,
                                 _v010,
                                 _zoom );
         _mpr01 = new EngineMpr( orderProvider,
                                 _volumes,
                                 _origin00,
                                 _v100,
                                 _v010,
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

         _engines.insert( _mpr00 );
         _engines.insert( _mpr10 );
         _engines.insert( _mpr01 );
      }

      ResourceVolumes& getVolumes()
      {
         return _volumes;
      }

      Pane* getLayout() const
      {
         return _rootLayout;
      }

      void setMprPosition( const nll::core::vector3d& pos )
      {
         _origin01.setValue( 0, pos[ 0 ] );
         _origin10.setValue( 1, pos[ 1 ] );
         _origin00.setValue( 2, pos[ 2 ] );
      }

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

  // protected:
      void _computeAutoAdjustSize()
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


         double sx = itmax->volume->getSize()[ 0 ] * itmax->volume->getSpacing()[ 0 ];
         double sy = itmax->volume->getSize()[ 1 ] * itmax->volume->getSpacing()[ 1 ];
         double sz = itmax->volume->getSize()[ 2 ] * itmax->volume->getSpacing()[ 2 ];

         const nll::core::vector2ui size = _rootLayout->getSize();

         _origin00.setValue( 0, -itmax->volume->getOrigin()[ 0 ] );
         _origin00.setValue( 1, -itmax->volume->getOrigin()[ 1 ] );
         _origin00.setValue( 2, -itmax->volume->getOrigin()[ 2 ] );
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

      EngineMpr*        _mpr00;
      EngineMpr*        _mpr10;
      EngineMpr*        _mpr01;
      Engines           _engines;
      Pane*             _rootLayout;
   };
}

#endif
