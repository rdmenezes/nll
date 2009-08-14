#include "stdafx.h"
#include <mvv/thread-pool.h>
#include <boost/thread/thread.hpp>
#include <time.h>

using namespace mvv;


void wait( double seconds )
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait) {}
}

class TestThreading
{
public:
   /**
    QueueOrder: Check a function onOrderCreated is working
    */
   void test1()
   {
      struct OrderResultVal : public OrderResult
      {
         OrderResultVal( unsigned v ) : val( v )
         {}

         unsigned val;
      };

      class OrderWait : public Order
      {
      public:
         OrderWait() : Order( ORDER_NULL, false, Order::Predecessors() )
         {
         }

         virtual OrderResult* run()
         {
            std::cout << "[working]" << std::endl;
            wait( 0.5 );
            return new OrderResultVal( 1 );
         }
      };

      ThreadPool  pool( 2 );

      OrderWait* order1 = new OrderWait();
      pool.run( order1 );
      wait( 1 );
      ThreadPool::Orders  results = pool.getFinishedOrdersAndClearList();
      TESTER_ASSERT( results.begin() != results.end(), "empty" );
      const OrderResult* res = ( *results.begin() )->getResult();
      TESTER_ASSERT( dynamic_cast<const OrderResultVal*>( res )->val );
      pool.kill();
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestThreading);
TESTER_TEST(test1);

TESTER_TEST_SUITE_END();
#endif