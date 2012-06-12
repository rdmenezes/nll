#include <nll/nll.h>
#include <tester/register.h>
#include "utils.h"

using namespace nll;


class TestVolumeMapper
{
   typedef test::VolumeUtils::Matrix           Matrix;
   typedef test::VolumeUtils::Volume           Volume;
   typedef test::VolumeUtils::Interpolator     Interpolator;
   typedef test::VolumeUtils::Slicef           Slicef;

public:
   void testSimpleMapping()
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

      for ( Volume::iterator it = target.begin(); it != target.end(); ++it )
      {
         *it = core::generateUniformDistribution( 100, 500 );
      }      

      core::Timer timer;
      imaging::VolumeTransformationProcessorResampler<Volume, Interpolator> resampler( target, resampled );
      imaging::VolumeTransformationMapper mapper;
      mapper.run( resampler, target, affineTfm, resampled );
      std::cout << "Time affine resampling 256*512*300=" << timer.getCurrentTime() << std::endl;

      std::cout << resampled( 0, 0, 0 ) << std::endl;
      std::cout << resampled( 8, 0, 0 ) << std::endl;
      std::cout << resampled( 0, 9, 0 ) << std::endl;
      std::cout << resampled( 0, 0, 10 ) << std::endl;
      std::cout << resampled( 12, 30, 40 ) << std::endl;
      std::cout << resampled( 150, 80, 50 ) << std::endl;

      // these values were computed using this library and are for regression tests only (it is assumed correct...)
      {
         const float v = resampled( 0, 0, 0 );
         TESTER_ASSERT( core::equal<float>( v, 221.367, 1e-2 ) );
      }

      {
         const float v = resampled( 8, 0, 0 );
         TESTER_ASSERT( core::equal<float>( v, 267.024, 1e-2 ) );
      }
      {
         const float v = resampled( 0, 9, 0 );
         TESTER_ASSERT( core::equal<float>( v, 266.664, 1e-2 ) );
      }
      {
         const float v = resampled( 0, 0, 10 );
         TESTER_ASSERT( core::equal<float>( v, 256.558, 1e-2 ) );
      }
      {
         const float v = resampled( 12, 30, 40 );
         TESTER_ASSERT( core::equal<float>( v, 336.237, 1e-2 ) );
      }
      {
         const float v = resampled( 150, 80, 50 );
         TESTER_ASSERT( core::equal<float>( v, 212.882, 1e-2 ) );
      }
   }

   void testRandomMapping()
   {
      srand( 6 );
      std::cout << "Test volume mapper: ";
      for ( ui32 nn = 0; nn < 20; ++nn )
      {
         std::cout << "#";
         Matrix pstTarget = core::createTransformationAffine3D(core::vector3f( core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1) ),
                                                               core::vector3f( core::generateUniformDistributionf( -0.1, 0.1), 0, 0 ),
                                                               core::vector3f( 0, 0, 0 ),
                                                               core::vector3f( core::generateUniformDistributionf( 0.9, 1.1), 1, 1 ) );
         Matrix pstResampled = core::createTransformationAffine3D(core::vector3f( core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1) ),
                                                               core::vector3f( 0, core::generateUniformDistributionf( -0.1, 0.1), 0 ),
                                                               core::vector3f( 0, 0, 0 ),
                                                               core::vector3f( core::generateUniformDistributionf( 0.9, 1.1), 1, 1 ) );
         Matrix affineTfm = core::createTransformationAffine3D(core::vector3f(core::generateUniformDistributionf( -1, 1), core::generateUniformDistributionf( -1, 1), -1.1 ),
                                                               core::vector3f( 0, 0, core::generateUniformDistributionf( -0.1, 0.1) ),
                                                               core::vector3f( 0, 0, 0 ),
                                                               core::vector3f( 1, core::generateUniformDistributionf( 0.9, 1.1), core::generateUniformDistributionf( 1, 1.1) ) );

         Volume resampled( core::vector3ui( 80, 85, 75 ), pstResampled );
         Volume target( core::vector3ui( 75, 80, 85 ), pstTarget );

         for ( Volume::iterator it = target.begin(); it != target.end(); ++it )
         {
            *it = core::generateUniformDistribution( 100, 500 );
         }      
         test::VolumeUtils::Average( target );   // we need to have a smooth volume, else the interpolation error will be big as it is only bilinear interpolation

         const core::vector3f pointInMm = resampled.indexToPosition( core::vector3f( 52, 67, 21 ) );
         const core::vector3f pointInMmTfm = core::transf4( affineTfm, pointInMm );
         const core::vector3f indexInTarget = target.positionToIndex( pointInMmTfm );

         core::Timer timer;
         imaging::VolumeTransformationProcessorResampler<Volume, Interpolator> resampler( target, resampled );
         imaging::VolumeTransformationMapper mapper;
         mapper.run( resampler, target, affineTfm, resampled );

         Interpolator interpolator( target );
         interpolator.startInterpolation();
         float meanError = 0;
         ui32 nbCases = 0;

         for ( ui32 n = 0; n < 500; ++n )
         {
            static const int border = 5;
            core::vector3f indexInResampled( ( border + rand() ) % ( resampled.size()[ 0 ] - 2 * border ),
                                             ( border + rand() ) % ( resampled.size()[ 1 ] - 2 * border ),
                                             ( border + rand() ) % ( resampled.size()[ 2 ] - 2 * border ) );
            const core::vector3f pointInMm = resampled.indexToPosition( indexInResampled );
            const core::vector3f pointInMmTfm = core::transf4( affineTfm, pointInMm );
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

            TESTER_ASSERT( core::equal<float>( valueFound, expectedValue, 1 ) );
         }
         interpolator.endInterpolation();

         meanError /= nbCases;
         std::cout << "meanError=" << meanError << std::endl;
         TESTER_ASSERT( meanError < 0.1 );
      }

      std::cout << " done!" << std::endl;
   }

   void testRandomMpr()
   {
      srand( 1 );
      std::cout << "Test MPR: ";
      for ( ui32 nn = 0; nn < 20; ++nn )
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

         Volume target( core::vector3ui( 64, 64, 64 ), pstTarget );

         for ( Volume::iterator it = target.begin(); it != target.end(); ++it )
         {
            *it = core::generateUniformDistribution( 100, 500 );
         }      
         test::VolumeUtils::Average( target );   // we need to have a smooth volume, else the interpolation error will be big as it is only bilinear interpolation

         // setup the MPR
         Slicef slice( core::vector3ui( 64, 64, 1 ),
                       core::vector3f( 1, core::generateUniformDistributionf( 0, 0.1 ), core::generateUniformDistributionf( 0, 0.1 ) ),
                       core::vector3f( core::generateUniformDistributionf( 0, 0.1 ), 1, core::generateUniformDistributionf( 0, 0.1 ) ),
                       core::vector3f( 0, 1, core::generateUniformDistribution( 20, 50 ) ),
                       core::vector2f( core::generateUniformDistributionf( 0.9, 1.1), core::generateUniformDistributionf( 0.9, 1.1) ) );
         imaging::Mpr<Volume, Interpolator> mpr( target );
         mpr.getSlice( slice, imaging::TransformationAffine( affineTfm ), false );

         // now compare
         Interpolator interpolator( target );
         interpolator.startInterpolation();
         float meanError = 0;
         ui32 nbCases = 0;
         for ( ui32 n = 0; n < 500; ++n )
         {
            static const int border = 8;
            core::vector2f indexInResampled( ( border + rand() ) % ( slice.size()[ 0 ] - 2 * border ),
                                             ( border + rand() ) % ( slice.size()[ 1 ] - 2 * border ) );
            const core::vector3f pointInMm = slice.sliceToWorldCoordinate( indexInResampled );
            const core::vector3f pointInMmTfm = core::transf4( affineTfm, pointInMm );
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
         TESTER_ASSERT( meanError < 0.01 );
      }
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeMapper);
TESTER_TEST(testSimpleMapping);
TESTER_TEST(testRandomMapping);
TESTER_TEST(testRandomMpr);
TESTER_TEST_SUITE_END();
#endif