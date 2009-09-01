#include "stdafx.h"
#include <mvv/order.h>
#include <mvv/thread-pool.h>
#include <mvv/queue-orders.h>
#include <mvv/resource.h>
#include <mvv/engine-mpr.h>
#include <mvv/layout.h>

using namespace mvv;

namespace
{
   void wait( double seconds )
   {
     clock_t endwait;
     endwait = clock () + seconds * CLOCKS_PER_SEC ;
     while (clock() < endwait) {}
   }
}

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
      orders.push_back( o );
   }

private:
   Orders orders;
};

class TestSubsystem
{
public:
   void testOrderMpr()
   {
      OrderProviderTest orderProvider;

      // load data
      const std::string path =  "D:/Devel/sandbox/nllTest/data/medical/pet-NAC.mf2";
      //const std::string path = "../../nllTest/data/medical/pet-NAC.mf2";
      MedicalVolume volume;
      bool loaded = nll::imaging::loadSimpleFlatFile( path, volume );
      ensure( loaded, "error" );

      // set up resources
      ResourceTransferFunctionWindowing windowing( 100, 500 );
      ResourceVolumes volumes;
      volumes.attachVolume( &volume, 1, &windowing );


      ResourceVector3d origin;
      origin.setValue( 0, 0 );
      origin.setValue( 1, 0 );
      origin.setValue( 2, 43 );

      ResourceVector3d v1;
      v1.setValue( 0, 1 );
      v1.setValue( 1, 0 );
      v1.setValue( 2, 0 );

      ResourceVector3d v2;
      v2.setValue( 0, 0 );
      v2.setValue( 1, 1 );
      v2.setValue( 2, 0 );

      ResourceVector2d zoom;
      zoom.setValue( 0, 1 );
      zoom.setValue( 1, 1 );

      // create a drawable object
      EngineMpr drawableMpr( orderProvider,
                             volumes,
                             origin,
                             v1,
                             v2,
                             zoom );

      // set up the layout
      PaneDrawable layout( drawableMpr,
                           nll::core::vector2ui( 0, 0 ),
                           nll::core::vector2ui( 512, 512 ) );

      // update the layout (says the dimension needed for the MPR)
      layout.updateLayout();

      // set up the queue
      QueueOrder queue( orderProvider, 2 );
      boost::thread dispatchThread( boost::ref( queue ) );

      // -- run --
      // simulate: check everything is up to date, as it is not it will create orders.
      drawableMpr.run();

      // notify the queue: an order is arrived. This should be automatically done by the
      // pushOrder of the order provdier
      queue.notify();


      wait( 3 );
      queue.kill();
      dispatchThread.interrupt();
      dispatchThread.join();
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSubsystem);
TESTER_TEST(testOrderMpr);
TESTER_TEST_SUITE_END();
#endif