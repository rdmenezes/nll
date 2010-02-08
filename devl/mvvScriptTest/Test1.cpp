#include <tester/register.h>
#include <mvvPlatform/types.h>


using namespace mvv;
using namespace mvv::platform;


struct TestBasic
{
   void test1()
   {

   }
};

TESTER_TEST_SUITE(TestBasic);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();