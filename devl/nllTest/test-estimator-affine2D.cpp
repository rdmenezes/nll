#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;



using namespace algorithm;

class TestSimilarityTransform
{
public:
   void testTransform()
   {
      std::cout << "Test similarity transform=";
      for ( ui32 n = 0; n < 1000; ++n )
      {
         if ( n % 100 == 0 )
            std::cout << "#";

         std::vector< core::vector2d > points1;
         std::vector< core::vector2d > points2;

         const double angle = core::generateUniformDistribution( 0, 2 * core::PI - 1e-30 );
         const double dx = core::generateUniformDistribution( -5, 5 );
         const double dy = core::generateUniformDistribution( -5, 5 );
         const double scale = core::generateUniformDistribution( 0.5, 5 );
         const core::Matrix<double> tfmRef = core::createTransformationAffine2D( angle, core::vector2d( scale, scale ), core::vector2d( dx, dy ) );

         for ( ui32 nn = 0; nn < 3; ++nn )
         {
            const double x = core::generateUniformDistribution( -50, 50 );
            const double y = core::generateUniformDistribution( -50, 50 );
            points1.push_back( core::vector2d( x, y ) );

            core::Buffer1D<double> p = core::make_buffer1D<double>( x, y, 1 );
            core::Matrix<double> m( p, 3, 1 );
            core::Matrix<double> ptfm = tfmRef * m;
            points2.push_back( core::vector2d( ptfm[ 0 ], ptfm[ 1 ] ) );
         }

         EstimatorTransformAffine2D estimator;
         core::Matrix<double> tfm = estimator.compute( points1, points2 );

         for ( ui32 n = 0; n < tfm.size(); ++n )
         {
            TESTER_ASSERT( fabs( tfm[ n ] - tfmRef[ n ] ) < 1e-6 );
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSimilarityTransform);
TESTER_TEST(testTransform);
TESTER_TEST_SUITE_END();
#endif
