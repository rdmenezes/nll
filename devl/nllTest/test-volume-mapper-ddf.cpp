#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace imaging
{
   /**
    @brief Dense deformable field grid overlay

	This helper class is used to visualize the DDF deformations. We are basically drawing a regular grid in target space and find the corresponding points
	in source space
    */
   class OverlayGrid
   {
   public:
      /**
       @param slice the slice in source space
       @param ddf the DDF to visualize

       @note The slice is defined in source space, transform it using the affine transformation so we have the bounds, then compute a regular grid within the bounds.
	          For each point of the grid, find its inverse, which will be in source space. Finally connect the points.

            To draw the grid, we actully do it in 2 passes:
            - all vertical lines
            - then all horizontal ones
       */
      template <class T>
      void getSlice( Slice<T>& slice,  T* gridColor, const TransformationDenseDeformableField& ddf, const core::vector2ui gridSize = core::vector2ui( 8, 8 ) )
      {
         ensure( slice.size()[ 0 ] > 0 && slice.size()[ 1 ] > 0, "must not be empty" );

         // grid in MM in source
         const core::vector3f bottomLeft  = slice.sliceToWorldCoordinate( core::vector2f( 0,                      0 ) );
         const core::vector3f bottomRight = slice.sliceToWorldCoordinate( core::vector2f( slice.size()[ 0 ] - 1,  0 ) );
         const core::vector3f topLeft     = slice.sliceToWorldCoordinate( core::vector2f( 0,                      slice.size()[ 1 ] - 1 ) );
         const core::vector3f topRight    = slice.sliceToWorldCoordinate( core::vector2f( slice.size()[ 0 ] - 1,  slice.size()[ 1 ] - 1 ) );

         // grid in MM in target
         const core::vector3f bottomLeftTarget  = core::transf4( ddf.getAffineMatrix(), bottomLeft );
         const core::vector3f bottomRightTarget = core::transf4( ddf.getAffineMatrix(), bottomRight );
         const core::vector3f topLeftTarget     = core::transf4( ddf.getAffineMatrix(), topLeft );
         const core::vector3f topRightTarget    = core::transf4( ddf.getAffineMatrix(), topRight );

         // vector director in target
         const core::vector3f dx = ( bottomRightTarget - bottomLeftTarget ) / gridSize[ 0 ];
         const core::vector3f dy = ( topLeftTarget     - bottomLeftTarget ) / gridSize[ 1 ];

         // draw the vertical and horizontal lines
         for ( ui32 x = 0; x < gridSize[ 0 ]; ++x )
         {
            const core::vector3f ddx = dx * x;
            _drawLine( slice, gridColor, ddf, bottomLeft + ddx, topLeft + ddx, dy, gridSize[ 1 ] );
         }

         for ( ui32 y = 0; y < gridSize[ 1 ]; ++y )
         {
            const core::vector3f ddy = dy * y;
            _drawLine( slice, gridColor, ddf, bottomLeft + ddy, bottomRight + ddy, dx, gridSize[ 0 ] );
         }
      }

   private:
      template <class T>
      static void _drawLine( Slice<T>& slice,
                             T* gridColor,
                             const TransformationDenseDeformableField& ddf,
                             const core::vector3f& first,
                             const core::vector3f& last,
                             const core::vector3f& direction,
                             ui32 nbSteps )
      {
         bool converged = false;
         core::vector3f previousMm = ddf.getInverseTransform( first, 1000, &converged );
         if ( !converged )
         {
            core::LoggerNll::write( core::LoggerNll::ERROR, "OverlayGrid::getSlice::_drawLine: ddf::getInverseTransform failed!" );
            return;
         }

         const core::vector2f previousIndexf = slice.worldToSliceCoordinate( slice.getOrthogonalProjection( previousMm ) );
         core::vector2i previousIndex( core::round( previousIndexf[ 0 ] ), core::round( previousIndexf[ 1 ] ) );
         if ( previousIndex[ 0 ] < 0 || previousIndex[ 1 ] < 0 || previousIndex[ 0 ] >= slice.size()[ 0 ] || previousIndex[ 1 ] >= slice.size()[ 1 ] )
         {
            // TODO: change this: we know the first should always be inside!!
            return;
         }

         for ( ui32 step = 0; step < nbSteps; ++step )
         {
            const core::vector3f point = first + direction * step;      // point in target
            core::vector3f currentMm = ddf.getInverseTransform( point, 1000, &converged );  // corresponding point in source
            if ( !converged )
            {
               core::LoggerNll::write( core::LoggerNll::ERROR, "OverlayGrid::getSlice::_drawLine: ddf::getInverseTransform failed!" );
               return;
            }

            // we need to project the point in 3D due to the DDF displacement on the slice
            const core::vector2f currentIndexf = slice.worldToSliceCoordinate( slice.getOrthogonalProjection( currentMm ) );
            const core::vector2i currentIndex( core::round( currentIndexf[ 0 ] ), core::round( currentIndexf[ 1 ] ) );
            if ( currentIndex[ 0 ] >= 0 && currentIndex[ 1 ] >= 0 && currentIndex[ 0 ] < slice.size()[ 0 ] && currentIndex[ 1 ] < slice.size()[ 1 ] )
            {
               core::bresham( slice.getStorage(), previousIndex, currentIndex, gridColor );
            } else {
               // point is not contained within the slice
               // TODO: we need to intercept at the last position within the slice
               return;
            }

            previousIndex = currentIndex;
         }
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
            TESTER_ASSERT( err < 1.5e-1 );

            bool converged = false;
            const core::vector3f backward = ddf.getInverseTransform( forward, 1000, &converged );
            ensure( converged, "didn't converged!!!" );
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
      core::vector3ui ddfSize( 32, 32, 32 );
      core::vector3f sizeMm( target.size()[ 0 ] * target.getSpacing()[ 0 ],
                             target.size()[ 1 ] * target.getSpacing()[ 0 ],
                             target.size()[ 2 ] * target.getSpacing()[ 0 ] );

      typedef core::DeformableTransformationRadialBasis<> RbfTransform;

      std::vector<RbfTransform::Rbf> rbfs;
      
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( -5, -30, -3 ),
                                         core::make_buffer1D<float>( 128, 64, 0 ),
                                         core::make_buffer1D<float>( 120, 120, 120 ) ) );
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( 4, 8, 2 ),
                                         core::make_buffer1D<float>( 0, 0, 0 ),
                                         core::make_buffer1D<float>( 60, 60, 60 ) ) );
      
      rbfs.push_back( RbfTransform::Rbf( core::make_buffer1D<float>( 0, 0, 0 ),
                                         core::make_buffer1D<float>( 0, 0, 0 ),
                                         core::make_buffer1D<float>( 60, 60, 60 ) ) );
      RbfTransform tfmRbf( affineTfm, rbfs );
      imaging::TransformationDenseDeformableField ddf = imaging::TransformationDenseDeformableField::create( tfmRbf, pstTarget, sizeMm, ddfSize );

      std::cout << ddf.getStorage()( 16, 8, 0 );
      std::cout << ddf.getStorage()( 17, 8, 0 );
      std::cout << ddf.getStorage()( 15, 8, 0 );

      std::cout << ddf.getStorage()( 16, 8, 0 );
      std::cout << ddf.getStorage()( 16, 7, 0 );
      std::cout << ddf.getStorage()( 16, 9, 0 );

      std::cout << ddf.getInverseTransform( core::vector3f( 127.5, 63.75, 0 ) );

      // now get the orverlay and export it
      imaging::OverlayGrid overlayGrid;

      imaging::Slice<ui8> slice( core::vector3ui( target.size()[ 0 ], target.size()[ 1 ], 3 ),
                                 core::vector3f( 1, 0, 0 ),
                                 core::vector3f( 0, 1, 0 ),
                                 target.getOrigin(),
                                 core::vector2f( 1, 1 ) );
      core::vector3uc color( 255, 0, 0 );
      overlayGrid.getSlice( slice, color.getBuf(), ddf, core::vector2ui( 50, 50 ) );

      core::writeBmp( slice.getStorage(), "c:/tmp/ddfOverlay.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformationMapperDdf3D);

TESTER_TEST(testSimpleAffineMappingOnly);
TESTER_TEST(testDdfConversionFromRbf);

//TESTER_TEST(testGridOverlay);
TESTER_TEST_SUITE_END();
#endif