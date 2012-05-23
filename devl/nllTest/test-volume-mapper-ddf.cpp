#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace imaging
{
   /**
    @brief Dense deformable field grid overlay
    */
   class OverlayGrid
   {
   public:
      /**
       @param slice the slice in source space
       @param ddf the DDF to visualize

       @note Internally, we construct the overlay
       */
      template <class T>
      void getSlice( Slice<T>& slice,  T* gridColor, const TransformationDenseDeformableField& ddf, const gridSize = core::vector2ui( 32, 32 ) )
      {
         for ( ui32 y = 0; y < gridSize[ 1 ]; ++y )
         {
            // start a new line from scratch
            core::vector3f startLineMm = slice.getOrigin() + slice.getAxisY() * y;
            for ( ui32 x = 0; x < gridSize[ 0 ]; ++x )
            {

               startLineMm += slice.getAxisX();
            }
         }
         //core::vector3f start 
      }
   };
}
}


class TestTransformationMapperDdf3D
{
   typedef core::Matrix<float>            Matrix;
   typedef imaging::VolumeSpatial<float>  Volume;
   typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;

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
         *it = core::generateUniformDistribution( 100, 500 );
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
      core::vector3ui ddfSize( 130, 135, 100 );
      core::vector3f sizeMm( target.size()[ 0 ] * target.getSpacing()[ 0 ],
                             target.size()[ 1 ] * target.getSpacing()[ 0 ],
                             target.size()[ 2 ] * target.getSpacing()[ 0 ] );

      typedef core::DeformableTransformationRadialBasis<> RbfTransform;

      std::vector<RbfTransform::Rbf> rbfs;
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( -5, -7, -3 ),
                                         core::make_buffer1D<float>( 128, 64, 32 ),
                                         core::make_buffer1D<float>( 30, 30, 30 ) ) );
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( 4, 8, 2 ),
                                         core::make_buffer1D<float>( 0, 0, 0 ),
                                         core::make_buffer1D<float>( 60, 60, 60 ) ) );
      
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
      for ( ui32 n = 0; n < 50000; ++n )
      {
         const core::vector3f p( core::generateUniformDistribution( -100, 2560 - 100 ) / 10,
                                 core::generateUniformDistribution( -200, 1280 - 200 ) / 10,
                                 core::generateUniformDistribution( -300, 640 - 300 ) / 10 );
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
            TESTER_ASSERT( err < 1e-1 );

            bool converged = false;
            const core::vector3f backward = ddf.getInverseTransform( forward, 1000, &converged );
            ensure( converged, "oups!" );
            const double err2 = (backward - p).norm2();
            if ( err2 >= 1.1e-1 )
            {
               std::cout << "error=" << err2 << std::endl;
               std::cout << backward  << p;
            }
            TESTER_ASSERT( err2 <= 1.1e-1 );
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformationMapperDdf3D);
TESTER_TEST(testSimpleAffineMappingOnly);
TESTER_TEST(testDdfConversionFromRbf);
TESTER_TEST_SUITE_END();
#endif