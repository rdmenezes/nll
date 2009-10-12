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

      Symbol      volume1_pet;
      Symbol      volume2_ct;

      ApplicationVariables() : volume1_pet( Symbol::create("volume1_pet") ), 
                               volume2_ct( Symbol::create("volume2_ct") )
      {
         bool loaded;

         // load volumes
         ContextGlobalResource* globalContext = new ContextGlobalResource();
         Context::instance().add( globalContext );

         const std::string pathPet = "../../nllTest/data/medical/1_-NAC.mf2";
         MedicalVolume *pet = new MedicalVolume();
         loaded = nll::imaging::loadSimpleFlatFile( pathPet, *pet );
         ensure( loaded, "error" );

         const std::string pathCt = "../../nllTest/data/medical/1_-CT.mf2";
         MedicalVolume *ct = new MedicalVolume();
         loaded = nll::imaging::loadSimpleFlatFile( pathCt, *ct );
         ensure( loaded, "error" );

         std::cout << "s=" << ct->getSize()[ 0 ] << "," << ct->getSize()[ 1 ] << "," << ct->getSize()[ 2 ] << std::endl;
         std::cout << "o=" << ct->getOrigin()[ 0 ] << "," << ct->getOrigin()[ 1 ] << "," << ct->getOrigin()[ 2 ] << std::endl;
         // create a MPR
         ContextMpr* mprContext = new ContextMpr();
         Context::instance().add( mprContext );
         ContextMpr::ContextMprInstance* mpr1Context = new ContextMpr::ContextMprInstance();
         mprContext->addMpr( Symbol::create("mpr1_frontal"), mpr1Context );

         ResourceTransferFunctionWindowing* petLut = new ResourceTransferFunctionWindowing( 0, 5000);
         petLut->setLutColor( nll::core::Image<nll::ui8>::red() );
         mpr1Context->addVolume( pet, 0.5, petLut );
         mpr1Context->addVolume( ct, 0.5, new ResourceTransferFunctionWindowing( 0, 1000) );
         mpr1Context->origin.setValue( 0, -80 );
         mpr1Context->origin.setValue( 1, -80 );
         mpr1Context->origin.setValue( 2, 0 );
         mpr1Context->vector1.setValue( 0, 0 );
         mpr1Context->vector1.setValue( 1, 1 );
         mpr1Context->vector1.setValue( 2, 0 );
         mpr1Context->vector2.setValue( 0, 1 );
         mpr1Context->vector2.setValue( 1, 0 );
         mpr1Context->vector2.setValue( 2, 0 );
         mpr1Context->zoom.setValue( 0, 2 );
         mpr1Context->zoom.setValue( 1, 2 );

         DrawableMprToolkits* toolkits = new DrawableMprToolkits( ResourceManager::instance(),
                                                                  mpr1Context->volumes,
                                                                  mpr1Context->origin,
                                                                  mpr1Context->vector1,
                                                                  mpr1Context->vector2,
                                                                  mpr1Context->zoom,
                                                                  mpr1Context->volumeIntensities,
                                                                  mpr1Context->luts );
         mpr1Context->setDrawableMprToolkits( toolkits );
         globalContext->addOrderCreator( toolkits );


         // create layout
         ui32 sizex = 512;
         ui32 sizey = 512;
         rootLayout = new PaneListHorizontal( nll::core::vector2ui( 0, 0 ),
                                              nll::core::vector2ui( sizex, sizey ) );
         rootLayout->addChild( new PaneDrawableEmpty( nll::core::vector2ui( 0, 0 ),
                                                      nll::core::vector2ui( 0, 0 ) ),
                               0.001 );
         PaneMpr* mpr = new PaneMpr( *toolkits,
                                     nll::core::vector2ui( 0, 0 ),
                                     nll::core::vector2ui( 0, 0 ) );
         rootLayout->addChild( mpr, 0.999 );
         rootLayout->updateLayout();

         mpr1Context->origin.notifyChanges();

         //PaneDrawable* layout1 = new PaneDrawable( *mpr1, nll::core::vector2ui( 0, 0 ), nll::core::vector2ui( sizex, sizey ) );


         /*

         // set zoom factors
         //zoom.setValue( 0, 3 );
         //zoom.setValue( 1, 3 );

         // set default windowing
         windowing.setMinWindow( -2000 );
         windowing.setMaxWindow( 1000 );
         windowing2.setMinWindow( -1000 );
         windowing2.setMaxWindow( 5000 );
         unsigned char red[] = { 255, 0, 0 };
         windowing2.setLutColor( red );
         */
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


/*
         rootLayout = mprs->getLayout();
         rootLayout->setOrigin( nll::core::vector2ui( 0, 0 ) );
         rootLayout->setSize( nll::core::vector2ui( sizex, sizey ) );
         rootLayout->updateLayout();
         mprs->autoAdjustSize();
*/
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
