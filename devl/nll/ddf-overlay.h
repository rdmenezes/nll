/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_IMAGING_DDF_OVERLAY_H_
# define NLL_IMAGING_DDF_OVERLAY_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Prints the DDF deformations on a slice
    */
   class OverlayPrinterDdf
   {
   public:
      void getSlice( Slice<ui8>& slice, const TransformationDenseDeformableField& ddf, const core::vector2ui gridSize = core::vector2ui( 16, 16 ) )
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

   /**
    @ingroup imaging
    @brief Prints the DDF gradient on a slice
    */
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
    @ingroup imaging
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
         core::vector2f max( (float)slice.size()[ 0 ], (float)slice.size()[ 1 ] );

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
         const core::vector2f half( (float)( slice.size()[ 0 ] / 2 ),
                                    (float)( slice.size()[ 1 ] / 2 ) );
         bool converged = false;
         core::vector3f previousMm = ddf.getInverseTransform( first, 100, &converged );
         if ( !converged )
         {
            std::cout << "Error begining!" << std::endl;
            core::LoggerNll::write( core::LoggerNll::ERROR, "OverlayGrid::getSlice::_drawLine: ddf::getInverseTransform failed!" );
         }

         core::GeometryBox2d box( core::vector2f( 0, 0 ),
                                  core::vector2f( (float)slice.size()[ 0 ] - 1,
                                                  (float)slice.size()[ 1 ] - 1 ) );

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
               core::bresham( slice.getStorage(),
                              core::vector2i( core::round( intersection[ 0 ] ), 
                                              core::round( intersection[ 1 ] ) ),
                              currentIndex, gridColor );
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

#endif
