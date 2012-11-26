#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   
}
}

class TestMatrixReflection
{
public:
   void testBasic()
   {
      for ( int n = 0; n < 2000000; ++n )
      {
         if ( n % 100000 == 0 )
            std::cout << "#";
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

         const core::Matrix<double> reflectionTfm = algorithm::comuteReflectionPlanar3dMatrix( plane );

         // reflexion of the reflexion is itself and reflexion of something not on the plane is not itself
         const core::vector3d point( core::generateUniformDistribution( -300, 300 ),
                                     core::generateUniformDistribution( -300, 300 ),
                                     core::generateUniformDistribution( -300, 300 ) );

         const core::vector3d r1 = transf4( reflectionTfm, point );
         const core::vector3d r2 = transf4( reflectionTfm, r1 );

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
TESTER_TEST_SUITE(TestMatrixReflection);
TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif