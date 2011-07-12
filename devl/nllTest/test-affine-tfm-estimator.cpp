#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;


class TestAffineTransformationEstimator
{
   typedef std::vector<double> Point;
   typedef std::vector<Point> Points;

public:
   void testAffineIsotropic2D()
   {
      for ( ui32 n = 0; n < 100; ++n )
      {
         const double angle = core::generateUniformDistribution( - core::PI, core::PI );
         const double dx = core::generateUniformDistribution( -100, 100 );
         const double dy = core::generateUniformDistribution( -100, 100 );
         const double sp = core::generateUniformDistribution( 0.5, 2 );
         core::Matrix<double> tfm = core::createTransformationAffine2D( angle, core::vector2d( sp, sp ), core::vector2d( dx, dy ) );

         Points points1;
         Points points2;
         for ( ui32 p = 0; p < 10; ++p )
         {
            core::Matrix<double> point( 3, 1 );
            point[ 0 ] = core::generateUniformDistribution( -100, 100 );
            point[ 1 ] = core::generateUniformDistribution( -100, 100 );
            point[ 2 ] = 1;

            const core::Matrix<double> pointTfm = tfm * point;
            points1.push_back( core::make_vector<double>( point[ 0 ], point[ 1 ] ) );
            points2.push_back( core::make_vector<double>( pointTfm[ 0 ], pointTfm[ 1 ] ) );
         }

         algorithm::EstimatorTransformAffineIsometric estimator( 0, 0.1, 10 );
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-1 );
         }
      }
   }

   void testAffineIsotropic3D()
   {
      for ( ui32 n = 0; n < 100; ++n )
      {
         const double anglex = core::generateUniformDistribution( - core::PI, core::PI );
         const double angley = core::generateUniformDistribution( - core::PI, core::PI );
         const double anglez = core::generateUniformDistribution( - core::PI, core::PI );
         const double dx = core::generateUniformDistribution( -100, 100 );
         const double dy = core::generateUniformDistribution( -100, 100 );
         const double dz = core::generateUniformDistribution( -100, 100 );

         core::Matrix<double> rx( 4, 4 );
         core::matrix4x4RotationX( rx, anglex );
         core::Matrix<double> ry( 4, 4 );
         core::matrix4x4RotationY( ry, angley );
         core::Matrix<double> rz( 4, 4 );
         core::matrix4x4RotationZ( rz, anglez );

         core::Matrix<double> tfm = core::createTranslation4x4( core::vector3d( dx, dy, dz ) ) *
                                    rz *
                                    ry *
                                    rz;

         Points points1;
         Points points2;
         for ( ui32 p = 0; p < 10; ++p )
         {
            core::Matrix<double> point( 4, 1 );
            point[ 0 ] = core::generateUniformDistribution( -100, 100 );
            point[ 1 ] = core::generateUniformDistribution( -100, 100 );
            point[ 2 ] = core::generateUniformDistribution( -100, 100 );
            point[ 3 ] = 1;

            const core::Matrix<double> pointTfm = tfm * point;
            points1.push_back( core::make_vector<double>( point[ 0 ], point[ 1 ], point[ 2 ] ) );
            points2.push_back( core::make_vector<double>( pointTfm[ 0 ], pointTfm[ 1 ], pointTfm[ 2 ] ) );
         }

         algorithm::EstimatorTransformAffineIsometric estimator;
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-1 );
         }
      }
   }

   void testAffineAnisotropic2D()
   {
      srand(11);
      for ( ui32 n = 0; n < 100; ++n )
      {
         const double angle = core::generateUniformDistribution( - core::PI, core::PI );
         const double dx = core::generateUniformDistribution( -100, 100 );
         const double dy = core::generateUniformDistribution( -100, 100 );
         const double spx = core::generateUniformDistribution( 0.5, 2 );
         const double spy = core::generateUniformDistribution( 0.5, 2 );
         core::Matrix<double> tfm = core::createTransformationAffine2D( angle, core::vector2d( spx, spy ), core::vector2d( dx, dy ) );

         Points points1;
         Points points2;
         for ( ui32 p = 0; p < 3; ++p )
         {
            core::Matrix<double> point( 3, 1 );
            point[ 0 ] = core::generateUniformDistribution( -100, 100 );
            point[ 1 ] = core::generateUniformDistribution( -100, 100 );
            point[ 2 ] = 1;

            const core::Matrix<double> pointTfm = tfm * point;
            points1.push_back( core::make_vector<double>( point[ 0 ], point[ 1 ] ) );
            points2.push_back( core::make_vector<double>( pointTfm[ 0 ], pointTfm[ 1 ] ) );
         }

         algorithm::EstimatorTransformAffine2dDlt estimator;
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         std::cout << "real spacing=" << spx << " " << spy << std::endl;
         tfm.print( std::cout );
         tfmEstimated.print( std::cout );
         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-3 );
         }
      }
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestAffineTransformationEstimator);
TESTER_TEST(testAffineIsotropic2D);
TESTER_TEST(testAffineIsotropic3D);
TESTER_TEST(testAffineAnisotropic2D);
TESTER_TEST_SUITE_END();
#endif