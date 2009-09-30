#include "stdafx.h"
#include <mvv/symbol.h>
#include <mvv/queue-orders.h>

class TestSymbol
{
public:
   void test1()
   {
      mvv::Symbol s1 = mvv::Symbol::create("test1");
      mvv::Symbol s2 = mvv::Symbol::create("test2");
      mvv::Symbol s3 = mvv::Symbol::create("test1");
      mvv::Symbol s4 = s1;

      TESTER_ASSERT( s1 == s3 );
      TESTER_ASSERT( s1 == s4 );
      TESTER_ASSERT( s1 != s2 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSymbol);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif