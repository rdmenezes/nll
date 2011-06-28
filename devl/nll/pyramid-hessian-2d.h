/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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
           note the normalization factor 0.9 comes from the gaussian approximation
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

         _pyramidDetHessian.clear();
         _scales = scales;
         _displacements = displacements;

         const T max = (T)std::max( abs( *std::max_element( i.begin(), i.end() ) ),
                                    abs( *std::min_element( i.begin(), i.end() ) ) );

         // construct an integral image
         IntegralImage image;
         image.process( i );
         _integralImage = image;
         for ( size_t n = 0; n < scales.size(); ++n )
         {
            ensure( scales[ n ] % 2 == 1, "scales must be odd numbers" );
            ensure( scales[ n ] >= 9, "minimal size" );

            const ui32 step = displacements[ n ];

            const int sizeFilterx = scales[ n ];
            const int sizeFiltery = scales[ n ];
            const double sizeFilter = sizeFilterx * sizeFiltery * max; // we normalize by the filter size and maximum value

            const int resx = ( (int)i.sizex() ) / (int)step;
            const int resy = ( (int)i.sizey() ) / (int)step;

            if ( resx <= 0 || resy <= 0 )
               break;   // the scale is too big!
            Matrix detHessian( resy, resx );

            // compute the hessian
            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for
            #endif
            for ( int y = 0; y < resy; ++y )
            {
               for ( int x = 0; x < resx; ++x )
               {
                  core::vector2ui bl( x * step, y * step );
                  core::vector2ui tr( bl[ 0 ] + sizeFilterx - 1, bl[ 1 ] + sizeFiltery - 1 );
                  if ( tr[ 0 ] < image.sizex() && tr[ 1 ] < image.sizey() )
                  {
                     const double dxx = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::VERTICAL_TRIPLE,
                                                                           image,
                                                                           bl,
                                                                           tr ) / sizeFilter;
                     const double dyy = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::HORIZONTAL_TRIPLE,
                                                                           image,
                                                                           bl,
                                                                           tr ) / sizeFilter;
                     const double dxy = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::CHECKER,
                                                                           image,
                                                                           bl,
                                                                           tr ) / sizeFilter;
                     detHessian( y, x ) = dxx * dyy - core::sqr( 0.9 * dxy );
                  } else {
                     detHessian( y, x ) = 0;
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
      core::vector3d getHessianGradient( ui32 x, ui32 y, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].sizex() - 1 &&
                 y < _pyramidDetHessian[ map ].sizey() - 1 );

         int xminus, yminus;
         indexInMap( x, y, map, map - 1, xminus, yminus );  // we need to look up the closed index in a map that has different dimensions

         int xplus, yplus;
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
      Matrix getHessianHessian( ui32 x, ui32 y, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].sizex() &&
                 y < _pyramidDetHessian[ map ].sizey() );

         const Matrix& mc = _pyramidDetHessian[ map ];
         const Matrix& mm = _pyramidDetHessian[ map - 1 ];
         const Matrix& mp = _pyramidDetHessian[ map + 1 ];
         const value_type val = mc( y, x );

         int xm, ym;
         indexInMap( x, y, map, map - 1, xm, ym );  // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xm > 0 && ym > 0 &&
                 xm < (int)_pyramidDetHessian[ map - 1 ].sizex() - 1 &&
                 ym < (int)_pyramidDetHessian[ map - 1 ].sizey() - 1 );

         int xp, yp;
         indexInMap( x, y, map, map + 1, xp, yp ); // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xp > 0 && yp > 0 &&
                 xp < (int)_pyramidDetHessian[ map + 1 ].sizex() - 1 &&
                 yp < (int)_pyramidDetHessian[ map + 1 ].sizey() - 1 );

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
      void indexInMap( ui32 xRef, ui32 yRef, ui32 mapRef, ui32 mapDest, int& outx, int& outy ) const
      {
         if ( mapRef == mapDest )
         {
            outx = xRef;
            outy = yRef;
         } else {
            // map a point at a given scale to the image space
            const int x = xRef * _displacements[ mapRef ];
            const int y = yRef * _displacements[ mapRef ];

            // convert the image space coordinate to the other scale space
            outx = ( x ) / (int)_displacements[ mapDest ];
            outy = ( y ) / (int)_displacements[ mapDest ];
         }
      }

      /**
       @brief returns true if all value around the projection (xRef, yRef, mapRef) on mapDest are smaller
       */
      bool isDetHessianMax( value_type val, ui32 xRef, ui32 yRef, ui32 mapRef, ui32 mapDest ) const
      {
         int x, y;

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

   private:
      std::vector<Matrix>  _pyramidDetHessian;
      std::vector<ui32>    _scales;
      std::vector<ui32>    _displacements;
      IntegralImage        _integralImage;
   };
}
}

#endif