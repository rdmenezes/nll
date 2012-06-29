#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"
#include "utils.h"

using namespace nll;

namespace nll
{
namespace imaging
{
   
}
}


class TestTransformationMapperDdf3D
{
   typedef core::Matrix<float>            Matrix;
   typedef imaging::VolumeSpatial<float>  Volume;
   typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;
   typedef imaging::Mpr<Volume, Interpolator>      Mpr;
   typedef imaging::Slice<float>                   Slicef;
   typedef imaging::Slice<ui8>                     Slicec;

public:
   // here just take the test of the affine mapping TestVolumeMapper::testSimpleMapping(), and check we still have the same result with a DDF
   void testSimpleAffineMappingOnly()
   {
      srand( 0 );
      Matrix pstTarget = core::createTransformationAffine3D(core::vector3f( -10, -20, -30 ),
                                                            core::vector3f( 0.1, 0, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 1, 1.05, 0.95 ) );
      Matrix pstResampled = core::createTransformationAffine3D(core::vector3f( 1, -2, -3 ),
                                                            core::vector3f( 0, 0.1, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 1.1, 1, 0.95 ) );
      Matrix affineTfm = core::createTransformationAffine3D(core::vector3f(-5, -0, -1 ),
                                                            core::vector3f( 0, 0, -0.15 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 1, 1.1, 1.03 ) );
      Volume resampled( core::vector3ui( 256, 512, 300 ), pstResampled );
      Volume target( core::vector3ui( 256, 512, 300 ), pstTarget );

      // DDF set up
      Matrix tfm = core::createTransformationAffine3D( core::vector3f( 1, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 1, 1, 1 ) );
      core::vector3ui ddfSize( 10, 15, 20 );
      core::vector3f sizeMm( target.size()[ 0 ] * target.getSpacing()[ 0 ],
                             target.size()[ 1 ] * target.getSpacing()[ 0 ],
                             target.size()[ 2 ] * target.getSpacing()[ 0 ] );
      imaging::TransformationDenseDeformableField ddf = imaging::TransformationDenseDeformableField::create( affineTfm, pstTarget, sizeMm, ddfSize );

      ddf.getGradient( core::vector3f() );

      for ( Volume::iterator it = target.begin(); it != target.end(); ++it )
      {
         *it = core::generateUniformDistributionf( 100, 500 );
      }      

      core::Timer timer;
      imaging::VolumeTransformationProcessorResampler<Volume, Interpolator> resampler( target, resampled );
      imaging::VolumeTransformationMapperDdf mapper;
      mapper.run( resampler, target, ddf, resampled );
      std::cout << "Time DDF resampling 256*512*300=" << timer.getCurrentTime() << std::endl;

      std::cout << resampled( 0, 0, 0 ) << std::endl;
      std::cout << resampled( 8, 0, 0 ) << std::endl;
      std::cout << resampled( 0, 9, 0 ) << std::endl;
      std::cout << resampled( 0, 0, 10 ) << std::endl;
      std::cout << resampled( 12, 30, 40 ) << std::endl;

      // these values were computed using this library and are for regression tests only (it is assumed correct...)
      {
         const float v = resampled( 0, 0, 0 );
         TESTER_ASSERT( core::equal<float>( v, 221.367, 1e-1 ) );
      }

      {
         const float v = resampled( 8, 0, 0 );
         TESTER_ASSERT( core::equal<float>( v, 267.024, 1e-1 ) );
      }
      {
         const float v = resampled( 0, 9, 0 );
         TESTER_ASSERT( core::equal<float>( v, 266.664, 1e-1 ) );
      }
      {
         const float v = resampled( 0, 0, 10 );
         TESTER_ASSERT( core::equal<float>( v, 256.558, 1e-1 ) );
      }
      {
         const float v = resampled( 12, 30, 40 );
         TESTER_ASSERT( core::equal<float>( v, 336.272, 1e-1 ) );
      }
   }

   void testDdfConversionFromRbf()
   {
      srand( 1 );
      Matrix pstTarget = core::createTransformationAffine3D(core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 1, 1, 1 ) );
      Matrix affineTfm = core::createTransformationAffine3D(core::vector3f(10, 20, 30 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 1, 1, 1 ) );
      Volume target( core::vector3ui( 256, 128, 64 ), pstTarget );

      // DDF set up
      Matrix tfm = core::createTransformationAffine3D( core::vector3f( 1, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 1, 1, 1 ) );
      core::vector3ui ddfSize( 100, 100, 100 );
      core::vector3f sizeMm( target.size()[ 0 ] * target.getSpacing()[ 0 ],
                             target.size()[ 1 ] * target.getSpacing()[ 0 ],
                             target.size()[ 2 ] * target.getSpacing()[ 0 ] );

      typedef core::DeformableTransformationRadialBasis<> RbfTransform;

      std::vector<RbfTransform::Rbf> rbfs;
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( -5, -7, -3 ),
                                         core::make_buffer1D<float>( 128, 64, 32 ),
                                         core::make_buffer1D<float>( 160, 160, 160 ) ) );
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( 4, 8, 2 ),
                                         core::make_buffer1D<float>( 0, 0, 0 ),
                                         core::make_buffer1D<float>( 160, 160, 160 ) ) );
      
      RbfTransform tfmRbf( affineTfm, rbfs );

      imaging::TransformationDenseDeformableField ddf = imaging::TransformationDenseDeformableField::create( tfmRbf, pstTarget, sizeMm, ddfSize );

      {
         const core::vector3f p( 3, 8, -1 );
         const core::vector3f pExpected = core::transf4( affineTfm, p );
         const core::vector3f pResult = ddf.transformAffineOnly( p );
         TESTER_ASSERT( (pExpected - pResult).norm2() < 1e-5 );
      }

      {
         core::vector3f p( -10 - 0 + 0,
                           -20 - 0 + 0,
                           -30 - 0 + 0 );

         const core::vector3f fp = ddf.transformDeformableOnly( p );
         TESTER_ASSERT( (fp - core::vector3f(4, 8, 2)).norm2() < 1e-3 );
      }

      // now test against expected value
      {
         core::vector3f p( -10 - 0 + 128,
                           -20 - 0 + 64,
                           -30 - 0 + 32 );

         const core::vector3f fp = ddf.transformDeformableOnly( p );
         TESTER_ASSERT( (fp - core::vector3f(-5, -7, -3)).norm2() < 1e-1 );
      }

      // now randomly test points
      for ( size_t n = 0; n < 200000; ++n )
      {
         const core::vector3f p( core::generateUniformDistributionf( -100, 2560 - 100 ) / 10,
                                 core::generateUniformDistributionf( -200, 1280 - 200 ) / 10,
                                 core::generateUniformDistributionf( -300, 640 - 300 ) / 10 );
         const core::vector3f pInDdfMm = core::transf4( affineTfm, p );
         if ( pInDdfMm[ 0 ] > 0 && pInDdfMm[ 1 ] > 0 && pInDdfMm[ 2 ] > 0 &&
              pInDdfMm[ 0 ] <256 && pInDdfMm[ 1 ] <128 && pInDdfMm[ 2 ] < 64 )
         {
            // forward
            const core::Buffer1D<float> def = tfmRbf.getRawDeformableDisplacementOnly( pInDdfMm );
            const core::vector3f forwardExpected( pInDdfMm[ 0 ] + def[ 0 ],
                                                  pInDdfMm[ 1 ] + def[ 1 ],
                                                  pInDdfMm[ 2 ] + def[ 2 ] );
            const core::vector3f forward = ddf.transform( p );
            const double err = (forward - forwardExpected).norm2();
			   if ( err >= 0.4 )
			   {
				   std::cout << "ERROR=" << err << " iter=" << n << std::endl;
			   }
            TESTER_ASSERT( err < 0.4 );

            bool converged = false;
            const core::vector3f backward = ddf.getInverseTransform( forward, 1000, &converged );
			   //const core::vector3f backward = ddf.getInverseTransform_experimental( forward, 1000, &converged );
            //const double err2 = (backward - p).norm2();
			   const double err2 = (ddf.transform(backward) - forward).norm2();
            if ( err2 >= 0.5 || !converged )
            {
               std::cout << "error=" << err2 << "iter="<< n << std::endl;
               std::cout << backward  << p;
            }
			   ensure( converged, "didn't converged!!!" );
            TESTER_ASSERT( err2 <= 0.5 );
         }
      }
   }

   void testGridOverlay()
   {
      srand( 1 );
      Matrix pstTarget = core::createTransformationAffine3D(core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 1, 1, 1 ) );
      Matrix affineTfm = core::createTransformationAffine3D(core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 0, 0, 0 ),
                                                            core::vector3f( 1, 1, 1 ) );
      Volume target( core::vector3ui( 256, 256, 64 ), pstTarget );

      // DDF set up: first, create a RBF, then import a DDF from RBF.
      core::vector3ui ddfSize( 64, 64, 64 );
      core::vector3f sizeMm( target.size()[ 0 ] * target.getSpacing()[ 0 ],
                             target.size()[ 1 ] * target.getSpacing()[ 0 ],
                             target.size()[ 2 ] * target.getSpacing()[ 0 ] );

      typedef core::DeformableTransformationRadialBasis<> RbfTransform;

      std::vector<RbfTransform::Rbf> rbfs;
      
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( 1, 20, 0 ),
                                         core::make_buffer1D<float>( 128, 64, 20 ),
                                         core::make_buffer1D<float>( 400, 400, 400 ) ) );
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( -20, 0, 0 ),
                                         core::make_buffer1D<float>( 64, 64, 20 ),
                                         core::make_buffer1D<float>( 400, 400, 400 ) ) );
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( 4, 8, 2 ),
                                         core::make_buffer1D<float>( 0, 0, 0 ),
                                         core::make_buffer1D<float>( 60, 60, 60 ) ) );

      RbfTransform tfmRbf( affineTfm, rbfs );
      imaging::TransformationDenseDeformableField ddf = imaging::TransformationDenseDeformableField::create( tfmRbf, pstTarget, sizeMm, ddfSize );
      
      // now get the orverlay and export it
      imaging::OverlayGrid overlayGrid;
      imaging::OverlayPrinterGradient gradientPrinter;
      imaging::OverlayPrinterDdf ddfPrinter;

      
      // 
      /*
      imaging::Slice<ui8> slice( core::vector3ui( target.size()[ 0 ], target.size()[ 1 ], 3 ),
                                 core::vector3f( 1, 0, 0 ),
                                 core::vector3f( 0, 1, 0 ),
                                 core::vector3f( 128 + 0.1, 64, 20 ),
                                 core::vector2f( 0.25 / 2, 0.25 / 2 ) );
      */
      /*
      imaging::Slice<ui8> slice( core::vector3ui( target.size()[ 0 ], target.size()[ 1 ], 3 ),
                                 core::vector3f( 1, 0, 0 ),
                                 core::vector3f( 0, 1, 0 ),
                                 core::vector3f( 128 - 32, 64 - 32, 20 ),
                                 core::vector2f( 0.25, 0.25 ) );
        */                       
      // zoomed version on the RBG
      /*
      imaging::Slice<ui8> slice( core::vector3ui( target.size()[ 0 ], target.size()[ 1 ], 3 ),
                                 core::vector3f( 1, 0, 0 ),
                                 core::vector3f( 0, 1, 0 ),
                                 core::vector3f( 128 - 16, 64 - 16, 20 ),
                                 core::vector2f( 0.25 / 2, 0.25 / 2 ) );
        */               
                                 
               
      imaging::Slice<ui8> slice( core::vector3ui( target.size()[ 0 ], target.size()[ 1 ], 3 ),
                                 core::vector3f( 1, 0, 0 ),
                                 core::vector3f( 0, 1, 0 ),
                                 core::vector3f( 0, 0, 20 ),
                                 core::vector2f( 1, 1 ) );
      imaging::Slice<ui8> sliceGradient; sliceGradient.import( slice );
      imaging::Slice<ui8> sliceDdf; sliceDdf.import( slice );
                                 
      std::fill( slice.getStorage().begin(), slice.getStorage().end(), 127 );
      std::fill( sliceGradient.getStorage().begin(), sliceGradient.getStorage().end(), 127 );
      std::fill( sliceDdf.getStorage().begin(), sliceDdf.getStorage().end(), 127 );
                             
                                 
      core::vector3uc color( 255, 255, 255 );
      core::Timer t1;
      overlayGrid.getSlice( slice, color.getBuf(), ddf, core::vector2ui( 20, 20 ) );
      gradientPrinter.getSlice( sliceGradient, color, ddf, core::vector2ui( 16, 16 ) );
      ddfPrinter.getSlice( sliceDdf, ddf, core::vector2ui( 16, 16 ) );
      std::cout << "TIMER=" << t1.getCurrentTime() << std::endl;

      // print the RBF[0] center
      core::vector2f v = slice.worldToSliceCoordinate( core::vector3f( 128, 64, 20 ) );
      ui8* p = slice.getStorage().point( v[ 0 ],
                                         v[ 1 ] );
      p[ 0 ] = 255;
      p[ 1 ] = 0;
      p[ 2 ] = 0;

      core::writeBmp( slice.getStorage(), NLL_TEST_PATH "data/ddfTfmOverlay.bmp" );
      core::writeBmp( sliceGradient.getStorage(), NLL_TEST_PATH "data/ddfTfmOverlayGradient.bmp" );
      core::writeBmp( sliceDdf.getStorage(), NLL_TEST_PATH "data/ddfTfmOverlayDdf.bmp" );

      // now use regression testing...
      core::Image<ui8> sliceRef( NLL_TEST_PATH "data/ddf/ddfTfmOverlay.bmp" );
      core::Image<ui8> sliceGradientRef( NLL_TEST_PATH "data/ddf/ddfTfmOverlayGradient.bmp" );
      core::Image<ui8> sliceDdfRef( NLL_TEST_PATH "data/ddf/ddfTfmOverlayDdf.bmp" );

      TESTER_ASSERT( sliceRef == slice.getStorage() );
      TESTER_ASSERT( sliceGradientRef == sliceGradient.getStorage() );
      TESTER_ASSERT( sliceDdfRef == sliceDdf.getStorage() );
   }

   Slicec getRgbSlice( const imaging::LookUpTransformWindowingRGB& lut, const Slicef& slice )
   {
      core::vector3ui sizeRgb( slice.size()[ 0 ], slice.size()[ 1 ], 3 );
      Slicec sliceRgb( sizeRgb, slice.getAxisX(), slice.getAxisY(), slice.getOrigin(), slice.getSpacing() );
      std::vector< imaging::BlendSliceInfof<imaging::LookUpTransformWindowingRGB> > sliceInfos;
      sliceInfos.push_back( imaging::BlendSliceInfof<imaging::LookUpTransformWindowingRGB>( slice, 0.9, lut ) );
      imaging::blendDummy( sliceInfos, sliceRgb );
      return sliceRgb;
   }



   void testDdfMpr()
   {
      Volume target;
      const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/pet-NAC.mf2", target );
      ensure( loaded, "Error!" );

      const core::vector3f targetCenter = target.indexToPosition( core::vector3f( target.size()[ 0 ] / 2, target.size()[ 1 ] / 2, target.size()[ 2 ] / 2 ) );

      imaging::LookUpTransformWindowingRGB lut( 500, 10000, 255 );
      lut.createGreyscale();

      Mpr mpr( target );
      Slicef slice( core::vector3ui( 256, 256, 1 ), core::vector3f( 1, 0, 0 ), core::vector3f( 0, 1, 0 ), targetCenter, core::vector2f( 1, 1 ) );
      mpr.getSlice( slice );

      core::vector3ui sizeRgb;
      Slicec sliceRgb = getRgbSlice( lut, slice );
      core::writeBmp( sliceRgb.getStorage(), "c:/tmp2/mpr.bmp" );
   }

   void testRandomDdfMapping()
   {
      std::cout << "testing DDF mapping: ";
      for ( size_t iter = 0; iter < 30; ++iter )
      {
         std::cout << "#";

         // setup the volumes
         Matrix pstTarget = core::createTransformationAffine3D(core::vector3f( core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1) ),
                                                               core::vector3f( core::generateUniformDistributionf( -0.1, 0.1), 0, 0 ),
                                                               core::vector3f( 0, 0, 0 ),
                                                               core::vector3f( core::generateUniformDistributionf( 0.9, 1.1), 1, 1 ) );
         Matrix affineTfm = core::createTransformationAffine3D(core::vector3f(core::generateUniformDistributionf( -3, 3), core::generateUniformDistributionf( -3, 3), -1.1 ),
                                                               core::vector3f( 0, 0, core::generateUniformDistributionf( -0.2, 0.2) ),
                                                               core::vector3f( 0, 0, 0 ),
                                                               core::vector3f( 1, core::generateUniformDistributionf( 0.9, 1.1), core::generateUniformDistributionf( 1, 1.1) ) );
         Matrix pstResampled = core::createTransformationAffine3D(core::vector3f( core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1) ),
                                                               core::vector3f( 0, core::generateUniformDistributionf( -0.1, 0.1), 0 ),
                                                               core::vector3f( 0, 0, 0 ),
                                                               core::vector3f( core::generateUniformDistributionf( 0.9, 1.1), 1, 1 ) );
         Volume target( core::vector3ui( 63, 70, 76 ), pstTarget );

         for ( Volume::iterator it = target.begin(); it != target.end(); ++it )
         {
            *it = core::generateUniformDistribution( 100, 500 );
         }      
         test::VolumeUtils::Average( target );   // we need to have a smooth volume, else the interpolation error will be big as it is only bilinear interpolation

         Volume resampled( core::vector3ui( 80, 62, 68 ), pstResampled );

         // setup the ddf
         core::vector3ui ddfSize( 33, 34, 35 );
         core::vector3f sizeMm( target.size()[ 0 ] * target.getSpacing()[ 0 ],
                                target.size()[ 1 ] * target.getSpacing()[ 1 ],
                                target.size()[ 2 ] * target.getSpacing()[ 2 ] );

         test::VolumeUtils::Rbfs rbfs;
         rbfs.push_back( test::VolumeUtils::RbfTransform::Rbf( core::make_buffer1D<float>( core::generateUniformDistributionf( -15, 15), core::generateUniformDistributionf( -15, 15), core::generateUniformDistributionf( -15, 15) ),
                                            core::make_buffer1D<float>( core::generateUniformDistributionf( 16, 48 ), 32, 32 ),
                                            core::make_buffer1D<float>( 410, 430, 380 ) ) );
         test::VolumeUtils::Ddf tfm = test::VolumeUtils::createDdf( affineTfm, pstTarget, rbfs, ddfSize, sizeMm );

         imaging::VolumeTransformationProcessorResampler<Volume, Interpolator> resampler( target, resampled );
         imaging::VolumeTransformationMapperDdf mapper;
         mapper.run( resampler, target, tfm, resampled );

         Interpolator interpolator( target );
         interpolator.startInterpolation();
         float meanError = 0;
         size_t nbCases = 0;
         for ( size_t n = 0; n < 500; ++n )
         {
            static const int border = 5;
            core::vector3f indexInResampled( ( border + rand() ) % ( resampled.size()[ 0 ] - 2 * border ),
                                             ( border + rand() ) % ( resampled.size()[ 1 ] - 2 * border ),
                                             ( border + rand() ) % ( resampled.size()[ 2 ] - 2 * border ) );
            const core::vector3f pointInMm = resampled.indexToPosition( indexInResampled );
            const core::vector3f pointInMmTfm = tfm.transform( pointInMm );
            const core::vector3f indexInTarget = target.positionToIndex( pointInMmTfm );

            if ( indexInTarget[ 0 ] < border || indexInTarget[ 1 ] < border || indexInTarget[ 2 ] < border ||
                  indexInTarget[ 0 ] + border >= resampled.size()[ 0 ] || indexInTarget[ 1 ] + border >= resampled.size()[ 1 ] || indexInTarget[ 2 ] + border >= resampled.size()[ 2 ] )
            {
               // on the border, the interpolator is not accurate, so avoid the comparison...
               continue;
            }

            ++nbCases;

            NLL_ALIGN_16 const float buf4[] = { indexInTarget[ 0 ], indexInTarget[ 1 ], indexInTarget[ 2 ], 0 };
            const float expectedValue = interpolator( buf4 );
            const float valueFound = resampled( indexInResampled[ 0 ], indexInResampled[ 1 ], indexInResampled[ 2 ] );
            meanError += fabs( expectedValue - valueFound );

         /*
            std::cout << "indexInTarget=" << indexInTarget;
            std::cout << "position=" << indexInResampled;
            std::cout << "expectedValue=" << expectedValue << std::endl;
            std::cout << "valueFound=" << valueFound << std::endl;
         */
            
            TESTER_ASSERT( core::equal<float>( valueFound, expectedValue, 1 ) );
         }
         interpolator.endInterpolation();

         meanError /= nbCases;
         std::cout << "meanError=" << meanError << std::endl;
         TESTER_ASSERT( meanError < 0.1 );
      }

      std::cout << "testing DDF mapping: Success! ";
   }

   void testRandomMprDdf()
   {
      srand( 1 );
      std::cout << "Test MPR DDF: ";
      for ( size_t nn = 0; nn < 20; ++nn )
      {
         std::cout << "#";

         // setup the volume
         
         Matrix pstTarget = core::createTransformationAffine3D(core::vector3f( core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1) ),
                                                               core::vector3f( core::generateUniformDistributionf( -0.1, 0.1), 0, 0 ),
                                                               core::vector3f( 0, 0, 0 ),
                                                               core::vector3f( core::generateUniformDistributionf( 0.9, 1.1), 1, 1 ) );
         Matrix affineTfm = core::createTransformationAffine3D(core::vector3f(core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1), -1.1 ),
                                                               core::vector3f( 0, 0, core::generateUniformDistributionf( -0.1, 0.1) ),
                                                               core::vector3f( 0, 0, 0 ),
                                                               core::vector3f( 1, core::generateUniformDistributionf( 0.9, 1.1), core::generateUniformDistributionf( 1, 1.1) ) );
         Volume target( core::vector3ui( 64, 67, 70 ), pstTarget );

         // setup the ddf
         core::vector3ui ddfSize( 30, 32, 36 );
         core::vector3f sizeMm( target.size()[ 0 ] * target.getSpacing()[ 0 ],
                                target.size()[ 1 ] * target.getSpacing()[ 1 ],
                                target.size()[ 2 ] * target.getSpacing()[ 2 ] );

         test::VolumeUtils::Rbfs rbfs;
         rbfs.push_back( test::VolumeUtils::RbfTransform::Rbf( core::make_buffer1D<float>( core::generateUniformDistributionf( -15, 15), core::generateUniformDistributionf( -15, 15), core::generateUniformDistributionf( -15, 15) ),
                                            core::make_buffer1D<float>( core::generateUniformDistributionf( 16, 48 ), 32, 32 ),
                                            core::make_buffer1D<float>( 410, 430, 380 ) ) );
         test::VolumeUtils::Ddf tfm = test::VolumeUtils::createDdf( affineTfm, pstTarget, rbfs, ddfSize, sizeMm );

         for ( Volume::iterator it = target.begin(); it != target.end(); ++it )
         {
            *it = core::generateUniformDistribution( 100, 500 );
         }      
         test::VolumeUtils::Average( target );   // we need to have a smooth volume, else the interpolation error will be big as it is only bilinear interpolation

         // setup the MPR
         Slicef slice( core::vector3ui( 64, 68, 1 ),
                       core::vector3f( 1, core::generateUniformDistributionf( 0, 0.1 ), core::generateUniformDistributionf( 0, 0.1 ) ),
                       core::vector3f( core::generateUniformDistributionf( 0, 0.1 ), 1, core::generateUniformDistributionf( 0, 0.1 ) ),
                       core::vector3f( 0, 1, core::generateUniformDistribution( 20, 50 ) ),
                       core::vector2f( core::generateUniformDistributionf( 0.9, 1.1), core::generateUniformDistributionf( 0.9, 1.1) ) );
         imaging::Mpr<Volume, Interpolator> mpr( target );
         mpr.getSlice( slice, tfm, false );

         // now compare
         Interpolator interpolator( target );
         interpolator.startInterpolation();
         float meanError = 0;
         size_t nbCases = 0;
         for ( size_t n = 0; n < 500; ++n )
         {
            static const int border = 8;
            core::vector2f indexInResampled( ( border + rand() ) % ( slice.size()[ 0 ] - 2 * border ),
                                             ( border + rand() ) % ( slice.size()[ 1 ] - 2 * border ) );
            const core::vector3f pointInMm = slice.sliceToWorldCoordinate( indexInResampled );
            const core::vector3f pointInMmTfm = tfm.transform( pointInMm );
            const core::vector3f indexInTarget = target.positionToIndex( pointInMmTfm );

            if ( indexInTarget[ 0 ] < border || indexInTarget[ 1 ] < border || indexInTarget[ 2 ] < border ||
                 indexInTarget[ 0 ] + border >= target.size()[ 0 ] || indexInTarget[ 1 ] + border >= target.size()[ 1 ] || indexInTarget[ 2 ] + border >= target.size()[ 2 ] )
            {
               // on the border, the interpolator is not accurate, so avoid the comparison...
               continue;
            }

            ++nbCases;

            NLL_ALIGN_16 const float buf4[] = { indexInTarget[ 0 ], indexInTarget[ 1 ], indexInTarget[ 2 ], 0 };
            const float valueFound = slice( indexInResampled[ 0 ], indexInResampled[ 1 ], 0 );
            const float expectedValue = interpolator( buf4 );
            meanError += fabs( expectedValue - valueFound );

            TESTER_ASSERT( core::equal<float>( valueFound, expectedValue, 0.1 ) );
         }
         interpolator.endInterpolation();

         meanError /= nbCases;
         std::cout << "meanError=" << meanError << std::endl;
         TESTER_ASSERT( meanError < 0.01 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformationMapperDdf3D);
TESTER_TEST(testSimpleAffineMappingOnly);
TESTER_TEST(testDdfConversionFromRbf);
TESTER_TEST(testGridOverlay);
TESTER_TEST(testDdfMpr);
TESTER_TEST(testRandomDdfMapping);
TESTER_TEST(testRandomMprDdf);
TESTER_TEST_SUITE_END();
#endif