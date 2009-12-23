#include <tester/register.h>
#include <mvvPlatform/types.h>
#include <mvvPlatform/resource.h>
#include <mvvPlatform/resource-volumes.h>
#include <mvvPlatform/resource-storage-volumes.h>
#include <mvvPlatform/resource-map.h>


using namespace mvv;
using namespace mvv::platform;

struct Test1
{
   void test1()
   {
      
   }   
};

TESTER_TEST_SUITE(Test1);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();