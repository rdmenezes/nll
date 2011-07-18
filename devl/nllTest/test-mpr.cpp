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
      const core::vector3f origingResampled( 3, 4, 0 );
      const core::vector3f origing( 2, 1, 0 );

      core::Matrix<float> volPst( 4, 4 );
      core::matrix4x4RotationZ( volPst, core::PIf / 2 );
      volPst = core::createTranslation4x4( origing ) * volPst;

      volPst.print( std::cout );

      Volume vol( core::vector3ui( 5, 5, 5 ), volPst );


      Mpr::Slice slice( nll::core::vector3ui( 11, 11, 1 ),
                        nll::core::vector3f( 1, 0, 0 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        origingResampled,
                        nll::core::vector2f( 0.9999f, 0.9999f ) );


      fillVolume( vol );
      Mpr mpr( vol );
      mpr.getSlice( slice );

      print( slice );

      // expected: rotation of the volume on the left on its origin (ie, due to the PST only)
      TESTER_ASSERT( slice( 4, 2, 0 ) == 1 );
      TESTER_ASSERT( slice( 4, 3, 0 ) == 2 );
      TESTER_ASSERT( slice( 4, 4, 0 ) == 3 );
      TESTER_ASSERT( slice( 3, 2, 0 ) == 6 );
      TESTER_ASSERT( slice( 3, 3, 0 ) == 7 );
      TESTER_ASSERT( slice( 3, 4, 0 ) == 8 );
   }

   void testMPRRotPstResampled()
   {
      const core::vector3f origingResampled( 3, 4, 0 );
      const core::vector3f origing( 2, 1, 0 );
      
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      Mpr::Slice slice( nll::core::vector3ui( 12, 12, 1 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        nll::core::vector3f( -1, 0, 0 ),
                        origingResampled,
                        nll::core::vector2f( 0.99, 0.99 ) );


      fillVolume( vol );
      Mpr mpr( vol );
      mpr.getSlice( slice );

      print( slice );
      // expected: rotation of the resampled volume on the left on the point (3,4)
      // x axis is (0 1 0)
      // y axis is (-1 0 0 )
      TESTER_ASSERT( slice( 3, 7, 0 ) == 1 );
      TESTER_ASSERT( slice( 3, 6, 0 ) == 2 );
      TESTER_ASSERT( slice( 3, 5, 0 ) == 3 );
      TESTER_ASSERT( slice( 4, 7, 0 ) == 6 );
      TESTER_ASSERT( slice( 4, 6, 0 ) == 7 );
      TESTER_ASSERT( slice( 4, 5, 0 ) == 8 );
   }

   void testMPRTrans()
   {
      const core::vector3f origingResampled( 10, 15, 0 );
      const core::vector3f origing( 13, 16, 0 );
      const core::vector3f tfmMat( 2, 3, 0 );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      imaging::TransformationAffine tfm( core::createTranslation4x4( tfmMat ) );

      // expected: vol is shifted on by (-2,-3): as tfm is defined from source->target,
      // but we are displaying the target, meaning we inverse the tfm to have the target->source
      Mpr::Slice slice( nll::core::vector3ui( 12, 12, 1 ),
                        nll::core::vector3f( 1, 0, 0 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        origingResampled,
                        nll::core::vector2f( 0.99, 0.99 ) );


      fillVolume( vol );
      Mpr mpr( vol );
      mpr.getSlice( slice, tfm );
      print( slice );

      TESTER_ASSERT( slice( 7, 4, 0 )  == 1 );
      TESTER_ASSERT( slice( 8, 4, 0 )  == 2 );
      TESTER_ASSERT( slice( 9, 4, 0 )  == 3 );
      TESTER_ASSERT( slice( 7, 5, 0 )  == 6 );
      TESTER_ASSERT( slice( 8, 5, 0 )  == 7 );
      TESTER_ASSERT( slice( 9, 5, 0 )  == 8 );
   }

   void testMPRRot()
   {
      
      // TEST: results not as expected, but checked with matlab -> this is what is expected
      const core::vector3f origingResampled( 0, 0, 0 );
      const core::vector3f origing( 3, 1, 0 );
      const core::vector3f tfmTrans( 1, 2, 0 );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), core::createTranslation4x4( origingResampled ) );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      core::Matrix<float> tfmMat( 4, 4 );
      core::matrix4x4RotationZ( tfmMat, -core::PIf / 2 *1  );
      tfmMat = core::createTranslation4x4( tfmTrans ) * tfmMat;

      imaging::TransformationAffine tfm( tfmMat );

      tfm.getAffineMatrix().print( std::cout );

      // expected: rotate the volume on the world origin (0, 0, 0) to the left
      // translate by (1,-2) due to the translation of the: as it is defined source->target, we need to invert it
      // tfm = T * R, tfm^-1 = R^-1 * T^-1 => now the translation is rotated by the rotational part of tfm
      Mpr::Slice slice( nll::core::vector3ui( 12, 12, 1 ),
                  nll::core::vector3f( 1, 0, 0 ),
                  nll::core::vector3f( 0, 1, 0 ),
                  origingResampled,
                  nll::core::vector2f( 0.99, 0.99 ) );
      fillVolume( vol );
      Mpr mpr( vol );
      mpr.getSlice( slice, tfm );
      print( slice );


      TESTER_ASSERT( slice( 7, 8, 0 )  == 1 );
      TESTER_ASSERT( slice( 7, 9, 0 )  == 2 );
      TESTER_ASSERT( slice( 7, 10, 0 ) == 3 );
      TESTER_ASSERT( slice( 6, 8, 0 )  == 6 );
      TESTER_ASSERT( slice( 6, 9, 0 )  == 7 );
      TESTER_ASSERT( slice( 6, 10, 0 ) == 8 );
   }

   void testMPRSpacing()
   {
      const core::vector3f origingResampled( -1, -1, 0 );
      const core::vector3f origing( 1, 2, 0 );
      const core::vector3f spacing( 0.5, 1, 1 );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      imaging::TransformationAffine tfm( core::createScaling4x4( spacing ) );

      // expected: origin in X is multiplied by two, size x too, finally with slice origin, shifted by (1,1,0)
      // but we are displaying the target, meaning we inverse the tfm to have the target->source, this is why scaling = 2
      Mpr::Slice slice( nll::core::vector3ui( 12, 12, 1 ),
                        nll::core::vector3f( 1, 0, 0 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        origingResampled,
                        nll::core::vector2f( 0.99, 0.99 ) );


      fillVolume( vol );
      Mpr mpr( vol );
      mpr.getSlice( slice, tfm, false );
      print( slice );

      TESTER_ASSERT( slice( 3, 3, 0 )  == 1 );
      TESTER_ASSERT( slice( 4, 3, 0 )  == 1 );
      TESTER_ASSERT( slice( 5, 3, 0 )  == 2 );
      TESTER_ASSERT( slice( 6, 3, 0 )  == 2 );
      TESTER_ASSERT( slice( 7, 3, 0 )  == 3 );
      TESTER_ASSERT( slice( 8, 3, 0 )  == 3 );

      TESTER_ASSERT( slice( 3, 4, 0 )  == 6 );
      TESTER_ASSERT( slice( 4, 4, 0 )  == 6 );
      TESTER_ASSERT( slice( 5, 4, 0 )  == 7 );
      TESTER_ASSERT( slice( 6, 4, 0 )  == 7 );
      TESTER_ASSERT( slice( 7, 4, 0 )  == 8 );
      TESTER_ASSERT( slice( 8, 4, 0 )  == 8 );
   }

   
   void testMPRSpacingRot()
   {
      const core::vector3f origingResampled( -8, -5, 0 );
      const core::vector3f origing( 1, 2, 0 );
      const core::vector3f spacing( 0.5, 1, 1 );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      imaging::TransformationAffine tfm( core::createScaling4x4( spacing ) * core::getRotation4Zf( -core::PI / 2 ) );

      Mpr::Slice slice( nll::core::vector3ui( 12, 12, 1 ),
                        nll::core::vector3f( 1, 0, 0 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        origingResampled,
                        nll::core::vector2f( 0.99, 0.99 ) );


      fillVolume( vol );
      Mpr mpr( vol );
      mpr.getSlice( slice, tfm, false );
      print( slice );

      TESTER_ASSERT( slice( 6, 7, 0 )  == 1 );
      TESTER_ASSERT( slice( 6, 8, 0 )  == 1 );
      TESTER_ASSERT( slice( 6, 9, 0 )  == 2 );
      TESTER_ASSERT( slice( 6, 10, 0 )  == 2 );
      TESTER_ASSERT( slice( 6, 11, 0 )  == 3 );

      TESTER_ASSERT( slice( 5, 7, 0 )  == 6 );
      TESTER_ASSERT( slice( 5, 8, 0 )  == 6 );
      TESTER_ASSERT( slice( 5, 9, 0 )  == 7 );
      TESTER_ASSERT( slice( 5, 10, 0 )  == 7 );
      TESTER_ASSERT( slice( 5, 11, 0 )  == 8 );
   }
  
   imaging::Slice<ui8> getSlice( const Volume& volume, const imaging::TransformationAffine& tfm, const core::vector3f& origin  )
   {
      // expected: vol is shifted on by (-2,-3): as tfm is defined from source->target,
      // but we are displaying the target, meaning we inverse the tfm to have the target->source
      Mpr::Slice slice( nll::core::vector3ui( 512, 512, 1 ),
                        nll::core::vector3f( 1, 0, 0 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        origin,
                        nll::core::vector2f( 1, 1 ) );

      Mpr mpr( volume );
      mpr.getSlice( slice, tfm, true );
      
      imaging::LookUpTransformWindowingRGB lut( -250, 250, 255 );
      lut.createGreyscale();

      imaging::Slice<ui8> sliceRgb( core::vector3ui(slice.size()[ 0 ], slice.size()[ 1 ], 3 ),
                           slice.getAxisX(),
                           slice.getAxisY(),
                           slice.getOrigin(),
                           slice.getSpacing() );

      std::vector< imaging::BlendSliceInfof<imaging::LookUpTransformWindowingRGB> > sliceInfos;
      sliceInfos.push_back( imaging::BlendSliceInfof<imaging::LookUpTransformWindowingRGB>( slice, 0.9, lut ) );
      imaging::blendDummy( sliceInfos, sliceRgb );
      return sliceRgb;
   }
   
   // the test must display the same thing (except with boundary cropping/NN artifacts due to the resampled volume)
   void testComparison()
   {
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;
      typedef core::Matrix<float>                           Matrix;

      const std::string volname = NLL_TEST_PATH "data/medical/pet.mf2";
      Volume volume;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );

      
      core::vector3f center = volume.indexToPosition( core::vector3f( volume.getSize()[ 0 ] / 2,
                                                                      volume.getSize()[ 1 ] / 2,
                                                                      volume.getSize()[ 2 ] / 2 ) );


      const core::vector3f spacing( 1.1, 0.9, 0.95 );
      imaging::TransformationAffine tfm( core::createTranslation4x4( core::vector3f( 10, -5, 20 ) ) * core::createScaling4x4( spacing ) * core::getRotation4Zf( -0.1 ) * core::getRotation4Yf( -0.05 ) * core::getRotation4Xf( 0.15 ) );
      imaging::TransformationAffine tfmId( core::identityMatrix<Matrix>( 4 ) );

     
      tfm.getAffineMatrix().print( std::cout );

      imaging::Slice<ui8> sliceRgb = getSlice( volume, tfm, center );
      core::writeBmp( sliceRgb.getStorage(), NLL_TEST_PATH "data/comparisonMpr-mpr.bmp" );

      Volume resampled( volume.getSize(), volume.getPst() );
      resampleVolumeTrilinear( volume, resampled, tfm );

      sliceRgb = getSlice( resampled, tfmId, center );
      core::writeBmp( sliceRgb.getStorage(), NLL_TEST_PATH "data/comparisonMpr-resampled.bmp" );

   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMPR);
TESTER_TEST( testMPROriginNoTfm );
TESTER_TEST( testMPRRotPstVol );
TESTER_TEST( testMPRRotPstResampled );
TESTER_TEST( testMPRTrans );
TESTER_TEST( testMPRRot );
TESTER_TEST( testMPRSpacingRot )
TESTER_TEST( testMPRSpacing );
TESTER_TEST( testComparison );
TESTER_TEST_SUITE_END();
#endif
