#include <tester/register.h>
#include <mvvPlatform/engine.h>
#include <mvvPlatform/engine-order.h>
#include <mvvPlatform/resource.h>

using namespace mvv::platform;

class DummyEngineHandler : public EngineHandler
{
   typedef std::set<Engine*> Storage;

public:
   virtual void connect( Engine& e )
   {
      engines.insert( &e );
   }

   virtual void disconnect( Engine& e )
   {
      engines.erase( &e );
   }

   virtual void run()
   {
      for ( Storage::iterator it = engines.begin(); it != engines.end(); ++it )
      {
         (*it)->run();
      }
   }

   Storage engines;
};

class DummyResource : public Resource<int>
{
public:
   typedef Resource<int>   Base;

   DummyResource( int* i, bool own ) : Resource<int>( i, own )
   {}

   int getValue() const
   {
      return Base::getValue();
   }

   void setValue( int v )
   {
      Base::getValue() = v;
      notify();
   }
};

class DummyEngine : public Engine
{
public:
   DummyEngine( EngineHandler& handler, DummyResource& r1, DummyResource& r2  ) : Engine( handler ), r1( r1 ), r2( r2 )
   {
      connect( r1 );
      connect( r2 );
   }

   virtual bool _run()
   {
      res = r1.getValue() + r2.getValue();
      return true;
   }

   int getResult() const
   {
      return res;
   }

   DummyResource& r1;
   DummyResource& r2;

private:
   int res;
};

class DummyOrder : public Order
{
public:
   DummyOrder() : Order( Order::OrderClassId::create( "dummy" ), Order::Predecessors() )
   {
   }


protected:
   virtual OrderResult* _compute()
   {
      return new OrderResult();
   }
};

class DummyEngineOrder : public EngineOrder
{
public:
   DummyEngineOrder( EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, DummyResource& r1, DummyResource& r2  ) : EngineOrder( handler, provider, dispatcher ), r1( r1 ), r2( r2 )
   {
      connect( r1 );
      connect( r2 );
      consumed = false;
   }

   virtual bool _run()
   {
      res = r1.getValue() + r2.getValue();
      currentOrder = RefcountedTyped<Order>( new DummyOrder() );

      _orderProvider.pushOrder( currentOrder );
      return true;
   }

   int getResult() const
   {
      return res;
   }

   void consume( RefcountedTyped<Order> o )
   {
      consumed = true;
   }

   virtual const std::set<OrderClassId>& interestedOrder() const
   {
      static std::set<OrderClassId> res;
      return res;
   }

   DummyResource& r1;
   DummyResource& r2;
   bool consumed;

private:
   int res;
   RefcountedTyped<Order> currentOrder;
};

class DummyOrderProvider : public OrderProvider
{
public:
   virtual Orders getOrdersAndClear()
   {
      Orders o = orders;
      orders.clear();
      return o;
   }

   virtual void pushOrder( RefcountedTyped<Order> order )
   {
      orders.push_back( order );
   }

   Orders   orders;
};

class DummyOrderDispatcher : public OrderDispatcher
{
public:
   typedef std::vector<OrderConsumer*>  Consumers;

   virtual void dispatch( RefcountedTyped<Order> order )
   {
      for ( Consumers::iterator it = consumers.begin(); it != consumers.end(); ++it )
      {
         (*it)->consume( order );
      }
   }

   virtual void connect( OrderConsumer& consumer )
   {
      consumers.push_back( &consumer );
   }

   virtual void disconnect( OrderConsumer& )
   {
   }

   Consumers consumers;
};

struct TestEngine
{
   void test1()
   {
      // basic computations
      int* i1 = new int;
      *i1 = 42;
      int i2 = 3;

      DummyResource resource1( i1, true );
      DummyResource resource2( &i2, false );
      DummyEngineHandler handler;

      DummyEngine engine1( handler, resource1, resource2 );

      handler.run();
      TESTER_ASSERT( engine1.getResult() == *i1 + i2 );
      TESTER_ASSERT( handler.engines.size() == 1 );

      // check what happen if we disconnect something
      resource2.disconnect( &engine1 );
   }

   void run1( DummyResource& r, DummyEngineHandler& h )
   {
      DummyEngine engine1( h, r, r );
      h.run();
      TESTER_ASSERT( engine1.getResult() == 2 * 42 );
      TESTER_ASSERT( h.engines.size() == 1 );
   }

   void testEngineDestroyedBeforeResource()
   {
      int i2 = 42;
      DummyResource resource2( &i2, false );
      DummyEngineHandler handler;
      run1( resource2, handler );

      TESTER_ASSERT( handler.engines.size() == 0 );
      resource2.notify();
   }

   void testInactive()
   {
      int* i1 = new int;
      *i1 = 42;
      int i2 = 3;

      DummyResource resource1( i1, true );
      resource1.setState( DummyResource::DISABLED );
      DummyResource resource2( &i2, false );
      DummyEngineHandler handler;

      DummyEngine engine1( handler, resource1, resource2 );
      TESTER_ASSERT( resource1.getNumberOfReference() == 2 );
      TESTER_ASSERT( resource2.getNumberOfReference() == 2 );

      handler.run();
      int val = engine1.getResult();
      TESTER_ASSERT( handler.engines.size() == 1 );

      resource1.setValue( -10 );
      handler.run();
      TESTER_ASSERT( engine1.getResult() == val );

      resource1.setState( DummyResource::ENABLED );
      handler.run();
      TESTER_ASSERT( engine1.getResult() != val );
   }

   
   void testOrderWorkflow()
   {
      DummyEngineHandler handler;
      DummyOrderProvider orderProvider;
      DummyOrderDispatcher dispatcher;

      int i1 = 42;
      int i2 = 3;

      DummyResource resource1( &i1, false );
      DummyResource resource2( &i2, false );
      DummyEngineOrder engine( handler, orderProvider, dispatcher, resource1, resource2 );

      TESTER_ASSERT( handler.engines.size() == 1 );
      TESTER_ASSERT( orderProvider.orders.size() == 0 );
      TESTER_ASSERT( dispatcher.consumers.size() == 1 );

      // run and create order
      handler.run();
      TESTER_ASSERT( orderProvider.orders.size() == 1 );

      // dispatch order, check it has been dispatched
      dispatcher.dispatch( orderProvider.orders[ 0 ] );
      TESTER_ASSERT( engine.consumed );
      TESTER_ASSERT( orderProvider.orders[ 0 ].getNumberOfReference() == 2 ); // in order & orderprovider
   }
};

TESTER_TEST_SUITE(TestEngine);
TESTER_TEST(test1);
TESTER_TEST(testEngineDestroyedBeforeResource);
TESTER_TEST(testInactive);
TESTER_TEST(testOrderWorkflow)
TESTER_TEST_SUITE_END();