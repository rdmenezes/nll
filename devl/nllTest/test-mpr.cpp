#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

class TestMPR
{
public:
   typedef  imaging::VolumeSpatial<int>                     Volume;
   typedef  imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
   typedef  imaging::Mpr<Volume, Interpolator>              Mpr;

   void fillVolume( Volume& v )
   {
      ui32 n = 1;
      for ( ui32 z = 0; z < v.size()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < v.size()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < v.size()[ 0 ]; ++x )
            {
               v( x, y, z ) = n++;
            }
         }
      }
   }

   void print( Mpr::Slice& v )
   {
      for ( i32 y = v.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( ui32 x = 0; x < v.size()[ 0 ]; ++x )
         {
            std::cout << v( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
   }

   // basic test checking the volume is correctly resampled, just with different origins
   void testMPROriginNoTfm()
   {
      int dx = 1;
      int dy = 2;
      int dz = 0;

      const core::vector3f origingResampled( 11, 16, 20 );
      const core::vector3f origing( 11,
                                    17,
                                    20 );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      Mpr::Slice slice( nll::core::vector3ui( 6, 6, 1 ),
                        nll::core::vector3f( 1, 0, 0 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        origingResampled,
                        nll::core::vector2f( 1.0f, 1.0f ) );


      fillVolume( vol );
      Mpr mpr( vol );
      mpr.getSlice( slice );

      print( slice );

      // expected: simple translation of the volume // don't forget: (0, 0, 0) of a slice is its center!
      // sourceOriginComparedToTarget=(-1, -2, 0)
      TESTER_ASSERT( slice( 3, 4, 0 ) == 1 );
      TESTER_ASSERT( slice( 4, 4, 0 ) == 2 );
      TESTER_ASSERT( slice( 5, 4, 0 ) == 3 );

      TESTER_ASSERT( slice( 3, 5, 0 ) == 6 );
      TESTER_ASSERT( slice( 4, 5, 0 ) == 7 );
      TESTER_ASSERT( slice( 5, 5, 0 ) == 8 );
   }

   
   void testMPRRotPstVol()
   {
      const core::vector3f origingResampled( 0, 0, 0 );
      const core::vector3f origing( 2, 1, 0 );

      core::Matrix<float> volPst( 4, 4 );
      core::matrix4x4RotationZ( volPst, core::PIf / 2 );
      volPst = core::createTranslation4x4( origing ) * volPst;

      volPst.print( std::cout );

      Volume vol( core::vector3ui( 5, 5, 5 ), volPst );


      Mpr::Slice slice( nll::core::vector3ui( 12, 12, 1 ),
                        nll::core::vector3f( 1, 0, 0 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        origingResampled,
                        nll::core::vector2f( 1.0f, 1.0f ) );


      fillVolume( vol );
      Mpr mpr( vol );
      mpr.getSlice( slice );

      print( slice );

      // expected: rotation of the volume on the left on its origin (ie, due to the PST only)
      // sourceOriginComparedToTarget=(-6, 7, 0)
      TESTER_ASSERT( slice( 7, 6, 0 ) == 1 );
      TESTER_ASSERT( slice( 7, 7, 0 ) == 2 );
      TESTER_ASSERT( slice( 7, 8, 0 ) == 3 );
      TESTER_ASSERT( slice( 6, 6, 0 ) == 6 );
      TESTER_ASSERT( slice( 6, 7, 0 ) == 7 );
      TESTER_ASSERT( slice( 6, 8, 0 ) == 8 );
   }
/*
   void testMPRRotPstResampled()
   {
      const core::vector3f origingResampled( 4, 2, 0 );
      const core::vector3f origing( 1, 1, 0 );
      
      core::Matrix<float> volPst( 4, 4 );
      core::matrix4x4RotationZ( volPst, core::PIf / 2 );
      volPst = core::createTranslation4x4( origingResampled ) * volPst;
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), volPst );

      fillVolume( vol );
      resampleVolumeNearestNeighbour( vol, resampled );

      print( resampled );

      // expected: rotation of the resampled volume on the left on the point (4,2)
      // x axis is (0 1 0)
      // y axis is (-1 0 0 )
      TESTER_ASSERT( resampled( 0, 0, 0 ) == 9 );
      TESTER_ASSERT( resampled( 1, 0, 0 ) == 14 );
      TESTER_ASSERT( resampled( 2, 0, 0 ) == 19 );
      TESTER_ASSERT( resampled( 0, 1, 0 ) == 8 );
      TESTER_ASSERT( resampled( 1, 1, 0 ) == 13 );
      TESTER_ASSERT( resampled( 2, 1, 0 ) == 18);
   }

   void testMPRTrans()
   {
      const core::vector3f origingResampled( 10, 15, 0 );
      const core::vector3f origing( 13, 16, 0 );
      const core::vector3f tfmMat( 2, 2, 0 );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), core::createTranslation4x4( origingResampled ) );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      imaging::TransformationAffine tfm( core::createTranslation4x4( tfmMat ) );

      // expected: vol is shifted on by (-2,-2): tfm is defined from source->target,
      // but we are resampling the target, meaning we inverse the tfm to have the target->source
      fillVolume( vol );
      resampleVolumeNearestNeighbour( vol, resampled, tfm );

      print( resampled );

      TESTER_ASSERT( resampled( 1, 0, 0 ) == 6 );
      TESTER_ASSERT( resampled( 2, 0, 0 ) == 7 );
      TESTER_ASSERT( resampled( 3, 0, 0 ) == 8 );
      TESTER_ASSERT( resampled( 1, 1, 0 ) == 11 );
      TESTER_ASSERT( resampled( 2, 1, 0 ) == 12 );
      TESTER_ASSERT( resampled( 3, 1, 0 ) == 13 );
   }

   void testMPRRot()
   {
      
      // TEST: results not as expected, but checked with matlab -> this is what is expected
      const core::vector3f origingResampled( -5, -5, 0 );
      const core::vector3f origing( 3, 1, 0 );
      const core::vector3f tfmTrans( 2, 4, 0 );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), core::createTranslation4x4( origingResampled ) );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      core::Matrix<float> tfmMat( 4, 4 );
      core::matrix4x4RotationZ( tfmMat, -core::PIf / 2 );
      tfmMat = core::createTranslation4x4( tfmTrans ) * tfmMat;

      imaging::TransformationAffine tfm( tfmMat );

      // expected: the coordinate system is rotated first by the PST. Within this coodinate sytem, do the TFM
      fillVolume( vol );
      resampleVolumeNearestNeighbour( vol, resampled, tfm );

      print( resampled );


      TESTER_ASSERT( resampled( 8, 6, 0 ) == 1 );
      TESTER_ASSERT( resampled( 8, 7, 0 ) == 2 );
      TESTER_ASSERT( resampled( 8, 8, 0 ) == 3 );
      TESTER_ASSERT( resampled( 7, 6, 0 ) == 6 );
      TESTER_ASSERT( resampled( 7, 7, 0 ) == 7 );
      TESTER_ASSERT( resampled( 7, 8, 0 ) == 8 );
   }
   */
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMPR);
//TESTER_TEST( testMPROriginNoTfm );
TESTER_TEST( testMPRRotPstVol );
/*TESTER_TEST( testMPRRotPstResampled );
TESTER_TEST( testMPRTrans );
TESTER_TEST( testMPRRot );*/
TESTER_TEST_SUITE_END();
#endif
