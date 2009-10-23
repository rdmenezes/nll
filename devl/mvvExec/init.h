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
# include <mvv/context.h>
# include <mvv/context-global-resource.h>
# include <mvv/symbol.h>
# include <mvv/volume-loader.h>
# include <set>

namespace mvv
{
   /**
    @brief Initialize and hold resources of the application
    */
   struct ApplicationVariables
   {
      QueueOrder*                         queue;
      PaneListHorizontal*                 rootLayout;
      boost::thread                       dispatchThread;

      unsigned int                        screenTextureId;
      nll::core::Image<nll::ui8>          screen;
      mvv::InteractionEvent               events;

      Symbol      volume1_pet;
      Symbol      volume2_ct;

      ApplicationVariables() : volume1_pet( Symbol::create("volume1_pet") ), 
                               volume2_ct( Symbol::create("volume2_ct") )
      {
         events.mousePosition = nll::core::vector2ui( 0, 0 );
         events.mouseLeftClickedPosition = nll::core::vector2ui( 0, 0 );
         events.mouseLeftReleasedPosition = nll::core::vector2ui( 0, 0 );
         events.isMouseLeftButtonPressed = false;
         events.isMouseRightButtonPressed = false;

         bool loaded;

         // load volumes
         ContextGlobalResource* globalContext = new ContextGlobalResource();
         Context::instance().add( globalContext );

         
         MedicalVolume* ct = loadTextRawVolume( "../../nllTest/data/text-volume1/header.txt" );
         loaded = ct;
         
         
         /*
         const std::string pathPet = "../../nllTest/data/medical/1_-NAC.mf2";
         MedicalVolume *pet = new MedicalVolume();
         loaded = nll::imaging::loadSimpleFlatFile( pathPet, *pet );
         ensure( loaded, "error" );
         
         
         const std::string pathCt = "../../nllTest/data/medical/1_-CT.mf2";
         MedicalVolume *ct = new MedicalVolume();
         loaded = nll::imaging::loadSimpleFlatFile( pathCt, *ct );
         */

         /*
         for ( int nx = 0; nx < 60; ++nx )
            for ( int ny = 0; ny < 60; ++ny )
               (*ct)( nx, ny, ct->getSize()[ 2 ] / 2 ) = 999;
         for ( int nx = 0; nx < 10; ++nx )
            for ( int ny = 0; ny < 10; ++ny )
               (*ct)( 512 - 10 + nx, 512 - 10 + ny, ct->getSize()[ 2 ] / 2 ) = 999;
         for ( int nx = 0; nx < 10; ++nx )
            for ( int ny = 0; ny < 10; ++ny )
               (*ct)( 256 - 5 + nx, 256 - 5 + ny, ct->getSize()[ 2 ] / 2 ) = 999;
         */
         ensure( loaded, "error" );

         // create a MPR
         ContextMpr* mprContext = new ContextMpr();
         Context::instance().add( mprContext );

         //
         // MPR1
         //
         ContextMpr::ContextMprInstance* mpr1Context = new ContextMpr::ContextMprInstance();
         mprContext->addMpr( Symbol::create("mpr1_frontal"), mpr1Context );

         /*
         ResourceTransferFunctionWindowing* petLut = new ResourceTransferFunctionWindowing( 0, 5000);
         petLut->setLutColor( nll::core::Image<nll::ui8>::blue() );
         mpr1Context->addVolume( pet, 0.5, petLut );
         */
         mpr1Context->addVolume( ct, 1, new ResourceTransferFunctionWindowing( 100, 1100) );
         mpr1Context->origin.setValue( 0, -80 );
         mpr1Context->origin.setValue( 1, -80 );
         mpr1Context->origin.setValue( 2, 0 );
         mpr1Context->vector1.setValue( 0, 0 );
         mpr1Context->vector1.setValue( 1, 1 );
         mpr1Context->vector1.setValue( 2, 0 );
         mpr1Context->vector2.setValue( 0, 1 );
         mpr1Context->vector2.setValue( 1, 0 );
         mpr1Context->vector2.setValue( 2, 0 );
         mpr1Context->zoom.setValue( 0, 1 );
         mpr1Context->zoom.setValue( 1, 1 );

         DrawableMprToolkits* toolkits = new DrawableMprToolkits( ResourceManager::instance(),
                                                                  mpr1Context->volumes,
                                                                  mpr1Context->origin,
                                                                  mpr1Context->vector1,
                                                                  mpr1Context->vector2,
                                                                  mpr1Context->zoom,
                                                                  mpr1Context->volumeIntensities,
                                                                  mpr1Context->luts );
         MprToolkitMove* moveToolkit = new MprToolkitMove();
         MprToolkitPoint* pointToolkit = new MprToolkitPoint();
         toolkits->addToolkit( *moveToolkit );
         toolkits->addToolkit( *pointToolkit );
         mpr1Context->setDrawableMprToolkits( toolkits );
         globalContext->addOrderCreator( toolkits );

         //
         // MPR2
         //
         ContextMpr::ContextMprInstance* mpr2Context = new ContextMpr::ContextMprInstance();
         mprContext->addMpr( Symbol::create("mpr2_coronal"), mpr2Context );
         
         /*
         ResourceTransferFunctionWindowing* petLut2 = new ResourceTransferFunctionWindowing( 0, 5000);
         petLut2->setLutColor( nll::core::Image<nll::ui8>::red() );
         mpr2Context->addVolume( pet, 0.5, petLut2 );
         */         
         mpr2Context->addVolume( ct, 1, new ResourceTransferFunctionWindowing( 100, 1100) );
         mpr2Context->origin.setValue( 0, -80 );
         mpr2Context->origin.setValue( 1, -80 );
         mpr2Context->origin.setValue( 2, 0 );
         mpr2Context->vector1.setValue( 0, 1 );
         mpr2Context->vector1.setValue( 1, 0 );
         mpr2Context->vector1.setValue( 2, 0 );
         mpr2Context->vector2.setValue( 0, 0 );
         mpr2Context->vector2.setValue( 1, 0 );
         mpr2Context->vector2.setValue( 2, 1 );
         mpr2Context->zoom.setValue( 0, 1 );
         mpr2Context->zoom.setValue( 1, 1 );

         DrawableMprToolkits* toolkits2 = new DrawableMprToolkits( ResourceManager::instance(),
                                                                   mpr2Context->volumes,
                                                                   mpr2Context->origin,
                                                                   mpr2Context->vector1,
                                                                   mpr2Context->vector2,
                                                                   mpr2Context->zoom,
                                                                   mpr2Context->volumeIntensities,
                                                                   mpr2Context->luts );
         toolkits2->addToolkit( *moveToolkit );
         toolkits2->addToolkit( *pointToolkit );
         mpr2Context->setDrawableMprToolkits( toolkits2 );
         globalContext->addOrderCreator( toolkits2 );


         // create layout
         ui32 sizex = 1024;
         ui32 sizey = 1024;

         std::cout << "size=" << sizex << " " << sizey << std::endl;
         rootLayout = new PaneListHorizontal( nll::core::vector2ui( 0, 0 ),
                                              nll::core::vector2ui( sizex, sizey ) );
         rootLayout->addChild( new PaneDrawableEmpty( nll::core::vector2ui( 0, 0 ),
                                                      nll::core::vector2ui( 0, 0 ) ),
                               0.001 );
         PaneMpr* mpr = new PaneMpr( *toolkits,
                                     nll::core::vector2ui( 0, 0 ),
                                     nll::core::vector2ui( 0, 0 ) );
         PaneMpr* mpr2 = new PaneMpr( *toolkits2,
                                      nll::core::vector2ui( 0, 0 ),
                                      nll::core::vector2ui( 0, 0 ) );
         rootLayout->addChild( mpr, 0.499 );
         rootLayout->addChild( mpr2, 0.5 );
         rootLayout->updateLayout();

         mpr1Context->origin.notifyChanges();



         // queue
         queue = new QueueOrder( ResourceManager::instance(), 4 );
         dispatchThread = boost::thread( boost::ref( *queue ) );
           
         ResourceManager::instance().setQueueOrder( queue );

         // screen
         screen = nll::core::Image<nll::ui8>( sizex, sizey, 3 );
      }

      void handleOrders()
      {
         ContextGlobalResource* context = Context::instance().get<ContextGlobalResource>();
         ensure( context, "can't be null" );
         QueueOrder::OrderBuffer orders = queue->getFinishedOrdersAndClear();
         context->consume( orders );
      }

      void runEngines()
      {
         ContextGlobalResource* context = Context::instance().get<ContextGlobalResource>();
         context->run();
      }
   };
}

#endif
