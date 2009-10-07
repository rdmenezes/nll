#include "stdafx.h"
#include <mvv/context.h>

using namespace mvv;

struct ContextInstance1 : public ContextInstance
{
   int n;
};
struct ContextInstance2 : public ContextInstance
{
   std::string s;
};

class TestContext
{
public:
   void test1()
   {
      ContextInstance1* c1 = new ContextInstance1();
      ContextInstance2* c2 = new ContextInstance2();
      c2->s = "ok1";
      Context::instance().add( c1 );
      Context::instance().add( c2 );
      TESTER_ASSERT( Context::instance().get<ContextInstance1>() == c1 );
      TESTER_ASSERT( Context::instance().get<ContextInstance2>() == c2 );

      ContextInstance2* c2_1 = new ContextInstance2();
      c2_1->s = "ok2";
      Context::instance().add( c2_1 );
      TESTER_ASSERT( Context::instance().get<ContextInstance2>() == c2_1 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestContext);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif