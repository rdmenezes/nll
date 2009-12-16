#include <tester/register.h>

struct TestEngine
{
   void test1()
   {
   }
};

TESTER_TEST_SUITE(TestEngine);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();