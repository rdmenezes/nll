#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

class TestVolumeResampling
{
public:
   typedef  imaging::VolumeSpatial<int>  Volume;
   typedef  imaging::VolumeSpatial<float>  Volumef;

   void fillVolume( Volume& v )
   {
      size_t n = 1;
      for ( size_t z = 0; z < v.size()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < v.size()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < v.size()[ 0 ]; ++x )
            {
               v( x, y, z ) = n++;
            }
         }
      }
   }

   void print( Volume& v )
   {
      for ( i32 y = v.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( size_t x = 0; x < v.size()[ 0 ]; ++x )
         {
            std::cout << v( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
   }

   // basic test checking the volume is correctly resampled, just with different origins
   void testResamplingOriginNoTfm()
   {
      int dx = 1;
      int dy = 2;
      int dz = 0;

      const core::vector3f origingResampled( 10, 15, 20 );
      const core::vector3f origing( origingResampled[ 0 ] + dx,
                                    origingResampled[ 1 ] + dy,
                                    origingResampled[ 2 ] + dz );
      Volume resampled(  core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origingResampled ) );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      fillVolume( vol );
      resampleVolumeNearestNeighbour( vol, resampled );

      print( resampled );

      // expected: simple translation of the volume
      // sourceOriginComparedToTarget=(-1, -2, 0)
      TESTER_ASSERT( resampled( 1, 2, 0 ) == 1 );
      TESTER_ASSERT( resampled( 2, 2, 0 ) == 2 );
      TESTER_ASSERT( resampled( 3, 2, 0 ) == 3 );

      TESTER_ASSERT( resampled( 1, 3, 0 ) == 6 );
      TESTER_ASSERT( resampled( 2, 3, 0 ) == 7 );
      TESTER_ASSERT( resampled( 3, 3, 0 ) == 8 );
   }

   void testResamplingRotPstVol()
   {
      const core::vector3f origingResampled( -5, -5, 0 );
      const core::vector3f origing( 2, 1, 0 );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), core::createTranslation4x4( origingResampled ) );

      core::Matrix<float> volPst( 4, 4 );
      core::matrix4x4RotationZ( volPst, core::PIf / 2 );
      volPst = core::createTranslation4x4( origing ) * volPst;
      Volume vol( core::vector3ui( 5, 5, 5 ), volPst );



      fillVolume( vol );
      resampleVolumeNearestNeighbour( vol, resampled );

      print( resampled );

      // expected: rotation of the volume on the left on its origin (ie, due to the PST only)
      // sourceOriginComparedToTarget=(-6, 7, 0)
      TESTER_ASSERT( resampled( 7, 6, 0 ) == 1 );
      TESTER_ASSERT( resampled( 7, 7, 0 ) == 2 );
      TESTER_ASSERT( resampled( 7, 8, 0 ) == 3 );
      TESTER_ASSERT( resampled( 6, 6, 0 ) == 6 );
      TESTER_ASSERT( resampled( 6, 7, 0 ) == 7 );
      TESTER_ASSERT( resampled( 6, 8, 0 ) == 8 );
   }

   void testResamplingRotPstResampled()
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

   void testResamplingTrans()
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
      resampleVolumeNearestNeighbour( vol, tfm, resampled );

      print( resampled );

      print( vol );

      TESTER_ASSERT( resampled( 1, 0, 0 ) == 6 );
      TESTER_ASSERT( resampled( 2, 0, 0 ) == 7 );
      TESTER_ASSERT( resampled( 3, 0, 0 ) == 8 );
      TESTER_ASSERT( resampled( 1, 1, 0 ) == 11 );
      TESTER_ASSERT( resampled( 2, 1, 0 ) == 12 );
      TESTER_ASSERT( resampled( 3, 1, 0 ) == 13 );
   }

   void testResamplingRot()
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
      resampleVolumeNearestNeighbour( vol, tfm, resampled );

      print( resampled );


      TESTER_ASSERT( resampled( 8, 6, 0 ) == 1 );
      TESTER_ASSERT( resampled( 8, 7, 0 ) == 2 );
      TESTER_ASSERT( resampled( 8, 8, 0 ) == 3 );
      TESTER_ASSERT( resampled( 7, 6, 0 ) == 6 );
      TESTER_ASSERT( resampled( 7, 7, 0 ) == 7 );
      TESTER_ASSERT( resampled( 7, 8, 0 ) == 8 );
   }

   void testResamplingPerf()
   {
      const std::string volname = NLL_TEST_PATH "data/medical/pet.mf2";

      Volumef volume;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );

      Volumef volume2( volume.getSize(), volume.getPst() );
/*
      core::Timer timer;
      imaging::resampleVolumeTrilinear2( volume, volume2 );
      std::cout << "Time=" << timer.getCurrentTime() << std::endl;
*/
      core::Matrix<float> tfmMat( 4, 4 );
      core::matrix4x4RotationZ( tfmMat, 0.01 );
      const core::vector3f tfmTrans( 2.1, 4.8, -1.5 );
      tfmMat = core::createTranslation4x4( tfmTrans ) * tfmMat;
      imaging::TransformationAffine tfm( tfmMat );

      Volumef volume3( volume.getSize(), volume.getPst() );

      core::Timer timer2;
      imaging::resampleVolumeTrilinear( volume, tfm, volume3 );
      std::cout << "Time=" << timer2.getCurrentTime() << std::endl;

      imaging::LookUpTransformWindowingRGB lut(-300,300, 256 );
      lut.createGreyscale();

      /*
      core::Timer timer4;
      core::vector3f b2 = imaging::computeBarycentre2( volume, lut );
      std::cout << "Time=" << timer4.getCurrentTime() << std::endl;
*/
      core::Timer timer3;
      core::vector3f b = imaging::computeBarycentre( volume, lut );
      std::cout << "Time=" << timer3.getCurrentTime() << std::endl;


      std::cout << "b=" << b;
      //std::cout << "b=" << b2;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeResampling);
TESTER_TEST( testResamplingOriginNoTfm );
TESTER_TEST( testResamplingRotPstVol );
TESTER_TEST( testResamplingRotPstResampled );
TESTER_TEST( testResamplingTrans );
TESTER_TEST( testResamplingRot );
TESTER_TEST( testResamplingPerf );
TESTER_TEST_SUITE_END();
#endif
