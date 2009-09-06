#ifndef MVV_INIT_H_
# define MVV_INIT_H_

# include <mvv/layout-3-mpr.h>
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
      Display3Mpr*                        mprs;
      QueueOrder*                         queue;
      Pane*                               rootLayout;

      boost::thread                       dispatchThread;

      MedicalVolume                       TODOREMOVE_volume;
      MedicalVolume                       TODOREMOVE_volume2;
      ResourceTransferFunctionWindowing   windowing;
      ResourceTransferFunctionWindowing   windowing2;

      unsigned int                        screenTextureId;
      nll::core::Image<nll::ui8>          screen;

      ApplicationVariables()
      {
         mprs = new Display3Mpr( ResourceManager::instance() );

         // load volumes
         const std::string pathV2 = "../../nllTest/data/medical/pet-NAC.mf2";
         const std::string pathV1 = "../../nllTest/data/medical/CT1orig.mf2";
         bool loaded;

         loaded = nll::imaging::loadSimpleFlatFile( pathV1, TODOREMOVE_volume );
         mprs->getVolumes().attachVolume( &TODOREMOVE_volume, 0.1, &windowing );
         ensure( loaded, "error" );

         loaded = nll::imaging::loadSimpleFlatFile( pathV2, TODOREMOVE_volume2 );
         mprs->getVolumes().attachVolume( &TODOREMOVE_volume2, 0.9, &windowing2 );
         ensure( loaded, "error" );


         // set zoom factors
         //zoom.setValue( 0, 3 );
         //zoom.setValue( 1, 3 );

         // set default windowing
         windowing.setMinWindow( -1000 );
         windowing.setMaxWindow( 1000 );
         windowing2.setMinWindow( -1000 );
         windowing2.setMaxWindow( 5000 );

         // set the MPR1
         /*
         originMpr1.setValue( 0, 0 );
         originMpr1.setValue( 1, 0 );
         originMpr1.setValue( 2, 43 );
         mpr1V1.setValue( 0, 1 );
         mpr1V1.setValue( 1, 0 );
         mpr1V1.setValue( 2, 0 );
         mpr1V2.setValue( 0, 0 );
         mpr1V2.setValue( 1, 1 );
         mpr1V2.setValue( 2, 0 );

         mpr1 = new EngineMpr( ResourceManager::instance(), volumes, originMpr1, mpr1V1, mpr1V2, zoom );

         engines.insert( mpr1 );

         // layout
         ui32 sizex = 1024;
         ui32 sizey = 1024;
         PaneDrawable* layout1 = new PaneDrawable( *mpr1, nll::core::vector2ui( 0, 0 ), nll::core::vector2ui( sizex, sizey ) );
         layout1->updateLayout();
         rootLayout = layout1;
         */

         ui32 sizex = 1024;
         ui32 sizey = 1024;

         rootLayout = mprs->getLayout();
         rootLayout->setOrigin( nll::core::vector2ui( 0, 0 ) );
         rootLayout->setSize( nll::core::vector2ui( sizex, sizey ) );
         rootLayout->updateLayout();
         mprs->_computeAutoAdjustSize();

         // queue
         queue = new QueueOrder( ResourceManager::instance(), 10 );
         dispatchThread = boost::thread( boost::ref( *queue ) );
         
         ResourceManager::instance().setQueueOrder( queue );

         // screen
         screen = nll::core::Image<nll::ui8>( sizex, sizey, 3 );
      }

      void handleOrders()
      {
         QueueOrder::OrderBuffer orders = queue->getFinishedOrdersAndClear();
         mprs->consume( orders );
      }

      void runEngines()
      {
         mprs->checkStatus();
      }
   };
}

#endif
