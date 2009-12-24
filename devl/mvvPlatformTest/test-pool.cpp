#include <tester/register.h>
#include <mvvPlatform/thread-pool.h>
#include <boost/thread/thread.hpp>

using namespace mvv::platform;

namespace
{
   void wait( float seconds )
   {
     clock_t endwait;
     endwait = clock () + seconds * CLOCKS_PER_SEC ;
     while (clock() < endwait) {}
   }


   struct DummyOrder3 : public Order
   {
      DummyOrder3( float time, const Order::Predecessors& pred ) : Order( Order::OrderClassId::create( "dummy2" ), pred ), timeToWait( time )
      {
         computed = false;
      }

      virtual OrderResult* _compute()
      {
         wait( timeToWait );
         computed = true;
         return new OrderResult();
      }

      bool computed;
      float timeToWait;
   };


   struct DummyOrder2 : public Order
   {
      DummyOrder2( float time ) : Order( Order::OrderClassId::create( "dummy" ), Order::Predecessors() ), timeToWait( time )
      {
         computed = false;
      }

      virtual OrderResult* _compute()
      {
         wait( timeToWait );
         computed = true;
         return new OrderResult();
      }

      bool computed;
      float timeToWait;
   };
}

struct TestPool
{
   void testDummy()
   {
      RefcountedTyped<Order> o1( new DummyOrder2( 0.5f ) );
      ThreadPool pool( 2 );
      boost::thread dispatchThread( boost::ref( pool ) );

      pool.push( o1 );
      wait( 1 );

      ThreadPool::Orders finished = pool.getFinishedOrdersAndClearList();

      // check the order has been run
      // check all references are gone ( only o1 & finished )
      TESTER_ASSERT( finished.size() == 1 && finished[ 0 ].getNumberOfReference() == 2 );
      pool.kill();
   }

   void testMulti()
   {
      
      RefcountedTyped<Order> o3( new DummyOrder2( 0.2f ) );
      RefcountedTyped<Order> o2( new DummyOrder2( 0.05f ) );
      RefcountedTyped<Order> o1( new DummyOrder2( 0.05f ) );

      ThreadPool pool( 2 );
      boost::thread dispatchThread( boost::ref( pool ) );

      ThreadPool::Orders finished;
      for ( int n = 0; n < 10; ++n )
      {
         pool.push( o1 );
         pool.push( o2 );
         pool.push( o3 );

         wait( 0.3f );
         finished = pool.getFinishedOrdersAndClearList();
      }


      TESTER_ASSERT( pool.getNumberOfOrdersToRun() == 0 );

      // check the order has been run
      // check all references are gone ( only o1 & finished )
      TESTER_ASSERT( finished.size() == 3 && finished[ 0 ].getNumberOfReference() == 2 && (*finished[ 0 ]).getResult()
                                          && finished[ 1 ].getNumberOfReference() == 2 && (*finished[ 1 ]).getResult()
                                          && finished[ 2 ].getNumberOfReference() == 2 && (*finished[ 2 ]).getResult() );
      std::cout << "kill" << std::endl;
      pool.kill();
   }

   void testPredecessor()
   {
      for ( int n = 0; n < 39; ++n )
      {
         RefcountedTyped<Order> o0( new DummyOrder3( 0.25f, Order::Predecessors() ) );
         RefcountedTyped<Order> o1( new DummyOrder3( 0.4f, Order::Predecessors() ) );
         RefcountedTyped<Order> o2( new DummyOrder3( 0.2f, Order::Predecessors() ) );

         Order::Predecessors pred;
         pred.insert( o2 );
         pred.insert( o1 );
         pred.insert( o0 );
         RefcountedTyped<Order> o3( new DummyOrder3( 0.3f, pred ) );

         ThreadPool pool( 4 );
         boost::thread dispatchThread( boost::ref( pool ) );

         pool.push( o3 );
         pool.push( o0 );
         pool.push( o1 );
         pool.push( o2 );

         wait( 3 );
         ThreadPool::Orders finished = pool.getFinishedOrdersAndClearList();
         TESTER_ASSERT( finished.size() == 4 );
         TESTER_ASSERT( finished.size() == 4 && finished[ 0 ].getNumberOfReference() == 4 && (*finished[ 0 ]).getResult()
                                             && finished[ 1 ].getNumberOfReference() == 4 && (*finished[ 1 ]).getResult()
                                             && finished[ 2 ].getNumberOfReference() == 4 && (*finished[ 2 ]).getResult()
                                             && finished[ 3 ].getNumberOfReference() == 2 && (*finished[ 3 ]).getResult() );
      }
   }
   
};

TESTER_TEST_SUITE(TestPool);
TESTER_TEST(testPredecessor);
//TESTER_TEST(testDummy);
//TESTER_TEST(testMulti);
TESTER_TEST_SUITE_END();