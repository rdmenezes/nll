#include "stdafx.h"
#include <mvv/engine-mpr.h>

class TestEngineMpr
{
public:
   void test1()
   {
      //mvv::EngineMpr mpr;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestEngineMpr);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif