#include <time.h>
#include <iostream>
#include <nll/nll.h>
#include <tester/register.h>

class TestOptimizerParameters
{
public:
   void testGeometric()
   {
      srand( 1 );
      nll::algorithm::ParameterOptimizerGaussianGeometric g( -10, 10, 1, 0.25, 1.1, 0.9 );

      double mean = 0;
      for ( unsigned n = 0; n < 100; ++n )
      {
         double val = g.generate();
         mean += val;
      }
      mean /= 100;
      TESTER_ASSERT( fabs( mean - 1 ) < 0.15 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestOptimizerParameters);
 TESTER_TEST(testGeometric);
TESTER_TEST_SUITE_END();
#endif
