#include <tester/register.h>
#include <mvvPlatform/context.h>

using namespace mvv::platform;

struct ContextInstanceTest1 : public ContextInstance
{
   ContextInstanceTest1( int n ) : t( n )
   {}

   int t;
};

struct ContextInstanceTest2 : public ContextInstance
{
   ContextInstanceTest2( int n ) : t( n )
   {}

   int t;
};

struct TestContext
{
   void test1()
   {
      Context  context;
      context.add( new ContextInstanceTest1( 42 ) );
      context.add( new ContextInstanceTest2( 43 ) );

      ContextInstanceTest1* c1 = 0;
      context.get( c1 );
      TESTER_ASSERT( c1 && c1->t == 42 );

      ContextInstanceTest2* c2 = 0;
      context.get( c2 );
      TESTER_ASSERT( c2 && c2->t == 43 );

      context.add( new ContextInstanceTest1( 44 ) );
      context.get( c1 );
      TESTER_ASSERT( c1 && c1->t == 44 );
   }

   
};

TESTER_TEST_SUITE(TestContext);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();