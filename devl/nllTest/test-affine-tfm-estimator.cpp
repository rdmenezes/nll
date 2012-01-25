#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
   

   /**
    @brief Find the exact solution for the similarity with non isotropic scaling mapping (i.e., this find the exact solution by using only the minimal number of points to find the transformation)
    
    Uses exactly 2 pairs of points A and B to find the transformation T such that B = T(A), with T = | [scalex scaley] * R(theta) 0 translation |
                                                                                                     | 0        1                               |
    @see http://personal.lut.fi/users/joni.kamarainen/downloads/publications/laitosrap111.pdf for implementation reference
         Experimental Study on Fast 2D Homography Estimation from a Few Point Correspondences
         Joni-Kristian Kamarainen
         Pekka Paalanen
    */
   /*
   class AffineSimilarityNonIsotropic2dExact
   {
   public:
      typedef core::Matrix<double>  Matrix;

   public:
      template <class Points1, class Points2>
      Matrix compute( const Points1& points1, const Points2& points2 )
      {
         ensure( points1.size() == 3, "we can only use 3 pairs of points" );
         ensure( points2.size() == 3, "we can only use 3 pairs of points" );

         // we take theta = 0 so that e1 = [ 1 0 ]' and e2 = [ 0 1 ]'
         const double avx1 = points1[ 1 ][ 0 ] - points1[ 0 ][ 0 ];
         const double avy1 = points1[ 1 ][ 1 ] - points1[ 0 ][ 1 ];
         const double avx2 = points1[ 2 ][ 0 ] - points1[ 0 ][ 0 ];
         const double avy2 = points1[ 2 ][ 1 ] - points1[ 0 ][ 1 ];
         const double u1 = avx1;    // e1.av1
         const double u2 = avx2;    // e1.av2
         const double v1 = avy1;    // e2.av2
         const double v2 = avy2;    // e2.av1

         const double bv2x = points2[ 2 ][ 0 ] - points2[ 0 ][ 0 ];
         const double bv2y = points2[ 2 ][ 1 ] - points2[ 0 ][ 1 ];
         const double bv1x = points2[ 1 ][ 0 ] - points2[ 0 ][ 0 ];
         const double bv1y = points2[ 1 ][ 1 ] - points2[ 0 ][ 1 ];
         const double bv2n = core::sqr( bv2x ) + core::sqr( bv2y );
         const double bv1n = core::sqr( bv1x ) + core::sqr( bv1y );

         Matrix atmp( 2, 2 );
         atmp( 0, 0 ) = u1 * u1;
         atmp( 0, 1 ) = v1 * v1;
         atmp( 1, 0 ) = u2 * u2;
         atmp( 1, 1 ) = v2 * v2;
         const bool inversed = core::inverse( atmp );
         if ( !inversed )
         {
            // degenerated configuration
            return core::identityMatrix<Matrix>( 3 );
         }

         Matrix anorm( 2, 1 );
         anorm[ 0 ] = bv1n;
         anorm[ 1 ] = bv2n;

         Matrix scaling = atmp * anorm;
         scaling[ 0 ] = sqrt( scaling[ 0 ] );
         scaling[ 1 ] = sqrt( scaling[ 1 ] );

         //
         // TODO: scaling is good, but not the rotation angle/translation... there is a better algorithm anyway <AffineSimilarityNonIsotropic2d>
         //

         std::cout << "scaling=" << scaling[ 0 ] << " " << scaling[ 1 ] << std::endl;

         const double dx = ( points1[ 0 ][ 0 ] - points1[ 1 ][ 0 ] );
         const double dy = ( points1[ 0 ][ 1 ] - points1[ 1 ][ 1 ] );

         const double dxp = ( points2[ 0 ][ 0 ] - points2[ 1 ][ 0 ] );
         const double dyp = ( points2[ 0 ][ 1 ] - points2[ 1 ][ 1 ] );

         const double r   = std::sqrt( core::sqr( dx )  + core::sqr( dy ) );
         const double rp  = std::sqrt( core::sqr( dxp ) + core::sqr( dyp ) );
         const double rrp = r * rp;

         if ( fabs( rrp ) <= 1e-7 )
         {
            return core::identityMatrix<Matrix>( 3 );
         }

         const double scale = rp / r;
         double r0 = (  dx * dxp + dy  * dyp ) / rrp;
         double r1 = ( -dx * dyp + dxp * dy )  / rrp;
         double r2 = -r1;
         double r3 =  r0;

         std::cout << "angle=" << acos( r0  ) << std::endl;

         r0 *= scaling[ 0 ];
         r2 *= scaling[ 0 ];

         r1 *= scaling[ 1 ];
         r3 *= scaling[ 1 ];

         Matrix tfm( 3, 3 );
         tfm( 2, 2 ) = 1;
         tfm( 0, 0 ) = r0;
         tfm( 0, 1 ) = r1;
         tfm( 0, 2 ) = - r0 * points1[ 0 ][ 0 ] + r2 * points1[ 0 ][ 1 ] + points2[ 0 ][ 0 ];

         tfm( 1, 0 ) = r2;
         tfm( 1, 1 ) = r3;
         tfm( 1, 2 ) = - r0 * points1[ 0 ][ 1 ] - r2 * points1[ 0 ][ 0 ] + points2[ 0 ][ 1 ];
         return tfm;
      }
   };*/

   
}
}

class TestAffineTransformationEstimator
{
   typedef std::vector<double> Point;
   typedef std::vector<Point> Points;

public:
   void testAffineIsotropic2D()
   {
      for ( ui32 n = 0; n < 1000; ++n )
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

         algorithm::EstimatorTransformSimilarityIsometric estimator( 0, 0.1, 10 );
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-1 );
         }
      }
   }

   void testAffineIsotropic3D()
   {
      for ( ui32 n = 0; n < 1000; ++n )
      {
         const double anglex = core::generateUniformDistribution( - core::PI, core::PI );
         const double angley = core::generateUniformDistribution( - core::PI, core::PI );
         const double anglez = core::generateUniformDistribution( - core::PI, core::PI );
         const double dx = core::generateUniformDistribution( -100, 100 );
         const double dy = core::generateUniformDistribution( -100, 100 );
         const double dz = core::generateUniformDistribution( -100, 100 );

         core::Matrix<double> rx( 4, 4 );
         core::matrix4x4RotationX( rx, (float)anglex );
         core::Matrix<double> ry( 4, 4 );
         core::matrix4x4RotationY( ry, (float)angley );
         core::Matrix<double> rz( 4, 4 );
         core::matrix4x4RotationZ( rz, (float)anglez );

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

         algorithm::EstimatorTransformSimilarityIsometric estimator;
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-1 );
         }
      }
   }

   void testAffineAnisotropic2D()
   {
      srand(12);
      for ( ui32 n = 0; n < 1000; ++n )
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

         algorithm::EstimatorTransformAffine2dDlt estimator( 0, 999, 999 );
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-3 );
         }
      }
   }


   void testDirectIsometry2D()
   {
      srand(12);
      for ( ui32 n = 0; n < 1000; ++n )
      {
         const double angle = core::generateUniformDistribution( - core::PI, core::PI );
         const double dx = core::generateUniformDistribution( -100, 100 );
         const double dy = core::generateUniformDistribution( -100, 100 );
         const double spx = 1;
         const double spy = 1;
         core::Matrix<double> tfm = core::createTransformationAffine2D( angle, core::vector2d( spx, spy ), core::vector2d( dx, dy ) );

         Points points1;
         Points points2;
         for ( ui32 p = 0; p < 2; ++p )
         {
            core::Matrix<double> point( 3, 1 );
            point[ 0 ] = core::generateUniformDistribution( -100, 100 );
            point[ 1 ] = core::generateUniformDistribution( -100, 100 );
            point[ 2 ] = 1;

            const core::Matrix<double> pointTfm = tfm * point;
            points1.push_back( core::make_vector<double>( point[ 0 ], point[ 1 ] ) );
            points2.push_back( core::make_vector<double>( pointTfm[ 0 ], pointTfm[ 1 ] ) );
         }

         algorithm::AffineIsometry2dExact estimator;
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-3 );
         }
      }
   }

   void testDirectSimilarity2D()
   {
      srand(12);
      for ( ui32 n = 0; n < 1000; ++n )
      {
         const double angle = core::generateUniformDistribution( - core::PI, core::PI );
         const double dx = core::generateUniformDistribution( -100, 100 );
         const double dy = core::generateUniformDistribution( -100, 100 );
         const double spx = core::generateUniformDistribution( 0.5, 2 );
         const double spy = spx;
         core::Matrix<double> tfm = core::createTransformationAffine2D( angle, core::vector2d( spx, spy ), core::vector2d( dx, dy ) );

         Points points1;
         Points points2;
         for ( ui32 p = 0; p < 2; ++p )
         {
            core::Matrix<double> point( 3, 1 );
            point[ 0 ] = core::generateUniformDistribution( -100, 100 );
            point[ 1 ] = core::generateUniformDistribution( -100, 100 );
            point[ 2 ] = 1;

            const core::Matrix<double> pointTfm = tfm * point;
            points1.push_back( core::make_vector<double>( point[ 0 ], point[ 1 ] ) );
            points2.push_back( core::make_vector<double>( pointTfm[ 0 ], pointTfm[ 1 ] ) );
         }

         algorithm::AffineSimilarity2dExact estimator;
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         //tfm.print(std::cout);
         //tfmEstimated.print(std::cout);
         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-3 );
         }
      }
   }

   void testSimilarityAnisotropic2D()
   {
      srand(12);
      for ( ui32 n = 0; n < 1000; ++n )
      {
         const double angle = core::generateUniformDistribution( - core::PI / 2, core::PI / 2 );
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

         algorithm::EstimatorAffineSimilarityNonIsotropic2d estimator( 0, 1000 );
         core::Matrix<double> tfmEstimated = estimator.compute( points1, points2 );

         for ( ui32 p = 0; p < tfm.size(); ++p )
         {
            TESTER_ASSERT( fabs( tfm[ p ] - tfmEstimated[ p ] ) < 1e-3 );
         }
      }
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestAffineTransformationEstimator);
TESTER_TEST(testSimilarityAnisotropic2D);
TESTER_TEST(testDirectSimilarity2D);
TESTER_TEST(testDirectIsometry2D);
TESTER_TEST(testAffineIsotropic2D);
TESTER_TEST(testAffineIsotropic3D);
TESTER_TEST(testAffineAnisotropic2D);
TESTER_TEST_SUITE_END();
#endif