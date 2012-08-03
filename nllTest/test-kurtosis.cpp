#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

class TestKurtosis
{
public:
   void testGaussian()
   {
      srand( (unsigned)time( 0 ) );
      for ( size_t t = 0; t < 500; ++t )
      {
         const double mean = core::generateUniformDistribution( -100, 100 );
         const double var = core::generateUniformDistribution( 0.5, 20 );

         std::vector<double> vals;
         for ( size_t n = 0; n < 100000; ++n )
         {
            vals.push_back( core::generateGaussianDistribution( mean, var ) );
         }

         const double k = core::kurtosis( vals );  // the kurtosis of a gaussian distrib is 0
         TESTER_ASSERT( fabs( k ) < 1e-1 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKurtosis);
TESTER_TEST(testGaussian);
TESTER_TEST_SUITE_END();
#endif
