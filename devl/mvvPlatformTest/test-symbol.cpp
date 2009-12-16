#include <tester/register.h>
#include <mvvPlatform/symbol.h>
#include <mvvPlatform/symbol-typed.h>

using namespace mvv::platform;

struct TestSymbol
{
   void test1()
   {
      const Symbol& s1 = Symbol::create("test1");
      const Symbol& s2 = Symbol::create("test2");
      const Symbol& s3 = Symbol::create("test1");

      TESTER_ASSERT( s1 == s3 );
      TESTER_ASSERT( s1 != s2 );
      TESTER_ASSERT( s3 == s1 );
      TESTER_ASSERT( s2 != s3 );

      Symbol s4 = s1;
      TESTER_ASSERT( s2 != s4 );
      TESTER_ASSERT( s1 == s4 );
   }

   void test2()
   {
      typedef SymbolTyped<int>   IntSymbol;

      IntSymbol s1 = IntSymbol::create( "test1", 3 );
      IntSymbol s2 = IntSymbol::get( "test1" );
   }
};

TESTER_TEST_SUITE(TestSymbol);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();