#include <tester/register.h>
#include <mvvPlatform/engine.h>
#include <mvvPlatform/resource.h>

using namespace mvv::platform;

class DummyEngineHandler : public EngineHandler
{
   typedef std::set<Engine*> Storage;

public:
   virtual void connect( Engine* e )
   {
      engines.insert( e );
   }

   virtual void disconnect( Engine* e )
   {
      engines.erase( e );
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
};

class DummyEngine : public Engine
{
public:
   DummyEngine( EngineHandler& handler, DummyResource& r1, DummyResource& r2  ) : Engine( handler ), r1( r1 ), r2( r2 )
   {
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
      engine1.disconnect( &resource1 );
   }
};

TESTER_TEST_SUITE(TestEngine);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();