#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

class TestVolumeInterpolator3d
{
public:
   void testNearestNeighbourInterpolator3d_dummy()
   {
      // we have the nearest neighbour interpolator for free...
      typedef imaging::Volume<core::vector3f> Volume;
      typedef imaging::InterpolatorNearestNeighbour<Volume> Interpolator;


      Volume v( 5, 6, 7, core::vector3f( 0, 0, 0 ) );
      Interpolator iv( v );

      v( 1, 2, 3 ) = core::vector3f( 1, 2, 3 );
      v( 0, 2, 3 ) = core::vector3f( 0, 2, 3 );
      v( 3, 4, 5 ) = core::vector3f( 3, 4, 5 );

      {
         core::vector3f v1 = iv( &core::vector3f( 0, 0, 0 )[ 0 ] );
         TESTER_ASSERT( v1[ 0 ] == 0 );
         TESTER_ASSERT( v1[ 1 ] == 0 );
         TESTER_ASSERT( v1[ 2 ] == 0 );
      }

      {
         core::vector3f v1 = iv( &core::vector3f( 0.49999f, 2, 3 )[ 0 ] );
         TESTER_ASSERT( core::equal<float>( v1[ 0 ], 0 ) );
         TESTER_ASSERT( core::equal<float>( v1[ 1 ], 2 ) );
         TESTER_ASSERT( core::equal<float>( v1[ 2 ], 3 ) );
      }

      {
         core::vector3f v1 = iv( &core::vector3f( 0.500001f, 2, 3 )[ 0 ] );
         TESTER_ASSERT( core::equal<float>( v1[ 0 ], 1 ) );
         TESTER_ASSERT( core::equal<float>( v1[ 1 ], 2 ) );
         TESTER_ASSERT( core::equal<float>( v1[ 2 ], 3 ) );
      }

      {
         core::vector3f v1 = iv( &core::vector3f( 3.3f, 4.3f, 4.7f )[ 0 ] );
         TESTER_ASSERT( core::equal<float>( v1[ 0 ], 3 ) );
         TESTER_ASSERT( core::equal<float>( v1[ 1 ], 4 ) );
         TESTER_ASSERT( core::equal<float>( v1[ 2 ], 5 ) );
      }
   }

   void testTrilinearInterpolator3d_dummy()
   {
      // we have the nearest neighbour interpolator for free...
      typedef imaging::Volume<core::StaticVector<float, 3>> VolumeT;
      typedef imaging::InterpolatorTriLinearDummy<VolumeT> Interpolator;

      VolumeT v( 5, 6, 7, core::vector3f( 0, 0, 0 ) );
      v( 1, 2, 3 ) = core::vector3f( 1, 2, 3 );
      v( 0, 2, 3 ) = core::vector3f( 0, 2, 3 );
      v( 3, 4, 5 ) = core::vector3f( 3, 4, 5 );

      Interpolator iv( v );

      {
         core::vector3f v1 = iv( &core::vector3f( 0.5f, 2.0f, 3.0f )[ 0 ] );
         TESTER_ASSERT( core::equal<float>( v1[ 0 ], 0.5f, 1e-3f ) );
         TESTER_ASSERT( core::equal<float>( v1[ 1 ], 2.0f, 1e-3f ) );
         TESTER_ASSERT( core::equal<float>( v1[ 2 ], 3.0f, 1e-3f ) );
      }

      {
         core::vector3f v1 = iv( &core::vector3f( 0.0f, 2.0f, 3.0f )[ 0 ] );
         TESTER_ASSERT( core::equal<float>( v1[ 0 ], 0.0f, 1e-3f ) );
         TESTER_ASSERT( core::equal<float>( v1[ 1 ], 2.0f, 1e-3f ) );
         TESTER_ASSERT( core::equal<float>( v1[ 2 ], 3.0f, 1e-3f ) );
      }

      {
         core::vector3f v1 = iv( &core::vector3f( 1.0f, 2.0f, 3.0f )[ 0 ] );
         TESTER_ASSERT( core::equal<float>( v1[ 0 ], 1.0f, 1e-3f ) );
         TESTER_ASSERT( core::equal<float>( v1[ 1 ], 2.0f, 1e-3f ) );
         TESTER_ASSERT( core::equal<float>( v1[ 2 ], 3.0f, 1e-3f ) );
      }

      {
         core::vector3f v1 = iv( &core::vector3f( 3.0f, 4.0f, 5.0f )[ 0 ] );
         TESTER_ASSERT( core::equal<float>( v1[ 0 ], 3.0f, 1e-3f ) );
         TESTER_ASSERT( core::equal<float>( v1[ 1 ], 4.0f, 1e-3f ) );
         TESTER_ASSERT( core::equal<float>( v1[ 2 ], 5.0f, 1e-3f ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeInterpolator3d);
TESTER_TEST(testNearestNeighbourInterpolator3d_dummy);
TESTER_TEST(testTrilinearInterpolator3d_dummy);
TESTER_TEST_SUITE_END();
#endif