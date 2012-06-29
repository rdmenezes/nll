#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>
#include "config.h"

//#define NLL_NOT_MULTITHREADED

using namespace nll;

namespace nll
{
namespace algorithm
{
   

   
}
}

class TestSurf3D
{
public:
   void testSurf3d()
   {
      const std::string volname = NLL_TEST_PATH "data/medical/ct.mf2";

      imaging::VolumeSpatial<float> volume;
      
      std::cout << "loadind..." << std::endl;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );

      std::cout << "runing surf..." << std::endl;
      algorithm::SpeededUpRobustFeatures3d surf( 5, 4, 2, 0.000005 );

      algorithm::SpeededUpRobustFeatures3d::Points points = surf.computesFeatures( volume );
      std::cout << "nbPoints=" << points.size() << std::endl;
      std::ofstream f( "c:/tmp/points.txt" );
      for ( size_t n = 0; n < points.size(); ++n )
      {
         const core::vector3f point = volume.indexToPosition( core::vector3f( points[ n ].position[ 0 ],
                                                                              points[ n ].position[ 1 ],
                                                                              points[ n ].position[ 2 ] ) );
         f << point[ 0 ] << " " << point[ 1 ] << " " << point[ 2 ] << " " << ( points[ n ].scale * volume.getSpacing()[ 0 ] ) << std::endl;
      }

      TESTER_ASSERT( points.size() > 1000 );
   }

   void testcartesianToSphericalCoordinate()
   {
      // checked against matlab cart2sph(x,y,z)
      {
         core::vector3d pos( 3, 0, 0 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 3, 1e-8 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-8 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-8 ) );
      }

      {
         core::vector3d pos( 1, 1, 0 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 1.4142, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0.7854, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0, 1e-4 ) );
      }

      {
         core::vector3d pos( 0, 0, 1 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 1, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 1.5708, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, 20, 15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, 20, 15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, -20, 15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, -20, 15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, 20, -15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, 20, -15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, -20, -15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, -20, -15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      // test identity
      for ( size_t n = 0; n < 100; ++n )
      {
         const core::vector3d p( core::generateUniformDistribution( -100, 100 ),
                                 core::generateUniformDistribution( -100, 100 ),
                                 core::generateUniformDistribution( -100, 100 ) );

         const core::vector3d spherical = core::cartesianToSphericalCoordinate( p );
         const core::vector3d back = core::sphericalToCartesianCoordinate( spherical );

         const double norm = ( back - p ).norm2();
         TESTER_ASSERT( norm < 1e-3 );
      }
   }

   void testRotation3d()
   {
      for ( size_t n = 0; n < 100; ++n )
      {
         const double a1 = core::generateUniformDistribution( -core::PI, core::PI );
         const double a2 = core::generateUniformDistribution( -core::PI, core::PI );

         const core::vector3d p( 1, 0, 0 );
         const core::vector3d expectedp = core::sphericalToCartesianCoordinate( core::vector3d( 1, a1, a2 ) );

         algorithm::SpeededUpRobustFeatures3d::RotationFromSpherical rot( a1, a2 );
         const core::vector3d r = rot.transform( p );
         TESTER_ASSERT( (expectedp - r).norm2() < 1e-3 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf3D);
TESTER_TEST(testSurf3d);
TESTER_TEST(testcartesianToSphericalCoordinate);
TESTER_TEST(testRotation3d);
TESTER_TEST_SUITE_END();
#endif