#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
  
}
}

class TestRegistration3d
{
   typedef core::Matrix<double>           Matrix;

   typedef core::Matrix<float>                     Matrixf;
   typedef imaging::VolumeSpatial<float>           Volume;
   typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;
   typedef imaging::Mpr<Volume, Interpolator>      Mpr;
   typedef imaging::Slice<float>                   Slicef;
   typedef imaging::Slice<ui8>                     Slicec;

public:
   static Slicec getRgbSlice( const imaging::LookUpTransformWindowingRGB& lut, const Slicef& slice )
   {
      core::vector3ui sizeRgb( slice.size()[ 0 ], slice.size()[ 1 ], 3 );
      Slicec sliceRgb( sizeRgb, slice.getAxisX(), slice.getAxisY(), slice.getOrigin(), slice.getSpacing() );
      std::vector< imaging::BlendSliceInfof<imaging::LookUpTransformWindowingRGB> > sliceInfos;
      sliceInfos.push_back( imaging::BlendSliceInfof<imaging::LookUpTransformWindowingRGB>( slice, 0.9, lut ) );
      imaging::blendDummy( sliceInfos, sliceRgb );
      return sliceRgb;
   }

   // get a RGB slice of a preprocessed volume
   static Slicec getRgbSlice( const Volume& v, const core::vector3f& position )
   {
      imaging::LookUpTransformWindowingRGB lutId( 0, 255, 256 );
      lutId.createGreyscale();

      Mpr mpr( v );
      Slicef slice( core::vector3ui( 512, 512, 1 ), core::vector3f( 1, 0, 0 ), core::vector3f( 0, 1, 0 ), position, core::vector2f( 1, 1 ) );
      mpr.getSlice( slice );

      core::vector3ui sizeRgb;
      Slicec sliceRgb = getRgbSlice( lutId, slice );
      return sliceRgb;
   }

   // compute registration between two normalized volumes
   template <class Registrator>
   static Matrix computeRegistration( const Registrator& registrator, const Volume& source, const Volume& target, const imaging::LookUpTransformWindowingRGB& lut )
   {
      // extract 3D points
      algorithm::SpeededUpRobustFeatures3d surf( 5, 4, 2, 0.00005 );
      algorithm::SpeededUpRobustFeatures3d::Points sourcePoints = surf.computesFeatures( source );
      algorithm::SpeededUpRobustFeatures3d::Points targetPoints = surf.computesFeatures( target );

      std::cout << "nbPointsSource=" << sourcePoints.size() << std::endl;
      std::cout << "nbPointsTarget=" << targetPoints.size() << std::endl;

      Matrix tfm = registrator.process( targetPoints, sourcePoints );
      return tfm;
   }

   // normalize a volume
   Volume prepareVolume( const Volume& input, const imaging::LookUpTransformWindowingRGB& lut )
   {
      // we ABSOLUTELY need to use the same spacing [1 1 1] for the volume to get the normal transformation
      // also we need to be in the same geometric space!!
      const core::Matrix<float> targetOriginalPst = core::createTranslation4x4( input.getOrigin() ) *
                                                    core::createScaling4x4( core::vector3f( 1, 1, 1 ) );
      Volume target( core::vector3ui( 350, 350, 256 ), targetOriginalPst );
      imaging::resampleVolumeTrilinear( input, target );
      target.setOrigin( core::vector3f( 0, 0, 0 ) );

      for ( size_t n = 0; n < target.getStorage().size(); ++n )
      {
         float& value = target.getStorage()[ n ];
         value = lut.transform( value )[ 0 ];
      }

      return target;
   }

   void testIdentity()
   {
      imaging::LookUpTransformWindowingRGB lut( 00, 5000, 255 );
      lut.createGreyscale();

      // load a volume
      Volume target;
      const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/pet-NAC.mf2", target );
      ensure( loaded, "Error!" );

      typedef algorithm::RegistrationCloudOfPoints3d<> Registrator3d;
      Registrator3d registrator;
      const Matrix tfm = computeRegistration( registrator, target, target, lut );

      TESTER_ASSERT( tfm.equal( core::identityMatrix<Matrix>( 4 ), 1e-4 ) );
      TESTER_ASSERT( registrator.getDebugInfo().inliers.size() >= 0.99 * registrator.getDebugInfo().nbSourcePoints );
   }

   void testSimpleTfm()
   {
      imaging::LookUpTransformWindowingRGB lut( 0, 1000, 255 );
      lut.createGreyscale();

      srand( 1 );
      for ( size_t n = 0; n < 10; ++n )
      {
         std::cout << "case="<< n << std::endl;
         core::vector3f translation( core::generateUniformDistribution( -30, 30 ),
                                     core::generateUniformDistribution( -30, 30 ),
                                     core::generateUniformDistribution( -30, 30 ) );

         const float scale = core::generateUniformDistribution( 0.9, 1.1 );
         core::vector3f scaling( scale, scale, scale );
         core::vector3f rotation( core::generateUniformDistribution(-0.2, 0.2), core::generateUniformDistribution(-0.2, 0.2), 0 );

         const core::Matrix<float> tfmMatrixExpected = core::createTranslation4x4( translation ) *
                                                       core::getRotation4Zf( rotation[ 2 ] ) *
                                                       core::getRotation4Zf( rotation[ 1 ] ) *
                                                       core::getRotation4Zf( rotation[ 0 ] ) *
                                                       core::createScaling4x4( scaling );

         // load a volume
         Volume targetOriginal;
         const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", targetOriginal );
         ensure( loaded, "Error!" );

         const Volume target = prepareVolume( targetOriginal, lut );

         Volume targetResampled( target.size(), target.getPst(), target.getBackgroundValue() );
         imaging::TransformationAffine tfmExpected( tfmMatrixExpected );
         imaging::resampleVolumeTrilinear( target, tfmExpected, targetResampled );

         {
            const core::vector3f targetCenter = target.indexToPosition( core::vector3f( target.size()[ 0 ] / 2, target.size()[ 1 ] / 2, target.size()[ 2 ] / 2 ) );
            Slicec slice = getRgbSlice( target, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/full-target-"+ core::val2str( n ) + ".bmp" );
            slice = getRgbSlice( targetResampled, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/full-targetResampled-"+ core::val2str( n ) + ".bmp" );
         }

         std::cout << "TfmExpected="<< std::endl;
         tfmExpected.getAffineMatrix().print( std::cout );

         typedef algorithm::RegistrationCloudOfPoints3d<> Registrator3d;
         Registrator3d registrator;
         const Matrix tfm = computeRegistration( registrator, target, targetResampled, lut );

         std::cout << "TfmFound="<< std::endl;
         tfm.print( std::cout );

         std::cout << "VolumeSize=" << target.size();

         Matrix tfmExpectedd;
         tfmExpectedd.import( tfmExpected.getAffineMatrix() );

         const Matrix tfmRot         = core::getRotationAndSpacing( tfm );
         const Matrix tfmRotExpected = core::getRotationAndSpacing( tfmExpectedd );

         const double tolTranslation = 4;
         TESTER_ASSERT( tfmRot.equal( tfmRotExpected, 0.05 ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 0, 3 ), tfm( 0, 3 ), tolTranslation ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 1, 3 ), tfm( 1, 3 ), tolTranslation ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 2, 3 ), tfm( 2, 3 ), tolTranslation ) );
      }
   }

   // here we give exact points. We want to test that the point matching mecanism works well
   void testSimpleTfmGivenPoints()
   {
      imaging::LookUpTransformWindowingRGB lut( 0, 1000, 255 );
      lut.createGreyscale();

      srand( 1 );
      for ( size_t n = 0; n < 10; ++n )
      {
         std::cout << "case="<< n << std::endl;

         // create a random transformation
         
         core::vector3f translation( core::generateUniformDistribution( -30, 30 ),
                                     core::generateUniformDistribution( -30, 30 ),
                                     core::generateUniformDistribution( -30, 30 ) );
                                     
         const float scale = core::generateUniformDistribution( 0.9, 1.1 );
         core::vector3f scaling( scale, scale, scale );
         core::vector3f rotation( core::generateUniformDistribution(-0.2, 0.2), core::generateUniformDistribution(-0.2, 0.2), 0 );
         const core::Matrix<float> tfmMatrixExpected = core::createTranslation4x4( translation ) *
                                                       core::getRotation4Zf( rotation[ 2 ] ) *
                                                       core::getRotation4Zf( rotation[ 1 ] ) *
                                                       core::getRotation4Zf( rotation[ 0 ] ) *
                                                       core::createScaling4x4( scaling );

         core::Matrix<float> tfmMatrixExpectedInv;
         tfmMatrixExpectedInv.clone( tfmMatrixExpected );
         core::inverse( tfmMatrixExpectedInv );

         // load a volume
         Volume targetOriginal;
         const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", targetOriginal );
         ensure( loaded, "Error!" );

         // create the source and target volumes
         const Volume target = prepareVolume( targetOriginal, lut );

         Volume targetResampled( target.size(), target.getPst(), target.getBackgroundValue() );
         imaging::TransformationAffine tfmExpected( tfmMatrixExpected );
         imaging::resampleVolumeTrilinear( target, tfmExpected, targetResampled );

         {
            const core::vector3f targetCenter = target.indexToPosition( core::vector3f( target.size()[ 0 ] / 2, target.size()[ 1 ] / 2, target.size()[ 2 ] / 2 ) );
            Slicec slice = getRgbSlice( target, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/simple-target-"+ core::val2str( n ) + ".bmp" );
            slice = getRgbSlice( targetResampled, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/simple-targetResampled-"+ core::val2str( n ) + ".bmp" );
         }

         {
            const core::vector3f sourceCenter = target.indexToPosition( core::vector3f( 168, 192, 153 ) );
            const core::vector3f targetCenter = targetResampled.indexToPosition( core::vector3f( 138, 192, 153 ) );
            Slicec slice = getRgbSlice( target, sourceCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/point-target-"+ core::val2str( n ) + ".bmp" );
            slice = getRgbSlice( targetResampled, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/point-targetResampled-"+ core::val2str( n ) + ".bmp" );
         }

         // get the points in the target and create the corresponding points in the targetResampled
         algorithm::SpeededUpRobustFeatures3d surf( 5, 4, 2, 0.00001 );
         algorithm::SpeededUpRobustFeatures3d::Points sourcePoints = surf.computesFeatures( target );

         std::cout << "nbPoints=" << sourcePoints.size() << std::endl;

         algorithm::SpeededUpRobustFeatures3d::Points targetPoints( sourcePoints.size() );
         for ( size_t n = 0; n < sourcePoints.size(); ++n )
         {
            algorithm::SpeededUpRobustFeatures3d::Point point;
            const core::vector3f input( sourcePoints[ n ].position[ 0 ],
                                        sourcePoints[ n ].position[ 1 ],
                                        sourcePoints[ n ].position[ 2 ] );
            const core::vector3f output = core::transf4( tfmMatrixExpectedInv, input );

            point.position = core::vector3i( output[ 0 ], output[ 1 ], output[ 2 ] );
            point.scale = sourcePoints[ n ].scale;
            point.weight = sourcePoints[ n ].weight;
            targetPoints[ n ] = point;
         }

         surf.computeFeatures( targetResampled, targetPoints );

         std::cout << "v=" << target( 152, 258, 14 ) << std::endl;

         // now we have the full points, so perform the registration. Ideally the matching will be perfect!
         typedef algorithm::RegistrationCloudOfPoints3d<> Registrator3d;
         Registrator3d registrator;
         const Matrix tfm = registrator.process( targetPoints, sourcePoints, 64 /* i.e. 4*4*4 voxels */, 20, 10000, 5000 );

         // check the matches
         size_t nbGoodMatches = 0;
         size_t nbZeroLength = 0;
         for ( size_t n = 0; n < registrator.getDebugInfo().matches.size(); ++n )
         {
            if ( registrator.getDebugInfo().matches[ n ].index1 == registrator.getDebugInfo().matches[ n ].index2 )
            {
               ++nbGoodMatches;
            }
         }

         for ( size_t n = 0; n < sourcePoints.size(); ++n )
         {
            if ( core::norm2( sourcePoints[ n ].features ) < 1e-8 )
            {
               ++nbZeroLength;
            }
         }

         std::cout << "zeroLength=" << nbZeroLength << std::endl;

         const double goodMatchRatio = nbGoodMatches / (double)  registrator.getDebugInfo().matches.size();
         std::cout << "goodMatchRatio=" << goodMatchRatio << std::endl;

         const double inlierRatio =  registrator.getDebugInfo().inliers.size() / (double)targetPoints.size();
         std::cout << "inlierRatio=" << inlierRatio << std::endl;

         tfmExpected.getAffineMatrix().print( std::cout );
         tfm.print( std::cout );

         Matrix tfmExpectedd;
         tfmExpectedd.import( tfmExpected.getAffineMatrix() );

         const Matrix tfmRot         = core::getRotationAndSpacing( tfm );
         const Matrix tfmRotExpected = core::getRotationAndSpacing( tfmExpectedd );

         const double tolTranslation = 4;
         TESTER_ASSERT( tfmRot.equal( tfmRotExpected, 0.05 ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 0, 3 ), tfm( 0, 3 ), tolTranslation ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 1, 3 ), tfm( 1, 3 ), tolTranslation ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 2, 3 ), tfm( 2, 3 ), tolTranslation ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRegistration3d);
TESTER_TEST(testIdentity);
TESTER_TEST(testSimpleTfm);
TESTER_TEST(testSimpleTfmGivenPoints);
TESTER_TEST_SUITE_END();
#endif