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

#ifndef NLL_ALGORITHM_PYRAMID_HESSIAN_2D_H_
# define NLL_ALGORITHM_PYRAMID_HESSIAN_2D_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Hold a stack of the hessian determinant using a crude approximation of a gaussian in 2D
           for each point, H(x, o) =| Lxx Lxy |
                                    | Lxy Lyy |
                           det(H)= Lxx*Lyy-(0.9 * Lxy)^2
           
    @note the area used to compute Lxx Lxy... are different and must be normalized, e.g., using the frobenius norm L=3

    Conventions: (x, y) refers to coordinates in integral image
                 (xp, yp, map) are in pyramid space

    Use double as the hessian will often be close to the float's epsilon.

    Internally we are discarding scale / 2 i.e., when in pyramid, (0, 0, s) corresponds to a filter that fits totally inside the integral image, e.g., the top left corner
    of the filter is set to (0,0). It means that when retrieving the actual coordinate from , we need to add 
    */
   class FastHessianDetPyramid2d
   {
   public:
      typedef double                   value_type;
      typedef core::Matrix<value_type> Matrix;

   public:
      /**
       @brief Construct and computes the hessian determinant pyramid
       @param i the image,
       @param scales the size of each level of the pyramid (in pixel), must be in increasing order
       @param displacements the step between two filter evaluation for this particular level
       */
      template <class T, class Mapper, class Alloc>
      void construct( const core::Image<T, Mapper, Alloc>& i, const std::vector<ui32>& scales, const std::vector<ui32>& displacements )
      {
         ensure( displacements.size() == scales.size(), "must be the same size" );

         const ui32 nbScales = (ui32)scales.size();

         _scales.clear();
         _displacements.clear();
         _halfScales.clear();
         for ( ui32 n = 0; n < nbScales; ++n )
         {
            _halfScales.push_back( scales[ n ] / 2 );
            _scales.push_back( scales[ n ] );
            _displacements.push_back( displacements[ n ] );
         }

         const T max = (T)std::max( abs( *std::max_element( i.begin(), i.end() ) ),
                                    abs( *std::min_element( i.begin(), i.end() ) ) );

         // construct an integral image
         IntegralImage image;
         image.process( i );
         _integralImage = image;
         for ( ui32 n = 0; n < nbScales; ++n )
         {
            ensure( _scales[ n ] % 2 == 1, "scales must be odd numbers" );
            ensure( _scales[ n ] >= 9, "minimal size" );

            const ui32 lobeSize = _scales[ n ] / 3;
            const value_type areaNormalization = max * _scales[ n ] * _scales[ n ]; // we use <max> so that we are independent of the kind of data
            const i32 step = (i32)displacements[ n ];
            i32 resx = ( (i32)i.sizex() ) / step;
            i32 resy = ( (i32)i.sizey() ) / step;

            // here we want the last (resx, resy) to fully fit inside so that we don't have 
            while ( (int)_getPositionPyramid2IntegralNoShift( (f32)resx, 0, n )[ 0 ] + _scales[ n ] >  (i32)i.sizex() )
               --resx;
            while ( (int)_getPositionPyramid2IntegralNoShift( 0, (f32)resy, n )[ 1 ] + _scales[ n ] >  (i32)i.sizey() )
               --resy;
            if ( resx <= 0 || resy <= 0 )
               break;   // the scale is too big!

            // now compute the hessians for all points inside
            Matrix detHessian( resy, resx );

            // compute the hessian
            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for
            #endif
            for ( int yp = 0; yp < resy; ++yp )
            {
               for ( int xp = 0; xp < resx; ++xp )
               {
                  // Note: (0,0) in the pyramid represents the filter whose top left corner is (0,0)
                  // (i.e., we don't need any shift by scale/2 here)
                  const core::vector2f centerf = getPositionPyramid2Integral( static_cast<f32>( xp ), static_cast<f32>( yp ), n );
                  const core::vector2i center( static_cast<int>( centerf[ 0 ] ), static_cast<int>(centerf[ 1 ] ) );

                  core::vector2ui bl( center[ 0 ], center[ 1 ] );
                  core::vector2ui tr( bl[ 0 ] + scales[ n ] - 1, bl[ 1 ] + scales[ n ] - 1 );
                  if ( tr[ 0 ] < image.sizex() && tr[ 1 ] < image.sizey() )
                  {
                     const value_type dxx = HaarFeatures2d::getValue( HaarFeatures2d::VERTICAL_TRIPLE,
                                                                      image,
                                                                      center,
                                                                      lobeSize ) / areaNormalization;
                     const value_type dyy = HaarFeatures2d::getValue( HaarFeatures2d::HORIZONTAL_TRIPLE,
                                                                      image,
                                                                      center,
                                                                      lobeSize ) / areaNormalization;
                     const value_type dxy = HaarFeatures2d::getValue( HaarFeatures2d::CHECKER,
                                                                      image,
                                                                      center,
                                                                      lobeSize ) / areaNormalization;

                     // here we normalize the dxy, as the area used for the filters are diffent than dxx or dyy
                     // using the frobenius norm (see http://mathworld.wolfram.com/FrobeniusNorm.html)
                     // i.e., CTE = sqrt( 4 * 9 ) / sqrt( 2*9 + 3*7 + 2*3 ) = 0.89
                     static const value_type NORMALIZATION = 0.9;
                     const value_type hessian = dxx * dyy - core::sqr( NORMALIZATION * dxy );
                     detHessian( yp, xp ) = hessian;
                  }
               }
            }

            _pyramidDetHessian.push_back( detHessian );
         }
      }

      /**
       @brief Computes the gradient of the hessian at position (x, y, map)
              using finite difference
       */
      core::vector3d getHessianGradient( i32 x, i32 y, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < (i32)_pyramidDetHessian[ map ].sizex() - 1 &&
                 y < (i32)_pyramidDetHessian[ map ].sizey() - 1 );

         i32 xminus, yminus;
         indexInMap( x, y, map, map - 1, xminus, yminus );  // we need to look up the closed index in a map that has different dimensions

         i32 xplus, yplus;
         indexInMap( x, y, map, map + 1, xplus, yplus ); // we need to look up the closed index in a map that has different dimensions

         const Matrix& current = _pyramidDetHessian[ map ];
         return core::vector3d( ( current( y, x + 1 ) - current( y, x - 1 ) ) / 2,
                                ( current( y + 1, x ) - current( y - 1, x ) ) / 2,
                                ( _pyramidDetHessian[ map + 1 ]( yplus, xplus ) -
                                  _pyramidDetHessian[ map - 1 ]( yminus, xminus ) ) / 2 );
      }

      /**
       @brief Computes the hessian of the hessian at position (x, y, map)
              using finite difference
       */
      Matrix getHessianHessian( i32 x, i32 y, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < (i32)_pyramidDetHessian[ map ].sizex() &&
                 y < (i32)_pyramidDetHessian[ map ].sizey() );

         const Matrix& mc = _pyramidDetHessian[ map ];
         const Matrix& mm = _pyramidDetHessian[ map - 1 ];
         const Matrix& mp = _pyramidDetHessian[ map + 1 ];
         const value_type val = mc( y, x );

         i32 xm, ym;
         indexInMap( x, y, map, map - 1, xm, ym );  // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xm > 0 && ym > 0 &&
                 xm < (i32)_pyramidDetHessian[ map - 1 ].sizex() - 1 &&
                 ym < (i32)_pyramidDetHessian[ map - 1 ].sizey() - 1 );

         i32 xp, yp;
         indexInMap( x, y, map, map + 1, xp, yp ); // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xp > 0 && yp > 0 &&
                 xp < (i32)_pyramidDetHessian[ map + 1 ].sizex() - 1 &&
                 yp < (i32)_pyramidDetHessian[ map + 1 ].sizey() - 1 );

         const value_type dxx = mc( y, x + 1 ) + mc( y, x - 1 ) - 2 * val;
         const value_type dyy = mc( y + 1, x ) + mc( y - 1, x ) - 2 * val;
         const value_type dss = mp( yp, xp )   + mm( ym, xm )   - 2 * val;
         const value_type dxy = ( mc( y + 1, x + 1 ) + mc( y - 1, x - 1 ) -
                                  mc( y - 1, x + 1 ) - mc( y + 1, x - 1 ) ) / 4;
         const value_type dxs = ( mp( yp, xp + 1 ) + mm( ym, xm - 1 ) -
                                  mm( ym, xm + 1 ) - mp( yp, xp - 1 ) ) / 4;
         const value_type dys = ( mp( yp + 1, xp ) + mm( ym - 1, xm ) -
                                  mm( ym + 1, xm ) - mp( yp - 1, xp ) ) / 4;

         Matrix hs( 3, 3 );
         hs( 0, 0 ) = dxx;
         hs( 0, 1 ) = dxy;
         hs( 0, 2 ) = dxs;

         hs( 1, 0 ) = dxy;
         hs( 1, 1 ) = dyy;
         hs( 1, 2 ) = dys;

         hs( 2, 0 ) = dxs;
         hs( 2, 1 ) = dys;
         hs( 2, 2 ) = dss;

         return hs;
      }

      // computes the index in mapDest the closest from (xRef, yRef, mapDest)
      void indexInMap( i32 xpRef, i32 ypRef, ui32 mapRef, ui32 mapDest, i32& outxp, i32& outyp ) const
      {
         if ( mapRef == mapDest )
         {
            outxp = xpRef;
            outyp = ypRef;
         } else {
            // map a point at a given scale to the image space
            const core::vector2f posInIntegral = _getPositionPyramid2IntegralNoShift( (f32)xpRef, (f32)ypRef, mapRef );
            const core::vector2f indexInOtherLevel = _getPositionIntegral2PyramidNoShift( posInIntegral[ 0 ], posInIntegral[ 1 ], mapDest );
            
            // convert the image space coordinate to the other scale space
            outxp = static_cast<i32>( core::round( indexInOtherLevel[ 0 ] ) );
            outyp = static_cast<i32>( core::round( indexInOtherLevel[ 1 ] ) );
         }
      }

      /**
       @brief returns true if all value around the projection (xRef, yRef, mapRef) on mapDest are smaller
       */
      bool isDetHessianMax( value_type val, i32 xRef, i32 yRef, ui32 mapRef, ui32 mapDest ) const
      {
         i32 x, y;

         // if it is outside, then skip it
         indexInMap( xRef, yRef, mapRef, mapDest, x, y );
         if ( mapDest >= _pyramidDetHessian.size() )
            return false;
         const Matrix& m = _pyramidDetHessian[ mapDest ];
         if ( x < 1 || y < 1 || x + 1 >= (int)m.sizex() || y + 1 >= (int)m.sizey() )
            return false;

         return val >= m( y + 0, x + 0 ) &&
                val >= m( y + 1, x + 0 ) &&
                val >= m( y - 1, x + 0 ) &&
                val >= m( y + 0, x + 1 ) &&
                val >= m( y + 1, x + 1 ) &&
                val >= m( y - 1, x + 1 ) &&
                val >= m( y + 0, x - 1 ) &&
                val >= m( y + 1, x - 1 ) &&
                val >= m( y - 1, x - 1 );
      }

      const std::vector<Matrix>& getPyramidDetHessian() const
      {
         return _pyramidDetHessian;
      }

      const IntegralImage& getIntegralImage() const
      {
         return _integralImage;
      }

      /**
       @brief Given an index in the pyramid, retrieve the corresponding index in the original image
              (i.e., the one the pyramid is built from)
       @note This is just the user facing API. Internally we want to use <code>_getPositionPyramid2IntegralNoShift</code>
       */
      core::vector2f getPositionPyramid2Integral( f32 x, f32 y, ui32 map ) const
      {
         return core::vector2f( x * _displacements[ map ] + _halfScales[ map ],
                                y * _displacements[ map ] + _halfScales[ map ] );
      }

      /**
       @brief Given a position in the integral image and a pyramid level, find the corresponding pyramid index at this level
       @note This is just the user facing API. Internally we want to use <code>_getPositionIntegral2PyramidNoShift</code>
       */
      core::vector2f getPositionIntegral2Pyramid( f32 xp, f32 yp, ui32 map ) const
      {
         return core::vector2f( ( xp - _halfScales[ map ] ) / _displacements[ map ],
                                ( yp - _halfScales[ map ] ) / _displacements[ map ] );
      }

   private:
      /**
       @brief Given an index in the pyramid, retrieve the corresponding index in the original image
              (i.e., the one the pyramid is built from) WITHOUT shift by scale / 2
       */
      core::vector2f _getPositionPyramid2IntegralNoShift( f32 x, f32 y, ui32 map ) const
      {
         return core::vector2f( x * _displacements[ map ],
                                y * _displacements[ map ] );
      }

      /**
       @brief Given a position in the integral image and a pyramid level, find the corresponding pyramid index at this level
              WITHOUT shift by scale / 2
       */
      core::vector2f _getPositionIntegral2PyramidNoShift( f32 xp, f32 yp, ui32 map ) const
      {
         return core::vector2f( xp / _displacements[ map ],
                                yp / _displacements[ map ] );
      }

   private:
      std::vector<Matrix>  _pyramidDetHessian;
      std::vector<i32>     _scales;
      std::vector<i32>     _halfScales;
      std::vector<i32>     _displacements;
      IntegralImage        _integralImage;
   };
}
}

#endif