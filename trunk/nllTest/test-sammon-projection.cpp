#include <nll/nll.h>
#include <iostream>
#include <tester/register.h>

struct TestSammonProjection
{
   void test()
   {
      srand( 0 );
      typedef std::vector<double>   Point;
      typedef std::vector<Point>    Points;

      Points points;
      for ( int n = 0; n < 4000; ++n )
         points.push_back( nll::core::make_vector<double>( nll::core::generateUniformDistribution( -50, 50 ),
                                                           nll::core::generateUniformDistribution( -50, 50 ),
                                                           nll::core::generateUniformDistribution( -50, 50 ) ) );

      nll::algorithm::SammonProjection projection;

      double stress = 10;
      std::vector<Point> p = projection.project( points, 0.8, 1000, 1e-5, 2, &stress );
      std::cout << "streass=" << stress << std::endl;
      TESTER_ASSERT( stress < 6.1e-2)

   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSammonProjection);
 TESTER_TEST(test);
TESTER_TEST_SUITE_END();
#endif
