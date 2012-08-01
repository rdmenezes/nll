#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
}
}

class TestFastVolumePyramid
{
public:
   void testPyramid()
   {
   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestFastVolumePyramid);
TESTER_TEST(testPyramid);
TESTER_TEST_SUITE_END();
#endif