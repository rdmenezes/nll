#include "stdafx.h"
#include <mvv/thread-pool.h>
#include <mvv/queue-orders.h>
#include <boost/thread/thread.hpp>
#include <time.h>

using namespace mvv;


void wait( double seconds )
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait) {}
}

struct OrderResultVal : public OrderResult
{
   OrderResultVal( unsigned v ) : val( v )
   {}

   unsigned val;
};

class OrderWait : public Order
{
public:
   OrderWait() : Order( ORDER_NULL, true, Order::Predecessors() )
   {
   }

   virtual OrderResult* run()
   {
      std::cout << "[working]" << std::endl;
      wait( 0.5 );
      return new OrderResultVal( 1 );
   }
};

class OrderWaitPredecessor : public Order
{
public:
   OrderWaitPredecessor( const Order::Predecessors& p ) : Order( ORDER_NULL, true, p )
   {
   }

   virtual OrderResult* run()
   {
      std::cout << "[working]" << std::endl;
      wait( 0.5 );
      return new OrderResultVal( 1 );
   }
};

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

   Orders orders;
};


class TestThreading
{
public:
   void test1()
   {
      
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

   void test2()
   {
      ThreadPool  pool( 1 );
      OrderWait* order1 = new OrderWait();
      OrderWait* order2 = new OrderWait();
      OrderWait* order3 = new OrderWait();
      pool.run( order1 );
      pool.run( order2 );
      pool.run( order3 );
      pool.kill();
   }

   /**
    @biref test basic orders
    */
   void testQueue()
   {
      OrderProviderTest orderProvider;
      QueueOrder        queue( orderProvider, 2 );

      boost::thread dispatchThread( boost::ref( queue ) );

      OrderWait* order1 = new OrderWait();
      OrderWait* order2 = new OrderWait();
      OrderWait* order3 = new OrderWait();
      OrderWait* order4 = new OrderWait();

      orderProvider.orders.push_back( order1 );
      orderProvider.orders.push_back( order2 );
      orderProvider.orders.push_back( order3 );

      //wait( 0.1 );
      //queue.notify();

      wait( 0.1 );
      orderProvider.orders.push_back( order4 );
      wait( 0.1 );
      queue.notify();

      wait( 3 );
      queue.kill();
      dispatchThread.interrupt();
      dispatchThread.join();

      TESTER_ASSERT( orderProvider.orders.size() == 0 );
      TESTER_ASSERT( order1->getResult() );
      TESTER_ASSERT( order2->getResult() );
      TESTER_ASSERT( order3->getResult() );
      TESTER_ASSERT( order4->getResult() );
   }

   /**
    @brief test predecessors
    */
   void testQueueOrders()
   {
      // create orders with dependencies
      Order::Predecessors pred1;
      OrderWait* order1 = new OrderWait();
      pred1.insert( order1->getId() );
      OrderWaitPredecessor* order2 = new OrderWaitPredecessor( pred1 );

      Order::Predecessors pred2;
      pred2.insert( order2->getId() + 2 ); // we know +2 will be the next 2 order
      OrderWaitPredecessor* order3 = new OrderWaitPredecessor( pred2 );
      OrderWait* order4 = new OrderWait();
      ensure( ( order4->getId() == order2->getId() + 2) , "something went wrong in the test" );

      Order::Predecessors pred3;
      pred3.insert( order2->getId() );
      pred3.insert( order3->getId() );
      OrderWaitPredecessor* order5 = new OrderWaitPredecessor( pred3 );

      // set up the orders
      OrderProviderTest orderProvider;
      QueueOrder        queue( orderProvider, 2 );
      boost::thread dispatchThread( boost::ref( queue ) );

      // run
      orderProvider.orders.push_back( order1 );
      orderProvider.orders.push_back( order2 );
      orderProvider.orders.push_back( order3 );
      orderProvider.orders.push_back( order4 );
      orderProvider.orders.push_back( order5 );
      queue.notify();

      wait( 3 );
      QueueOrder::OrderBuffer orders = queue.getFinishedOrdersAndClear();
      TESTER_ASSERT( orders.size() == 5 );

      queue.kill();
      dispatchThread.interrupt();
      dispatchThread.join();

      TESTER_ASSERT( orderProvider.orders.size() == 0 );
      TESTER_ASSERT( order1->getResult() );
      TESTER_ASSERT( order2->getResult() );
      TESTER_ASSERT( order3->getResult() );
      TESTER_ASSERT( order4->getResult() );
      TESTER_ASSERT( order5->getResult() );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestThreading);
//TESTER_TEST(test1);
//TESTER_TEST(test2);
//TESTER_TEST(testQueue);
//TESTER_TEST(testQueueOrders);
TESTER_TEST_SUITE_END();
#endif