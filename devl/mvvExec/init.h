#ifndef MVV_INIT_H_
# define MVV_INIT_H_

# include <mvv/queue-orders.h>
# include <mvv/resource-manager.h>
# include <mvv/order.h>
# include <mvv/thread-pool.h>
# include <mvv/resource.h>
# include <mvv/engine-mpr.h>
# include <mvv/layout.h>
# include <set>

namespace mvv
{
   /**
    @brief Initialize and hold resources of the application
    */
   struct ApplicationVariables
   {
      typedef std::set<Engine*>           Engines;


      ResourceVolumes                     volumes;
      ResourceVector2d                    zoom;
      ResourceTransferFunctionWindowing   windowing;

      ResourceVector3d                    originMpr1;
      ResourceVector3d                    mpr1V1;
      ResourceVector3d                    mpr1V2;
      EngineMpr*                          mpr1;

      QueueOrder*                         queue;
      Engines                             engines;
      Pane*                               rootLayout;

      boost::thread                       dispatchThread;

      MedicalVolume                       TODOREMOVE_volume;

      unsigned int                        screenTextureId;
      nll::core::Image<nll::ui8>          screen;

      ApplicationVariables()
      {
         // load volumes
         const std::string pathV1 = "C:/Users/Civo/Desktop/nll/nllTest/data/medical/pet-NAC.mf2";
         bool loaded = nll::imaging::loadSimpleFlatFile( pathV1, TODOREMOVE_volume );
         volumes.attachVolume( &TODOREMOVE_volume, 1 );
         ensure( loaded, "error" );

         // set zoom factors
         zoom.setValue( 0, 2 );
         zoom.setValue( 1, 2 );

         // set default windowing
         windowing.setMinWindow( -1000 );
         windowing.setMaxWindow( 5000 );

         // set the MPR1
         originMpr1.setValue( 0, 0 );
         originMpr1.setValue( 1, 0 );
         originMpr1.setValue( 2, 43 );
         mpr1V1.setValue( 0, 1 );
         mpr1V1.setValue( 1, 0 );
         mpr1V1.setValue( 2, 0 );
         mpr1V2.setValue( 0, 0 );
         mpr1V2.setValue( 1, 1 );
         mpr1V2.setValue( 2, 0 );
         mpr1 = new EngineMpr( ResourceManager::instance(), volumes, windowing, originMpr1, mpr1V1, mpr1V2, zoom );

         engines.insert( mpr1 );

         // layout
         PaneDrawable* layout = new PaneDrawable( *mpr1, nll::core::vector2ui( 0, 0 ), nll::core::vector2ui( 512, 512) );
         layout->updateLayout();
         rootLayout = layout;

         // queue
         queue = new QueueOrder( ResourceManager::instance(), 10 );
         dispatchThread = boost::thread( boost::ref( *queue ) );
         
         ResourceManager::instance().setQueueOrder( queue );

         // screen
         screen = nll::core::Image<nll::ui8>( 512, 512, 3 );
      }

      void handleOrders()
      {
         _ResourceManager::Orders orders = queue->getFinishedOrdersAndClear();
         for ( Engines::iterator it = engines.begin(); it != engines.end(); ++it )
         {
            for ( _ResourceManager::Orders::iterator oit = orders.begin(); oit != orders.end(); ++oit )
               (*it)->consume( *oit );
         }
      }
   };
}

#endif
