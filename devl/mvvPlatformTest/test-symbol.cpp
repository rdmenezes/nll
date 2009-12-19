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

      const IntSymbol& s1 = IntSymbol::create("test1");
      const IntSymbol& s2 = IntSymbol::create("test2");
      const IntSymbol& s3 = IntSymbol::create("test1");

      TESTER_ASSERT( s1 == s3 );
      TESTER_ASSERT( s1 != s2 );
      TESTER_ASSERT( s3 == s1 );
      TESTER_ASSERT( s2 != s3 );

      IntSymbol s4 = s1;
      TESTER_ASSERT( s2 != s4 );
      TESTER_ASSERT( s1 == s4 );

      typedef SymbolTyped<void>   VoidSymbol;

      //VoidSymbol v1 = VoidSymbol::create("void1");
      //v1 == s1;
   }
};

TESTER_TEST_SUITE(TestSymbol);
TESTER_TEST(test1);
TESTER_TEST(test2);
TESTER_TEST_SUITE_END();