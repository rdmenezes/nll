#include "stdafx.h"
#include <mvv/queue-orders.h>
#include <boost/thread/thread.hpp>

using namespace mvv;

class OrderAction
{
public:
   OrderAction ()
   {
      id = 0;
   }

   void orderAdded( OrderInterface* o )
   {
      id = o->getId();
   }

   ui32 id;
};

class OrderNewThread
{
public:
   OrderNewThread( OrderInterface* o )
   {
      launch = 0;
      order = o;
   }

   void doOnActionFinished( QueueOrdersInterface::OnOrderToBeProcessedSlot slot )
   {
      launch = new QueueOrdersInterface::OnOrderToBeProcessed();
      launch->connect( slot );
   }

   void operator()()
   {
      (*launch)( order );
   }

   QueueOrdersInterface::OnOrderToBeProcessed* launch;
   OrderInterface* order;
};

class TestThreading
{
public:
   /**
    QueueOrder: Check a function onOrderCreated is working
    */
   void test1()
   {
      OrderAction orderAction;
      QueueOrdersInterface* queue = new QueueOrders();
      queue->doOnActionAdded( boost::bind(&OrderAction::orderAdded, &orderAction, _1 ) );

      OrderInterface* o = new OrderInterface( NOOP );  
      queue->registerOrder( o );
      TESTER_ASSERT( orderAction.id == o->getId() );
   }

   /**
    QueueOrder: Check on an action is ended, the order manager is called
    */
   void test2()
   {
      QueueOrdersInterface* queue = new QueueOrders();

      OrderInterface order( NOOP );
      OrderNewThread pool( &order );
      pool.doOnActionFinished( boost::bind(&QueueOrdersInterface::onOrderFinished, queue, _1 ) );

      OrderAction orderAction;
      queue->doOnOrderFinished( boost::bind(&OrderAction::orderAdded, &orderAction, _1 ) );

      boost::thread thrd1( pool );

      thrd1.join();
      TESTER_ASSERT( orderAction.id == order.getId() );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestThreading);
TESTER_TEST(test1);
TESTER_TEST(test2);
TESTER_TEST_SUITE_END();
#endif