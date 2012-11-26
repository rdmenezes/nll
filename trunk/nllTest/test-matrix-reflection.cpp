#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   /**
    @brief Renomalize the homogeneous matrix so that the element tfm( 3, 3 ) = 1
    @note most algorithms assume tfm( 3, 3 ) = 1
    */
   template <class T, class Mapper, class Alloc>
   core::Matrix<T, Mapper, Alloc> resetHomogeneous4x4( const core::Matrix<T, Mapper, Alloc>& tfm )
   {
      ensure( tfm.sizex() == 4 && tfm.sizey() == 4, "must be a 4x4 matrix" );

      core::Matrix<T, Mapper, Alloc> m = tfm.clone();
      const T coef = tfm( 3, 3 );
      for ( size_t y = 0; y < 3; ++y )
      {
         for ( size_t x = 0; x < 3; ++x )
         {
            m( y, x ) /= coef;
         }
      }

      m( 0, 3 ) =  tfm( 0, 3 );
      m( 1, 3 ) =  tfm( 1, 3 );
      m( 2, 3 ) =  tfm( 2, 3 );
      m( 3, 3 ) = 1;

      return m;
   }

   /**
    @brief Compute the matrix representing a 3D planar reflexion

    @note we can determine algebraically the transformation matrix, however it involves a lot of system solving.
          Instead we are going to use a transformation estimator that maps point of the plane on the plane passing by
          the origin belonging the plane XY. Solutions should be identical.
    */
   core::Matrix<double> comuteReflexionPlanar3dMatrix( const core::GeometryPlane& plane )
   {
      typedef EstimatorTransformSimilarityIsometric TransformationEstimator;
      typedef core::Matrix<double>                  Matrix;

      // first compute a transformation that maps the plane origin to the coordinate system's origin
      // additionally the transformation will map the input plane onto the plane XY
      // these conditions are enough to define a unique transformation
      std::vector<core::vector3f> pointsInOriginalPlane;
      pointsInOriginalPlane.reserve( 3 );
      pointsInOriginalPlane.push_back( plane.getOrigin() );
      pointsInOriginalPlane.push_back( plane.getOrigin() + plane.getAxisX() );
      pointsInOriginalPlane.push_back( plane.getOrigin() + plane.getAxisY() );

      std::vector<core::vector3f> pointsInDestinationPlane;
      pointsInDestinationPlane.reserve( 3 );
      pointsInDestinationPlane.push_back( core::vector3f( 0, 0, 0 ) );
      pointsInDestinationPlane.push_back( core::vector3f( 1, 0, 0 ) );
      pointsInDestinationPlane.push_back( core::vector3f( 0, 1, 0 ) );

      TransformationEstimator estimator( 0, 0, std::numeric_limits<double>::max() );   // we don't want any constraints on the transformation
      const Matrix tfmPlaneToDefault = estimator.compute( pointsInOriginalPlane, pointsInDestinationPlane );
      ensure( estimator.getLastResult() == TransformationEstimator::OK, "should be impossible, there must always be a unique solution if the plane is well defined" );

      // we know exactly the reflexion matrix for the plane XY
      Matrix zDefaultReflexion = core::identityMatrix<Matrix>( 4 );
      zDefaultReflexion( 3, 3 ) = -1;

      // finally, compute the transformation default plane back to the original plane
      Matrix tfmDefaultToPlane = tfmPlaneToDefault.clone();
      const bool isOk = core::inverse( tfmDefaultToPlane );
      ensure( isOk, "should not happen: the matrix should be well defined" );

      // finally return the composition of these transformations
      // note: the homogeneous coefficien may not be equal to 1 anymore,
      // so normalize it so that tfm( 3, 3 ) = 1
      return resetHomogeneous4x4( tfmDefaultToPlane * zDefaultReflexion * tfmPlaneToDefault );
   }
}
}

class TestMatrixReflexion
{
public:
   template <class T, class Mapper, class Allocator, class Vector>
   static Vector transf4Full( const core::Matrix<T, Mapper, Allocator>& m, const Vector& v )
   {
      assert( m.sizex() == 4 && m.sizey() == 4 );
      return Vector( - v[ 0 ] * m( 0, 0 ) + - v[ 1 ] * m( 0, 1 ) + - v[ 2 ] * m( 0, 2 ) + m( 0, 3 ),
                     - v[ 0 ] * m( 1, 0 ) + - v[ 1 ] * m( 1, 1 ) + - v[ 2 ] * m( 1, 2 ) + m( 1, 3 ),
                     - v[ 0 ] * m( 2, 0 ) + - v[ 1 ] * m( 2, 1 ) + - v[ 2 ] * m( 2, 2 ) + m( 2, 3 ) );
   }

   void testBasic()
   {
      for ( int n = 0; n < 10000; ++n )
      {
         const double tol = 1e-3;

         // generate the reflexion plane         
         core::vector3f axisx( core::generateUniformDistributionf( -3, 3 ),
                               core::generateUniformDistributionf( -3, 3 ),
                               core::generateUniformDistributionf( -3, 3 ) );
         core::vector3f axisy( core::generateUniformDistributionf( -3, 3 ),
                               core::generateUniformDistributionf( -3, 3 ),
                               core::generateUniformDistributionf( -3, 3 ) );
         core::vector3f origin( core::generateUniformDistributionf( -300, 300 ),
                                core::generateUniformDistributionf( -300, 300 ),
                                core::generateUniformDistributionf( -300, 300 ) );
                                
         const double outerNorm = core::cross( axisx, axisy ).norm2();
         if ( core::equal( outerNorm, 0.0, tol ) )
         {
            // the plane is ill defined so restart
            continue;
         }

         core::GeometryPlane plane( origin, axisx, axisy );

         const core::Matrix<double> reflexionTfm = algorithm::comuteReflexionPlanar3dMatrix( plane );

         // reflexion of the reflexion is itself and reflexion of something not on the plane is not itself
         const core::vector3d point( core::generateUniformDistribution( -300, 300 ),
                                     core::generateUniformDistribution( -300, 300 ),
                                     core::generateUniformDistribution( -300, 300 ) );

         const core::vector3d r1 = transf4( reflexionTfm, point );
         const core::vector3d r2 = transf4( reflexionTfm, r1 );

         // check
         const double distR2P = (r2 - point).norm2();
         const double distR1P = (r1 - point).norm2();
         const bool isPonPlane = plane.contains( core::vector3f( (float)point[ 0 ],
                                                                 (float)point[ 1 ],
                                                                 (float)point[ 2 ] ) );
         TESTER_ASSERT( core::equal( distR2P, 0.0, tol ) );
         if ( !isPonPlane )
         {
            TESTER_ASSERT( !core::equal( distR1P, 0.0, tol ) );
         }

      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMatrixReflexion);
TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif