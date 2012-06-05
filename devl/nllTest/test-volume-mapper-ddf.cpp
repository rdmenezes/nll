#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace imaging
{
   class OverlayPrinterDdf
   {
   public:
      void getSlice( Slice<ui8>& slice, const core::vector3uc& color, const TransformationDenseDeformableField& ddf, const core::vector2ui gridSize = core::vector2ui( 16, 16 ) )
      {
         ensure( slice.size()[ 2 ] == 3, "must be a rgb slice" );
         typedef TransformationDenseDeformableField::Matrix Matrix;

         // grid in MM in source
         const core::vector3f bottomLeft  = slice.sliceToWorldCoordinate( core::vector2f( 0,                             0 ) );
         const core::vector3f bottomRight = slice.sliceToWorldCoordinate( core::vector2f( (float)slice.size()[ 0 ] - 1,  0 ) );
         const core::vector3f topLeft     = slice.sliceToWorldCoordinate( core::vector2f( 0,                             (float)slice.size()[ 1 ] - 1 ) );
         const core::vector3f topRight    = slice.sliceToWorldCoordinate( core::vector2f( (float)slice.size()[ 0 ] - 1,  (float)slice.size()[ 1 ] - 1 ) );

         // grid in MM in target
         const core::vector3f bottomLeftTarget  = core::transf4( ddf.getAffineMatrix(), bottomLeft );
         const core::vector3f bottomRightTarget = core::transf4( ddf.getAffineMatrix(), bottomRight );
         const core::vector3f topLeftTarget     = core::transf4( ddf.getAffineMatrix(), topLeft );
         const core::vector3f topRightTarget    = core::transf4( ddf.getAffineMatrix(), topRight );

         // vector director in target
         const core::vector3f dx = ( bottomRightTarget - bottomLeftTarget ) / (float)gridSize[ 0 ];
         const core::vector3f dy = ( topLeftTarget     - bottomLeftTarget ) / (float)gridSize[ 1 ];

         const float dxIndex = static_cast<float>( slice.size()[ 0 ] ) / gridSize[ 0 ];
         const float dyIndex = static_cast<float>( slice.size()[ 1 ] ) / gridSize[ 1 ];

         core::GeometryPlane plane( core::vector3f( 0, 0, 0 ), slice.getAxisX() * slice.getSpacing()[ 0 ], slice.getAxisY() * slice.getSpacing()[ 1 ] ); // set up a plane a (0, 0, 0) with the same orientation as the slice
         for ( ui32 y = 0; y < gridSize[ 1 ]; ++y )
         {
            const int yi = static_cast<int>( dyIndex * y );
            const core::vector3f dyMm = dy * (float)y + bottomLeftTarget;
            for ( ui32 x = 0; x < gridSize[ 0 ]; ++x )
            {
               const int displayRand = 0;
               core::vector3uc color( rand() % 255, rand() % 255, rand() % 255 );
               const int xi = static_cast<int>( dxIndex * x );
               const core::vector3f dxMm = dyMm + dx * (float)x;

               // get the DDF deformation
               const core::vector3f indexInDdf = core::transf4( ddf.getStorage().getInvertedPst(), dxMm );
               const core::vector3f gradv = ddf.transformDeformableOnlyIndex( indexInDdf );
                                           
               // then project it on the plane to have a 2D vector
               const core::vector3f gradvProj = plane.getOrthogonalProjection( gradv );         // get a 3D point in MM on the plane
               const core::vector2f gradvProj2d = plane.worldToPlaneCoordinate( gradvProj );    // get a 2D index in the slice space

               // finally draw the vector
               const core::vector2i start( xi + displayRand, yi );
               const core::vector2i end( core::round( start[ 0 ] + gradvProj2d[ 0 ] ) + displayRand,
                                         core::round( start[ 1 ] + gradvProj2d[ 1 ] ) );
               core::bresham( slice.getStorage(), start, end, color );

               if ( start[ 0 ] >= 0 && start[ 0 ] < (int)slice.size()[ 0 ] &&
                    start[ 1 ] >= 0 && start[ 1 ] < (int)slice.size()[ 1 ] )
               {
                  ui8* p = slice.getStorage().point( start[ 0 ], start[ 1 ] );
                  p[ 0 ] = 0;
                  p[ 1 ] = 0;
                  p[ 2 ] = 255;
               }
            }
         }
      }
   };

   class OverlayPrinterGradient
   {
   public:
      void getSlice( Slice<ui8>& slice, const core::vector3uc& color, const TransformationDenseDeformableField& ddf, const core::vector2ui gridSize = core::vector2ui( 16, 16 ) )
      {
         ensure( slice.size()[ 2 ] == 3, "must be a rgb slice" );
         typedef TransformationDenseDeformableField::Matrix Matrix;

         // grid in MM in source
         const core::vector3f bottomLeft  = slice.sliceToWorldCoordinate( core::vector2f( 0,                             0 ) );
         const core::vector3f bottomRight = slice.sliceToWorldCoordinate( core::vector2f( (float)slice.size()[ 0 ] - 1,  0 ) );
         const core::vector3f topLeft     = slice.sliceToWorldCoordinate( core::vector2f( 0,                             (float)slice.size()[ 1 ] - 1 ) );
         const core::vector3f topRight    = slice.sliceToWorldCoordinate( core::vector2f( (float)slice.size()[ 0 ] - 1,  (float)slice.size()[ 1 ] - 1 ) );

         // vector director in source
         const core::vector3f dx = ( bottomRight - bottomLeft ) / (float)gridSize[ 0 ];
         const core::vector3f dy = ( topLeft     - bottomLeft ) / (float)gridSize[ 1 ];

         const float dxIndex = static_cast<float>( slice.size()[ 0 ] ) / (float)gridSize[ 0 ];
         const float dyIndex = static_cast<float>( slice.size()[ 1 ] ) / (float)gridSize[ 1 ];

         core::GeometryPlane plane( core::vector3f( 0, 0, 0 ), slice.getAxisX() * slice.getSpacing()[ 0 ], slice.getAxisY() * slice.getSpacing()[ 1 ] ); // set up a plane a (0, 0, 0) with the same orientation as the slice
         Matrix id( 3, 1 );
         id[ 0 ] = 1;
         id[ 1 ] = 1;
         id[ 2 ] = 1;
         for ( ui32 y = 0; y < gridSize[ 1 ]; ++y )
         {
            const int yi = static_cast<int>( dyIndex * y );
            const core::vector3f dyMm = dy * (float)y + slice.getOrigin();
            for ( ui32 x = 0; x < gridSize[ 0 ]; ++x )
            {
               const int xi = static_cast<int>( dxIndex * x );
               const core::vector3f dxMm = dyMm + dx * (float)x;

               // get the 3D gradient vector
               const Matrix grad = ddf.getGradient( dxMm );
               const core::vector3f gradv = core::vector3f( (grad * id).getBuf() );
                                           
               // then project it on the plane to have a 2D vector
               const core::vector3f gradvProj = plane.getOrthogonalProjection( gradv );         // get a 3D point in MM on the plane
               const core::vector2f gradvProj2d = plane.worldToPlaneCoordinate( gradvProj );    // get a 2D index in the slice space

               // finally draw the vector
               const core::vector2i start( xi, yi );
               const core::vector2i end( core::round( start[ 0 ] + gradvProj2d[ 0 ] ),
                                         core::round( start[ 1 ] + gradvProj2d[ 1 ] ) );
               core::bresham( slice.getStorage(), start, end, color );

               ui8* p = slice.getStorage().point( start[ 0 ], start[ 1 ] );
               p[ 0 ] = 0;
               p[ 1 ] = 0;
               p[ 2 ] = 255;
            }
         }
      }
   };

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

            To draw the grid, we actually do it in 2 passes:
            - all vertical lines
            - then all horizontal ones
       */
      template <class T>
      void getSlice( Slice<T>& slice,  T* gridColor, const TransformationDenseDeformableField& ddf, const core::vector2ui gridSize = core::vector2ui( 8, 8 ) )
      {
         ensure( slice.size()[ 0 ] > 0 && slice.size()[ 1 ] > 0, "must not be empty" );

         // grid in MM in source
         const core::vector3f bottomLeft  = slice.sliceToWorldCoordinate( core::vector2f( 0,                             0 ) );
         const core::vector3f bottomRight = slice.sliceToWorldCoordinate( core::vector2f( (float)slice.size()[ 0 ] - 1,  0 ) );
         const core::vector3f topLeft     = slice.sliceToWorldCoordinate( core::vector2f( 0,                             (float)slice.size()[ 1 ] - 1 ) );
         const core::vector3f topRight    = slice.sliceToWorldCoordinate( core::vector2f( (float)slice.size()[ 0 ] - 1,  (float)slice.size()[ 1 ] - 1 ) );

         // grid in MM in target
         core::vector3f bottomLeftTarget  = core::transf4( ddf.getAffineMatrix(), bottomLeft );
         core::vector3f bottomRightTarget = core::transf4( ddf.getAffineMatrix(), bottomRight );
         core::vector3f topLeftTarget     = core::transf4( ddf.getAffineMatrix(), topLeft );
         core::vector3f topRightTarget    = core::transf4( ddf.getAffineMatrix(), topRight );

         // now, because of the deformation, the grid may not be fully visible, so we are finding
         // the corners back in source. If these points are extending the grid in source space,
         // then extend the grid. Finally, transform the extended source grid in target space...
         const core::vector3f bottomLeft2  = slice.getOrthogonalProjection( ddf.getInverseTransform( bottomLeftTarget - ddf.transformDeformableOnlyNoTfm( bottomLeftTarget ) ) );
         const core::vector3f bottomRight2 = slice.getOrthogonalProjection( ddf.getInverseTransform( bottomRight - ddf.transformDeformableOnlyNoTfm( bottomRight ) ) );
         const core::vector3f topLeft2     = slice.getOrthogonalProjection( ddf.getInverseTransform( topLeft - ddf.transformDeformableOnlyNoTfm( topLeft ) ) );
         const core::vector3f topRight2    = slice.getOrthogonalProjection( ddf.getInverseTransform( topRight - ddf.transformDeformableOnlyNoTfm( topRight ) ) );

         // now we are in slice index, which is very simple to determine if the grid needs to be extended...
         const core::vector2f bottomLeft2Index  = slice.worldToSliceCoordinate( bottomLeft2 );
         const core::vector2f bottomRight2Index = slice.worldToSliceCoordinate( bottomRight2 );
         const core::vector2f topLeft2Index     = slice.worldToSliceCoordinate( topLeft2 );
         const core::vector2f topRight2Index    = slice.worldToSliceCoordinate( topRight2 );
         

         // default grid range
         core::vector2f min( 0, 0 );
         core::vector2f max( slice.size()[ 0 ], slice.size()[ 1 ] );

         // extend the range if needed
         _minMaxGridIndex( min, max, bottomLeft2Index );
         _minMaxGridIndex( min, max, bottomRight2Index );
         _minMaxGridIndex( min, max, topLeft2Index );
         _minMaxGridIndex( min, max, topRight2Index );

         // add extra padding just to be sure
         min -= core::vector2f( 10, 10 );
         max += core::vector2f( 10, 10 );

         // export the min/max: they will be our grid 
         bottomLeftTarget  = core::transf4( ddf.getAffineMatrix(), slice.sliceToWorldCoordinate( min ) );
         topRightTarget    = core::transf4( ddf.getAffineMatrix(), slice.sliceToWorldCoordinate( max ) );
         bottomRightTarget = core::transf4( ddf.getAffineMatrix(), slice.sliceToWorldCoordinate( core::vector2f( max[ 0 ], min[ 1 ] ) ) );
         topLeftTarget     = core::transf4( ddf.getAffineMatrix(), slice.sliceToWorldCoordinate( core::vector2f( min[ 0 ], max[ 1 ] ) ) );
         
         // vector director in target
         const core::vector3f dx = ( bottomRightTarget - bottomLeftTarget ) / (float)gridSize[ 0 ];
         const core::vector3f dy = ( topLeftTarget     - bottomLeftTarget ) / (float)gridSize[ 1 ];

         // draw the vertical and horizontal lines
         for ( ui32 x = 0; x < gridSize[ 0 ]; ++x )
         {
            const core::vector3f ddx = dx * (float)x;
            _drawLine( slice, gridColor, ddf, bottomLeft + ddx, dy, gridSize[ 1 ] );
         }

         for ( ui32 y = 0; y < gridSize[ 1 ]; ++y )
         {
            const core::vector3f ddy = dy * (float)y;
            _drawLine( slice, gridColor, ddf, bottomLeft + ddy, dx, gridSize[ 0 ] );
         }
      }

   private:
      // given a point, a segment and the current min/max position on this segment, check if we need to extend
      // the grid so that this point is contained within min/max
      void _minMaxGridIndex( core::vector2f& min, core::vector2f& max, const core::vector2f& point )
      {
         min[ 0 ] = std::min( min[ 0 ], point[ 0 ] );
         min[ 1 ] = std::min( min[ 1 ], point[ 1 ] );
         max[ 0 ] = std::max( max[ 0 ], point[ 0 ] );
         max[ 1 ] = std::max( max[ 1 ], point[ 1 ] );
      }

      template <class T>
      static void _drawLine( Slice<T>& slice,
                             T* gridColor,
                             const TransformationDenseDeformableField& ddf,
                             const core::vector3f& first,
                             const core::vector3f& direction,
                             ui32 nbSteps )
      {
         const core::vector2f half( slice.size()[ 0 ] / 2,
                                    slice.size()[ 1 ] / 2 );
         bool converged = false;
         core::vector3f previousMm = ddf.getInverseTransform( first, 100, &converged );
         if ( !converged )
         {
            std::cout << "Error begining!" << std::endl;
            core::LoggerNll::write( core::LoggerNll::ERROR, "OverlayGrid::getSlice::_drawLine: ddf::getInverseTransform failed!" );
         }

         core::GeometryBox2d box( core::vector2f( 0, 0 ),
                                  core::vector2f( slice.size()[ 0 ] - 1,
                                                  slice.size()[ 1 ] - 1 ) );

         core::vector2f previousIndexf = slice.worldToSliceCoordinate( slice.getOrthogonalProjection( previousMm ) ) /* + half */;
         core::vector2i previousIndex( core::round( previousIndexf[ 0 ] ), core::round( previousIndexf[ 1 ] ) );
         bool isPreviousInside = box.contains( core::vector2f( previousIndexf ) );
         for ( ui32 step = 1; step < nbSteps; ++step )
         {
            const core::vector3f point = first + direction * (float)step;      // point in target
            core::vector3f currentMm = ddf.getInverseTransform( point, 100, &converged );
            if ( !converged )
            {
               std::cout << "not converged=" << point;
               core::LoggerNll::write( core::LoggerNll::ERROR, "OverlayGrid::getSlice::_drawLine: ddf::getInverseTransform failed!" );
            }

            // we need to project the point in 3D due to the DDF displacement on the slice
            const core::vector2f currentIndexf = slice.worldToSliceCoordinate( slice.getOrthogonalProjection( currentMm ) ) /* + half*/;
            const core::vector2i currentIndex( core::round( currentIndexf[ 0 ] ), core::round( currentIndexf[ 1 ] ) );
            const bool isCurrentInside = box.contains( currentIndexf );
            
            if ( isCurrentInside && isPreviousInside )
            {
               // both points are inside so simply draw the line
               core::bresham( slice.getStorage(), previousIndex, currentIndex, gridColor );
            } else if ( isCurrentInside && !isPreviousInside )
            {
               core::vector2f intersection;
               box.getIntersection( core::GeometrySegment2d( currentIndexf, previousIndexf ), intersection, 0.1 );
               core::bresham( slice.getStorage(), core::vector2i( intersection[ 0 ], intersection[ 1 ] ), currentIndex, gridColor );
            } else if ( !isCurrentInside && isPreviousInside )
            {
               core::vector2f intersection;
               box.getIntersection( core::GeometrySegment2d( currentIndexf, previousIndexf ), intersection, 0.1 );
               core::bresham( slice.getStorage(),
                              core::vector2i( core::round( intersection[ 0 ] ),
                                              core::round( intersection[ 1 ] ) ),
                              previousIndex, gridColor );
            }

            previousIndex = currentIndex;
            previousIndexf = currentIndexf;
            isPreviousInside = isCurrentInside;
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
      for ( ui32 n = 0; n < 200000; ++n )
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
      ddfPrinter.getSlice( sliceDdf, color, ddf, core::vector2ui( 16, 16 ) );
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
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformationMapperDdf3D);

TESTER_TEST(testSimpleAffineMappingOnly);
TESTER_TEST(testDdfConversionFromRbf);

TESTER_TEST(testGridOverlay);
TESTER_TEST_SUITE_END();
#endif