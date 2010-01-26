#include "stdafx.h"
#include <nll/nll.h>

class TestTransformation
{
public:
   void testAffine()
   {
      nll::core::vector3f t( 10, -5, 1 );
      nll::imaging::Transformation::Matrix tfm = nll::core::identityMatrix<nll::imaging::Transformation::Matrix>( 4 );
      tfm( 0, 3 ) = t[ 0 ];
      tfm( 1, 3 ) = t[ 1 ];
      tfm( 2, 3 ) = t[ 2 ];

      nll::imaging::TransformationAffine affine1( tfm );
      for ( int n = 0; n < 100; ++n )
      {
         nll::core::vector3f p( (float)(rand()%100), (float)(rand()%100), (float)(rand()%100) );
         TESTER_ASSERT( affine1.transformDeformableOnly( p ) == p );
         TESTER_ASSERT( affine1.transform( p ) == ( p + t ) );

         nll::imaging::Transformation* c = affine1.clone();
         TESTER_ASSERT( c->transform( p ) == ( p + t ) );
      }
   }

   void testMprIdentity()
   {
      typedef nll::imaging::VolumeSpatial<float>      Volume;
      typedef nll::imaging::Slice<float>              Slice;
      typedef Volume::Matrix                          Matrix;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator > Mpr;

      // rotation of 90 degree on the left, and step on the left of -10
      Matrix pst;
      nll::core::matrix4x4RotationZ( pst, static_cast<float>( nll::core::PI / 2 ) );
      pst( 0, 3 ) = -10;
      Volume v( nll::core::vector3ui( 16, 16, 16 ),
                pst, 0 );

      // fill with specific voxels
      v( 0, 0, 0 ) = 100;
      v( 10, 0, 0 ) = 10;
      v( 0, 5, 0 ) = 5;
      v( 15, 15, 15 ) = 200;

      // check geometry
      nll::core::vector3f p1 = v.indexToPosition( nll::core::vector3f( 0, 0, 0 ) );
      nll::core::vector3f p2 = v.indexToPosition( nll::core::vector3f( 10, 0, 0 ) );
      nll::core::vector3f p3 = v.indexToPosition( nll::core::vector3f( 0, 5, 0 ) );
      TESTER_ASSERT( ( p1 - nll::core::vector3f( -10, 0, 0 ) ).norm2() < 1e-5  );
      TESTER_ASSERT( ( p2 - nll::core::vector3f( -10, -10, 0 ) ).norm2() < 1e-5  );
      TESTER_ASSERT( ( p3 - nll::core::vector3f( -10 + 5, 0, 0 ) ).norm2() < 1e-5  );

      // get a slice in this geometry
      Slice s( nll::core::vector3ui( 16, 16, 1 ),
               nll::core::vector3f( 0, -1, 0 ),
               nll::core::vector3f( 1, 0, 0 ),
               nll::core::vector3f( -10 + 16 / 2, -16 / 2 , 0 ),
               nll::core::vector2f( 1.0f, 1.0f ) );

      Mpr mpr( v );
      mpr.getSlice( s );
      std::cout << s(10, 0, 0 ) << std::endl;

      for ( int y = 0; y < 16; ++y )
      {
         for ( int x = 0; x < 16; ++x )
         {
            std::cout << s( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      TESTER_ASSERT( s( 10, 0, 0 ) == 10 );
   }
   
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformation);
TESTER_TEST(testAffine);
TESTER_TEST(testMprIdentity);
TESTER_TEST_SUITE_END();
#endif

