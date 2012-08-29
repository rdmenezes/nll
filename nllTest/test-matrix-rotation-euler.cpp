#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

class TestMatrixRotationEuler
{
public:
   typedef core::Matrix<float>      Matrix;

   void testHasScaling()
   {
      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         const bool r = core::hasSclaling4x4( m );
         TESTER_ASSERT( !r );
      }

      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         m( 0, 0 ) = 2;
         const bool r = core::hasSclaling4x4( m );
         TESTER_ASSERT( r );
      }

      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         m( 1, 1 ) = 2;
         const bool r = core::hasSclaling4x4( m );
         TESTER_ASSERT( r );
      }

      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         m( 2, 2 ) = 2;
         const bool r = core::hasSclaling4x4( m );
         TESTER_ASSERT( r );
      }
   }

   void testHasShearing()
   {
      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         const bool r = core::hasShearing4x4( m );
         TESTER_ASSERT( !r );
      }

      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         m( 1, 0 ) = 0.1f;
         const bool r = core::hasShearing4x4( m );
         TESTER_ASSERT( r );
      }

      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         m( 0, 1 ) = 0.1f;
         const bool r = core::hasShearing4x4( m );
         TESTER_ASSERT( r );
      }

      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         m( 0, 2 ) = 0.1f;
         const bool r = core::hasShearing4x4( m );
         TESTER_ASSERT( r );
      }

      {
         Matrix m = core::identityMatrix<Matrix>( 4 );
         m( 0, 0 ) = 3;
         const bool r = core::hasShearing4x4( m );
         TESTER_ASSERT( !r );
      }

      {
         for ( size_t n = 0; n < 100; ++n )
         {
            const float angle = core::generateUniformDistributionf( -0.5f, 0.5f );
            const float spacing = core::generateUniformDistributionf( 0.5f, 2.5f );
            core::vector2f v1( spacing * std::cos( angle ), spacing * std::sin( angle ) );
            core::vector2f v2( -std::sin( angle ), std::cos( angle ) );
            const float dot = v1.dot( v2 );
            ensure( core::equal<float>( dot, 0, 1e-3f ), "PB!" );
         
            Matrix m = core::identityMatrix<Matrix>( 4 );
            m( 0, 0 ) = v1[ 0 ];
            m( 1, 0 ) = v1[ 1 ];
            m( 0, 1 ) = v2[ 0 ];
            m( 1, 1 ) = v2[ 1 ];
            const bool r = core::hasShearing4x4( m );
            TESTER_ASSERT( !r );
         }
      }
   }

   // create a random rotation matrix, get the euler angles, ensure that the matrix constructed from the euler angles is the same as the source
   // this is to avoid the different solutions for the euler angles...
   void testGetEulerAngle()
   {
      for ( size_t iter = 0; iter < 100000; ++iter )
      {
         srand( iter );
         const core::vector3f angles( core::generateUniformDistributionf( -core::PIf / 2, core::PIf / 2 ),
                                      core::generateUniformDistributionf( -core::PIf / 2, core::PIf / 2 ),
                                      core::generateUniformDistributionf( -core::PIf / 2, core::PIf / 2 ) );

         Matrix m;
         core::createRotationMatrix4x4FromEuler( angles, m );

         const core::vector3f anglesFound = core::getEulerAngleFromRotationMatrix( m );

         Matrix res;
         core::createRotationMatrix4x4FromEuler( anglesFound, res );

         
         Matrix diff = m - res;
         for ( size_t n = 0; n < diff.size(); ++n )
         {
            const bool isGood = fabs( diff[ n ] ) < 1e-2;
            TESTER_ASSERT( isGood );
         }
      }
   }

   // test against ground truth
   void testGetExpected()
   {
      Matrix m( 4, 4 );
      m( 0, 0 ) = 0.5f;
      m( 0, 1 ) = -0.1464f;
      m( 0, 2 ) = 0.8536f;

      m( 1, 0 ) = 0.5f;
      m( 1, 1 ) = 0.8536f;
      m( 1, 2 ) = -0.1464f;

      m( 2, 0 ) = -0.7071f;
      m( 2, 1 ) = 0.5f;
      m( 2, 2 ) = 0.5f;

      const core::vector3f anglesFound = core::getEulerAngleFromRotationMatrix( m );
      std::cout << anglesFound;
      const bool s1 = core::equal<float>( anglesFound[ 0 ], - 0.75f * core::PIf, 1e-3f ) &&
                      core::equal<float>( anglesFound[ 1 ], + 0.75f * core::PIf, 1e-3f ) &&
                      core::equal<float>( anglesFound[ 2 ], - 0.75f * core::PIf, 1e-3f );
      const bool s2 = core::equal<float>( anglesFound[ 0 ], + 0.25f * core::PIf, 1e-3f ) &&
                      core::equal<float>( anglesFound[ 1 ], + 0.25f * core::PIf, 1e-3f ) &&
                      core::equal<float>( anglesFound[ 2 ], + 0.25f * core::PIf, 1e-3f );
      TESTER_ASSERT( s1 || s2 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMatrixRotationEuler);
 TESTER_TEST(testHasScaling);
 TESTER_TEST(testHasShearing);
 TESTER_TEST(testGetEulerAngle);
 TESTER_TEST( testGetExpected );
TESTER_TEST_SUITE_END();
#endif