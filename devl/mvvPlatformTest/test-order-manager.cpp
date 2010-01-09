#include <tester/register.h>
#include <mvvPlatform/order-manager-thread-pool.h>
#include <mvvPlatform/order-consumer.h>

using namespace mvv::platform;

namespace
{
   void wait( float seconds )
   {
     clock_t endwait;
     endwait = (clock_t)(clock () + seconds * CLOCKS_PER_SEC);
     while (clock() < endwait) {}
   }

   struct DummyOrder4 : public Order
   {
      DummyOrder4( float time, const Order::Predecessors& pred ) : Order( Order::OrderClassId::create( "dummy2" ), pred ), timeToWait( time )
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

   class DummyOrderConsumer : public OrderConsumer
   {
   public:
      typedef std::vector< RefcountedTyped<Order> > Orders;
      DummyOrderConsumer()
      {
         interested.insert( Order::OrderClassId::create( "dummy2" ) );
      }

      virtual void consume( RefcountedTyped<Order> order )
      {
         orders.push_back( order );
      }

      virtual const std::set<OrderClassId>& interestedOrder() const
      {
         return interested;
      }

      std::set<OrderClassId> interested;
      Orders orders;
   };
}

struct TestOrderManager
{
   void test1()
   {
      OrderManagerThreadPool manager( 4 );
      DummyOrderConsumer orderConsumer;
      manager.connect( &orderConsumer );

      RefcountedTyped<Order> o1( new DummyOrder4( 0.5, Order::Predecessors() ) );
      manager.pushOrder( o1 );

      // first launch orders
      manager.run();

      // then dispatch orders
      wait( 1.0f );
      manager.run();

      TESTER_ASSERT( orderConsumer.orders.size() == 1 );
   }

   
};

TESTER_TEST_SUITE(TestOrderManager);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();