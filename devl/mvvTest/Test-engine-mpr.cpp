#include "stdafx.h"
#include <mvv/engine-mpr.h>
#include <mvv/resource.h>
#include <mvv/queue-orders.h>

using namespace mvv;

class OrderProviderTest : public OrderProvider
{
public:
   virtual Orders getOrdersAndClear()
   {
      std::cout << "getOrder" << std::endl;
      Orders o = orders;
      orders.clear();
      return o;
   }

   void pushOrder( Order* o )
   {
      std::cout << "push order=" << o->getId() << " t=" << o->getOrderClassId() << std::endl;
      orders.push_back( o );
   }

   virtual Orders getOrdersToDestroyAndClear()
   {
      return Orders();
   }

   virtual void pushOrderToDestroy( Order* )
   {
   }

   Orders orders;
};

namespace
{
   void wait( double seconds )
   {
     clock_t endwait;
     endwait = clock () + seconds * CLOCKS_PER_SEC ;
     while (clock() < endwait) {}
   }
}

class TestEngineMpr
{
public:
   void test1()
   {
      // load volumes
      const std::string path_pet = MVV_RESOURCE_PATH "1_-NAC.mf2";
      const std::string path_ct = MVV_RESOURCE_PATH "1_-CT.mf2";
      MedicalVolume volumePet;
      MedicalVolume volumeCt;

      bool loaded = nll::imaging::loadSimpleFlatFile( path_pet, volumePet );
      ensure( loaded, "error" );

      loaded = nll::imaging::loadSimpleFlatFile( path_ct, volumeCt );
      ensure( loaded, "error" );

      // set up resources
      ResourceVolumes volumes;
      ResourceVector3d origin( -150, -150, 0 );
      ResourceVector3d vector1( 1, 0, 0 );
      ResourceVector3d vector2( 0, 1, 0 );
      ResourceVector2d zoom( 5, 5 );
      ResourceVector2ui renderingSize( 1024, 1024 );
      ResourceVolumeIntensities intensities;
      ResourceLuts luts;
      ResourceTransferFunctionWindowing   lutCt;
      ResourceTransferFunctionWindowing   lutPet;

      // TODO update
      luts.addLut( &volumeCt, &lutCt );
      luts.addLut( &volumePet, &lutPet );
      volumes.attachVolume( &volumeCt );
      volumes.attachVolume( &volumePet );
      intensities.addIntensity( &volumeCt, 0.5 );
      intensities.addIntensity( &volumePet, 0.5 );

      lutCt.setMinWindow( -2000 );
      lutCt.setMaxWindow( 1000 );
      lutPet.setMinWindow( -1000 );
      lutPet.setMaxWindow( 5000 );
      unsigned char red[] = { 255, 0, 0 };
      lutPet.setLutColor( red );

      // init threading...
      OrderProviderTest orderProvider;
      QueueOrder        queue( orderProvider, 2 );
      boost::thread     dispatchThread( boost::ref( queue ) );

      // run engine
      EngineMprImpl engine( orderProvider,
                            volumes,
                            origin,
                            vector1,
                            vector2,
                            zoom,
                            renderingSize,
                            intensities,
                            luts );
      TESTER_ASSERT( !engine.isNotified() );
      engine.notify();

      std::cout << "isNotified=" << engine.isNotified() << std::endl;

      // create the orders
      engine.run();
      
      // notify the queue: orders will be launched
      queue.notify();

      // render the MPR
      wait( 1 );
      QueueOrder::OrderBuffer orders = queue.getFinishedOrdersAndClear();
      TESTER_ASSERT( orders.size() == 2 ); // 2 volumes => 2 MPR to render
      for ( QueueOrder::OrderBuffer::iterator it = orders.begin(); it != orders.end(); ++it )
      {
         engine.consume( *it );
      }

      engine.run();
      queue.notify();

      // fuse the MPR
      wait( 1 );
      orders = queue.getFinishedOrdersAndClear();
      TESTER_ASSERT( orders.size() == 1 );   // fuse order
      for ( QueueOrder::OrderBuffer::iterator it = orders.begin(); it != orders.end(); ++it )
      {
         engine.consume( *it );
      }

      nll::core::writeBmp( engine.outFusedMpr.image, "a.bmp" );
      TESTER_ASSERT( !engine.isNotified() );
      TESTER_ASSERT( engine.outFusedMpr.image.sizex() == 1024 && engine.outFusedMpr.image.sizey() == 1024 );

      queue.kill();
      dispatchThread.interrupt();
      dispatchThread.join();
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestEngineMpr);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif