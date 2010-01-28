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

      // rotation of 90 degree on the right, and step on the left of -10
      Matrix pst;
      nll::core::matrix4x4RotationZ( pst, static_cast<float>( nll::core::PI / 2 ) );
      pst( 0, 3 ) = -10;
      Volume v( nll::core::vector3ui( 16, 16, 16 ),
                pst, 0 );

      // fill with specific voxels
      v( 0, 0, 0 ) = 100;
      v( 10, 0, 0 ) = 10;
      v( 0, 5, 0 ) = 5;
      v( 10, 10, 10 ) = 200;

      // check geometry
      nll::core::vector3f p1 = v.indexToPosition( nll::core::vector3f( 0, 0, 0 ) );
      nll::core::vector3f p2 = v.indexToPosition( nll::core::vector3f( 10, 0, 0 ) );
      nll::core::vector3f p3 = v.indexToPosition( nll::core::vector3f( 0, 5, 0 ) );
      TESTER_ASSERT( ( p1 - nll::core::vector3f( -10, 0, 0 ) ).norm2() < 1e-5  );
      TESTER_ASSERT( ( p2 - nll::core::vector3f( -10, -10, 0 ) ).norm2() < 1e-5  );
      TESTER_ASSERT( ( p3 - nll::core::vector3f( -10 + 5, 0, 0 ) ).norm2() < 1e-5  );

      // get a slice in this geometry
      Slice s( nll::core::vector3ui( 16, 16, 1 ),
               nll::core::vector3f( 1, 0, 0 ),
               nll::core::vector3f( 0, 1, 0 ),
               nll::core::vector3f( -10+ 8, -7, 0 ),
               //nll::core::vector3f( -10+1, 0, 0 ),
               nll::core::vector2f( 1.0f, 1.0f ) );

      Mpr mpr( v );
      mpr.getSlice( s );

      
      for ( int y = s.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < s.size()[ 0 ]; ++x )
         {
            std::cout << s( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      

      // the volume is turned 90deg clockwise, moved by (-10, 0)
      // we use a regular base (1, 0, 0) and (0, 1, 0)
      // then at (-10, 0, 0) is the voxel(0, 0, 0) in the volume. We need to move the volume origin at top-left corner of the slice
      // so we need to add (sx/2,-sy/2)
      TESTER_ASSERT( s( 0, 0, 0 ) == 0 );
      TESTER_ASSERT( s( 0, 15, 0 ) == 100 );
      TESTER_ASSERT( s( 0, 15 - 10, 0 ) == 10 );
      TESTER_ASSERT( s( 5, 15, 0 ) == 5 );
   }

   void testMprTranslation()
   {
      typedef nll::imaging::VolumeSpatial<float>      Volume;
      typedef nll::imaging::Slice<float>              Slice;
      typedef Volume::Matrix                          Matrix;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator > Mpr;

      // rotation of 90 degree on the right, and step on the left of -10
      Matrix pst;
      nll::core::matrix4x4RotationZ( pst, static_cast<float>( nll::core::PI / 2 ) );
      pst( 0, 3 ) = -10;
      Volume v( nll::core::vector3ui( 16, 16, 16 ),
                pst, 0 );
      Matrix translation = nll::core::identityMatrix<Matrix>( 4 );
      translation( 0, 3 ) = 1;
      translation( 1, 3 ) = -2;

      // we have a target volume, we want to add (1, -2) from target->source, but the slice is defined in source so we need to invert the matrix
      nll::core::inverse( translation );

      // fill with specific voxels
      v( 0, 0, 0 ) = 100;
      v( 10, 0, 0 ) = 10;
      v( 0, 5, 0 ) = 5;
      v( 10, 10, 10 ) = 200;

      // check geometry
      nll::core::vector3f p1 = v.indexToPosition( nll::core::vector3f( 0, 0, 0 ) );
      nll::core::vector3f p2 = v.indexToPosition( nll::core::vector3f( 10, 0, 0 ) );
      nll::core::vector3f p3 = v.indexToPosition( nll::core::vector3f( 0, 5, 0 ) );
      TESTER_ASSERT( ( p1 - nll::core::vector3f( -10, 0, 0 ) ).norm2() < 1e-5  );
      TESTER_ASSERT( ( p2 - nll::core::vector3f( -10, -10, 0 ) ).norm2() < 1e-5  );
      TESTER_ASSERT( ( p3 - nll::core::vector3f( -10 + 5, 0, 0 ) ).norm2() < 1e-5  );

      // get a slice in this geometry
      Slice s( nll::core::vector3ui( 16, 16, 1 ),
               nll::core::vector3f( 1, 0, 0 ),
               nll::core::vector3f( 0, 1, 0 ),
               nll::core::vector3f( -10+ 8, -7, 0 ),
               nll::core::vector2f( 1.0f, 1.0f ) );

      Mpr mpr( v );
      mpr.getSlice( s, translation );

      /*
      std::cout << "print=" << std::endl;
      for ( int y = s.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < s.size()[ 0 ]; ++x )
         {
            std::cout << s( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      */

      // the volume is turned 90deg clockwise, moved by (-10, 0)
      // then a translation (1, -2, 0) is added
      // we use a regular base (1, 0, 0) and (0, 1, 0)
      // then at (-10, 0, 0) is the voxel(0, 0, 0) in the volume. We need to move the volume origin at top-left corner of the slice
      // so we need to add (sx/2,-sy/2)
      TESTER_ASSERT( s( 0, 0, 0 ) == 0 );
      TESTER_ASSERT( s( 0 + 1, 15 - 2, 0 ) == 100 );
      TESTER_ASSERT( s( 0 + 1, 15 - 10 - 2, 0 ) == 10 );
      TESTER_ASSERT( s( 5 + 1, 15 - 2, 0 ) == 5 );
   }

   void testMprTranslationInverse()
   {
      typedef nll::imaging::VolumeSpatial<float>      Volume;
      typedef nll::imaging::Slice<float>              Slice;
      typedef Volume::Matrix                          Matrix;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator > Mpr;

      // rotation of 90 degree on the right, and step on the left of -10
      Matrix pst;
      nll::core::matrix4x4RotationZ( pst, static_cast<float>( nll::core::PI / 2 ) );
      pst( 0, 3 ) = -10;
      Volume v( nll::core::vector3ui( 16, 16, 16 ),
                pst, 0 );

      // we want to rotate the volume by -90deg and (10, 0, 0). As the transformation is defined from
      // source to target, and the slice is in source but the volume is in target, we actualy need to set
      // tfm^-1 to transform how we want to!
      Matrix translation;
      nll::core::matrix4x4RotationZ( translation, static_cast<float>( nll::core::PI / 2 ) );
      translation( 0, 3 ) = 1;
      translation( 1, 3 ) = 2;

      // fill with specific voxels
      v( 0, 0, 0 ) = 100;
      v( 10, 0, 0 ) = 10;
      v( 0, 5, 0 ) = 5;
      v( 10, 10, 10 ) = 200;

      // get a slice in this geometry
      Slice s( nll::core::vector3ui( 22, 22, 1 ),
               nll::core::vector3f( 1, 0, 0 ),
               nll::core::vector3f( 0, 1, 0 ),
               nll::core::vector3f( -10, 0, 0 ),
               nll::core::vector2f( 1.0f, 1.0f ) );

      Mpr mpr( v );
      nll::imaging::TransformationAffine tfm( translation );
      mpr.getSlice( s, tfm );

      
      std::cout << "print=" << std::endl;
      for ( int y = s.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < s.size()[ 0 ]; ++x )
         {
            std::cout << s( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      

      // the volume is turned 90deg clockwise, moved by (-10, 0, 0)
      // then a rotation -90deg moved by (10, 0, 0)
      // we use a regular base (1, 0, 0) and (0, 1, 0)
      // we should be back from the starting point
      TESTER_ASSERT( s( 0, 0, 0 ) == 0 );
      TESTER_ASSERT( s( 11 -1, 11 - 2, 0 ) == 100 );
      TESTER_ASSERT( s( 11 -1, 11 + 5 - 2, 0 ) == 5 );
      TESTER_ASSERT( s( 10 + 11 - 1, 11 - 2, 0 ) == 10 );
   }


   void testMprTranslationInverse2()
   {
      typedef nll::imaging::VolumeSpatial<float>      Volume;
      typedef nll::imaging::Slice<float>              Slice;
      typedef Volume::Matrix                          Matrix;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator > Mpr;

      // rotation of 90 degree on the right, and step on the left of -10
      Matrix pst;
      nll::core::matrix4x4RotationZ( pst, static_cast<float>( nll::core::PI / 2 ) );
      pst( 0, 3 ) = -10;
      Volume v( nll::core::vector3ui( 16, 16, 16 ),
                pst, 0 );

      // we want to rotate the volume by -90deg and (10, 0, 0). As the transformation is defined from
      // source to target, and the slice is in source but the volume is in target, we actualy need to set
      // tfm^-1 to transform how we want to!
      Matrix translation;
      nll::core::matrix4x4RotationZ( translation, static_cast<float>( nll::core::PI / 2 ) );
      translation( 0, 3 ) = 1;
      translation( 1, 3 ) = 2;

      // fill with specific voxels
      v( 0, 0, 0 ) = 100;
      v( 10, 0, 0 ) = 10;
      v( 0, 5, 0 ) = 5;
      v( 10, 10, 10 ) = 200;

      // get a slice in this geometry
      Slice s( nll::core::vector3ui( 22, 22, 1 ),
               nll::core::vector3f( 1, 0, 0 ),
               nll::core::vector3f( 0, 1, 0 ),
               nll::core::vector3f( -10 + 4, 0 + 5, 0 ),
               nll::core::vector2f( 1.0f, 1.0f ) );

      Mpr mpr( v );
      nll::imaging::TransformationAffine tfm( translation );
      mpr.getSlice( s, tfm );

      
      std::cout << "print=" << std::endl;
      for ( int y = s.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < s.size()[ 0 ]; ++x )
         {
            std::cout << s( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      

      // the volume is turned 90deg clockwise, moved by (-10, 0, 0)
      // then a rotation -90deg moved by (10, 0, 0)
      // we use a regular base (1, 0, 0) and (0, 1, 0)
      // we should be back from the starting point
      TESTER_ASSERT( s( 0, 0, 0 ) == 0 );
      TESTER_ASSERT( s( 11 -1 - 4, 11 - 2 - 5, 0 ) == 100 );
      TESTER_ASSERT( s( 11 -1 - 4, 11 + 5 - 2 - 5, 0 ) == 5 );
      TESTER_ASSERT( s( 10 + 11 - 1 - 4, 11 - 2 - 5, 0 ) == 10 );
   }

   void testMprTranslationTfmSp1()
   {
      typedef nll::imaging::VolumeSpatial<float>      Volume;
      typedef nll::imaging::Slice<float>              Slice;
      typedef Volume::Matrix                          Matrix;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator > Mpr;

      Matrix pst;
      nll::core::matrix4x4RotationZ( pst, 0 );
      Volume v( nll::core::vector3ui( 16, 16, 16 ),
                pst, 0 );

      // we want to rotate the volume by -90deg and (10, 0, 0). As the transformation is defined from
      // source to target, and the slice is in source but the volume is in target, we actualy need to set
      // tfm^-1 to transform how we want to!
      Matrix translation;
      nll::core::matrix4x4RotationZ( translation, static_cast<float>( nll::core::PI / 2 ) );
      translation( 0, 3 ) = 2;
      translation( 1, 3 ) = 3;

      // fill with specific voxels
      v( 0, 0, 0 )    = 100;
      v( 2, 0, 0 )   = 10;
      v( 0, 1, 0 )    = 4;
      v( 10, 10, 10 ) = 200;

      // get a slice in this geometry
      Slice s( nll::core::vector3ui( 13, 13, 1 ),
               nll::core::vector3f( 1, 0, 0 ),
               nll::core::vector3f( 0, 1, 0 ),
               nll::core::vector3f( 0, 0, 0 ),
               nll::core::vector2f( 0.5, 0.5 ) );

      Mpr mpr( v );
      nll::imaging::TransformationAffine tfm( translation );
      mpr.getSlice( s, tfm );

      
      std::cout << "print=" << std::endl;
      for ( int y = s.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < s.size()[ 0 ]; ++x )
         {
            std::cout << s( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      

      // the volume is turned 90deg clockwise, moved by (-10, 0, 0)
      // then a rotation -90deg moved by (10, 0, 0)
      // we use a regular base (1, 0, 0) and (0, 1, 0)
      // we should be back from the starting point
      TESTER_ASSERT( s( 0, 0, 0 ) == 4 );
      TESTER_ASSERT( s( 1, 0, 0 ) == 4 );
      TESTER_ASSERT( s( 0, 1, 0 ) == 4 );
      TESTER_ASSERT( s( 1, 1, 0 ) == 4 );

      TESTER_ASSERT( s( 0 + 2, 0, 0 ) == 100 );
      TESTER_ASSERT( s( 1 + 2, 0, 0 ) == 100 );
      TESTER_ASSERT( s( 0 + 2, 1, 0 ) == 100 );
      TESTER_ASSERT( s( 1 + 2, 1, 0 ) == 100 );

      TESTER_ASSERT( s( 0 + 2, 0 + 4, 0 ) == 10 );
      TESTER_ASSERT( s( 1 + 2, 0 + 4, 0 ) == 10 );
      TESTER_ASSERT( s( 0 + 2, 1 + 4, 0 ) == 10 );
      TESTER_ASSERT( s( 1 + 2, 1 + 4, 0 ) == 10 );
   }

   void testMprTranslationTfmSp2()
   {
      typedef nll::imaging::VolumeSpatial<float>      Volume;
      typedef nll::imaging::Slice<float>              Slice;
      typedef Volume::Matrix                          Matrix;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator > Mpr;

      Matrix pst;
      nll::core::matrix4x4RotationZ( pst, 0 );
      Volume v( nll::core::vector3ui( 16, 16, 16 ),
                pst, 0 );

      // we want to rotate the volume by -90deg and (10, 0, 0). As the transformation is defined from
      // source to target, and the slice is in source but the volume is in target, we actualy need to set
      // tfm^-1 to transform how we want to!
      Matrix translation;
      nll::core::matrix4x4RotationZ( translation, static_cast<float>( nll::core::PI / 2 ) );
      translation( 0, 3 ) = 0;
      translation( 1, 3 ) = 0;

      // fill with specific voxels
      v( 0, 0, 0 )    = 100;
      v( 2, 0, 0 )   = 10;
      v( 0, 1, 0 )    = 4;
      v( 10, 10, 10 ) = 200;

      // get a slice in this geometry
      Slice s( nll::core::vector3ui( 13, 13, 1 ),
               nll::core::vector3f( 1, 0, 0 ),
               nll::core::vector3f( 0, 1, 0 ),
               nll::core::vector3f( 2, 3, 0 ),
               nll::core::vector2f( 0.5, 0.5 ) );

      Mpr mpr( v );
      nll::imaging::TransformationAffine tfm( translation );
      mpr.getSlice( s, tfm );

      
      std::cout << "print=" << std::endl;
      for ( int y = s.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < s.size()[ 0 ]; ++x )
         {
            std::cout << s( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      

      // the volume is turned 90deg clockwise, moved by (-10, 0, 0)
      // then a rotation -90deg moved by (10, 0, 0)
      // we use a regular base (1, 0, 0) and (0, 1, 0)
      // we should be back from the starting point
      TESTER_ASSERT( s( 0, 0, 0 ) == 4 );
      TESTER_ASSERT( s( 1, 0, 0 ) == 4 );
      TESTER_ASSERT( s( 0, 1, 0 ) == 4 );
      TESTER_ASSERT( s( 1, 1, 0 ) == 4 );

      TESTER_ASSERT( s( 0 + 2, 0, 0 ) == 100 );
      TESTER_ASSERT( s( 1 + 2, 0, 0 ) == 100 );
      TESTER_ASSERT( s( 0 + 2, 1, 0 ) == 100 );
      TESTER_ASSERT( s( 1 + 2, 1, 0 ) == 100 );

      TESTER_ASSERT( s( 0 + 2, 0 + 4, 0 ) == 10 );
      TESTER_ASSERT( s( 1 + 2, 0 + 4, 0 ) == 10 );
      TESTER_ASSERT( s( 0 + 2, 1 + 4, 0 ) == 10 );
      TESTER_ASSERT( s( 1 + 2, 1 + 4, 0 ) == 10 );
   }

   void testMprTranslationTfmSp3()
   {
      typedef nll::imaging::VolumeSpatial<float>      Volume;
      typedef nll::imaging::Slice<float>              Slice;
      typedef Volume::Matrix                          Matrix;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator > Mpr;

      Matrix pst;
      nll::core::matrix4x4RotationZ( pst, 0 );

      // set scaling to 0.5
      pst( 0, 0 ) = 0.5;
      pst( 1, 1 ) = 0.5;
      pst( 2, 2 ) = 0.5;
      Volume v( nll::core::vector3ui( 16, 16, 16 ),
                pst, 0 );

      // we want to rotate the volume by -90deg and (10, 0, 0). As the transformation is defined from
      // source to target, and the slice is in source but the volume is in target, we actualy need to set
      // tfm^-1 to transform how we want to!
      Matrix translation;
      nll::core::matrix4x4RotationZ( translation, static_cast<float>( nll::core::PI / 2 ) );
      translation( 0, 3 ) = 0;
      translation( 1, 3 ) = 0;

      // fill with specific voxels
      v( 0, 0, 0 )    = 100;
      v( 2, 0, 0 )   = 10;
      v( 0, 1, 0 )    = 4;
      v( 10, 10, 10 ) = 200;

      // get a slice in this geometry
      Slice s( nll::core::vector3ui( 13, 13, 1 ),
               nll::core::vector3f( 1, 0, 0 ),
               nll::core::vector3f( 0, 1, 0 ),
               nll::core::vector3f( 0, 1, 0 ),
               nll::core::vector2f( 0.5, 0.5 ) );

      Mpr mpr( v );
      nll::imaging::TransformationAffine tfm( translation );
      mpr.getSlice( s, tfm );

      
      std::cout << "print=" << std::endl;
      for ( int y = s.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < s.size()[ 0 ]; ++x )
         {
            std::cout << s( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      

      // the volume is turned 90deg clockwise, moved by (-10, 0, 0)
      // then a rotation -90deg moved by (10, 0, 0)
      // we use a regular base (1, 0, 0) and (0, 1, 0)
      // we should be back from the starting point
      TESTER_ASSERT( s( 7, 4, 0 ) == 100 );
      TESTER_ASSERT( s( 7-1, 4, 0 ) == 4 );
      TESTER_ASSERT( s( 7, 4+2, 0 ) == 10 );
   }
   
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformation);
TESTER_TEST(testAffine);
TESTER_TEST(testMprIdentity);
TESTER_TEST(testMprTranslation);
TESTER_TEST(testMprTranslationInverse);
TESTER_TEST(testMprTranslationInverse2);
TESTER_TEST(testMprTranslationTfmSp1);
TESTER_TEST(testMprTranslationTfmSp2);
TESTER_TEST(testMprTranslationTfmSp3);
TESTER_TEST_SUITE_END();
#endif

