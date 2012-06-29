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

#ifndef NLL_ALGORITHM_PYRAMID_HESSIAN_3D_H_
# define NLL_ALGORITHM_PYRAMID_HESSIAN_3D_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Hold a stack of the hessian determinant using a crude approximation of a gaussian in 2D
           for each point,           | Lxx Lxy Lxz |   | a b c |
                           H(x, o) = | Lyx Lyy Lyz | = | d e f |
                                     | Lzx Lzy Lzz |   | g h i |

                           det(H)= Lxx * | Lyy Lyz | - Lyy * | Lxx Lxz | + Lzz * | Lxx Lxy |
                                         | Lzy Lzz |         | Lzx Lzz |         | Lyx Lyy |
    @note the area used to compute Lxx Lxy... are different and must be normalized, e.g., using the frobenius norm L=3
    */
   class FastHessianDetPyramid3d
   {
   public:
      typedef double                      value_type; // float as we don't need that much accuracy, we will save a lot of space like this...
      typedef imaging::Volume<value_type> Volume;
      typedef core::Matrix<value_type>    Matrix;

   public:
      /**
       @brief Construct and computes the hessian determinant pyramid
       @param i the image,
       @param scales the size of each level of the pyramid (in pixel), must be in increasing order
       @param displacements the step between two filter evaluation for this particular level
       */
      template <class VolumeT>
      void construct( const VolumeT& i, const std::vector<size_t>& scales, const std::vector<size_t>& displacements )
      {
         ensure( displacements.size() == scales.size(), "must be the same size" );

         _pyramidDetHessian.clear();
         _scales = scales;
         _displacements = displacements;

         const size_t sizex = i.size()[ 0 ];
         const size_t sizey = i.size()[ 1 ];
         const size_t sizez = i.size()[ 2 ];

         const value_type max = (value_type)std::max( abs( *std::max_element( i.begin(), i.end() ) ),
                                                      abs( *std::min_element( i.begin(), i.end() ) ) );

         // construct an integral image
         IntegralImage3d image;
         image.process( i );
         _integralImage = image;

         for ( size_t n = 0; n < scales.size(); ++n )
         {
            core::Timer scaleTimer;
            ensure( scales[ n ] % 2 == 1, "scales must be odd numbers" );
            ensure( scales[ n ] >= 9, "minimal size" );

            const size_t step = displacements[ n ];

            const int sizeFilterz = static_cast<int>( scales[ n ] );
            const int sizeFilterx = static_cast<int>( scales[ n ] );
            const int sizeFiltery = static_cast<int>( scales[ n ] );
            const double sizeFilter = sizeFilterx * sizeFiltery * sizeFilterz * max; // we normalize by the filter size and maximum value

            // the total size must take into account the step size and filter size (it must be fully inside the image to be computed)
            const int resx = ( static_cast<int>( i.size()[ 0 ] ) ) / static_cast<int>( step );
            const int resy = ( static_cast<int>( i.size()[ 1 ] ) ) / static_cast<int>( step );
            const int resz = ( static_cast<int>( i.size()[ 2 ] ) ) / static_cast<int>( step );

            if ( resx <= 0 || resy <= 0 || resz <= 0 )
               break;   // the scale is too big!
            Volume detHessian( resx, resy, resz );

            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for
            #endif
            for ( int z = 0; z < resz; ++z )
            {
               for ( int y = 0; y < resy; ++y )
               {
                  for ( int x = 0; x < resx; ++x )
                  {
                     const core::vector3ui bl( x * step, y * step, z * step );
                     const core::vector3ui tr( bl[ 0 ] + sizeFilterx - 1, bl[ 1 ] + sizeFiltery - 1, bl[ 2 ] + sizeFilterz - 1 );
                     if ( tr[ 0 ] < sizex && tr[ 1 ] < sizey && tr[ 2 ] < sizez )
                     {
                        const double dxx = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DX,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dyy = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DY,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dzz = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DZ,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dxy = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DXY,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dxz = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DXZ,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dyz = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DYZ,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        
                        // the dxx/dxy filters have a different area, so normalize it with the frobenius norm, L=3
                        // f = (9^3-4*9^2)^1/3 / (9^3-9*(2*9+3*7+2*3)^1/3 = 0.928
                        // see http://mathworld.wolfram.com/FrobeniusNorm.html
                        static const double NORMALIZATION = 0.928 * 0.928;
                        const double val = dxx * ( dyy * dzz - NORMALIZATION * dyz * dyz ) -
                                           dyy * ( dxx * dzz - NORMALIZATION * dxz * dxz ) +
                                           dzz * ( dxx * dyy - NORMALIZATION * dxy * dxy );
                        detHessian( x, y, z ) = static_cast<value_type>( val );
                     }
                  }
               }
            }

            std::cout << "Time scale=" << scaleTimer.getCurrentTime() << std::endl;
            _pyramidDetHessian.push_back( detHessian );
         }
      }

      // computes the index in mapDest the closest from (xRef, yRef, mapDest)
      void indexInMap( size_t xRef, size_t yRef, size_t zRef, size_t mapRef, size_t mapDest, int& outx, int& outy, int& outz ) const
      {
         if ( mapRef == mapDest )
         {
            outx = static_cast<int>( xRef );
            outy = static_cast<int>( yRef );
            outz = static_cast<int>( zRef );
         } else {
            // map a point at a given scale to the image space
            const int x = static_cast<int>( xRef * _displacements[ mapRef ] );
            const int y = static_cast<int>( yRef * _displacements[ mapRef ] );
            const int z = static_cast<int>( zRef * _displacements[ mapRef ] );

            // convert the image space coordinate to the other scale space
            outx = ( x ) / static_cast<int>( _displacements[ mapDest ] );
            outy = ( y ) / static_cast<int>( _displacements[ mapDest ] );
            outz = ( z ) / static_cast<int>( _displacements[ mapDest ] );
         }
      }

      /**
       @brief Computes the gradient of the hessian at position (x, y, z, map)
              using finite difference
       */
      core::vector4d getHessianGradient( size_t x, size_t y, size_t z, size_t map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && z > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].size()[ 0 ] - 1 &&
                 y < _pyramidDetHessian[ map ].size()[ 1 ] - 1 &&
                 z < _pyramidDetHessian[ map ].size()[ 2 ] - 1 );

         int xminus, yminus, zminus;
         indexInMap( x, y, z, map, map - 1, xminus, yminus, zminus );  // we need to look up the closed index in a map that has different dimensions

         int xplus, yplus, zplus;
         indexInMap( x, y, z, map, map + 1, xplus, yplus, zplus ); // we need to look up the closed index in a map that has different dimensions

         const Volume& current = _pyramidDetHessian[ map ];
         return core::vector4d( ( current( x + 1, y, z ) - current( x - 1, y, z ) ) / 2,
                                ( current( x, y + 1, z ) - current( x, y - 1, z ) ) / 2,
                                ( current( x, y, z + 1 ) - current( x, y, z - 1 ) ) / 2,
                                ( _pyramidDetHessian[ map + 1 ]( xplus,  yplus,  zplus ) -
                                  _pyramidDetHessian[ map - 1 ]( xminus, yminus, zplus ) ) / 2 );
      }

      /**
       @brief returns true if all value around the projection (xRef, yRef, mapRef) on mapDest are smaller
       */
      bool isDetHessianMax( value_type val, size_t xRef, size_t yRef, size_t zRef, size_t mapRef, size_t mapDest ) const
      {
         int x, y, z;

         // if it is outside, then skip it
         indexInMap( xRef, yRef, zRef, mapRef, mapDest, x, y, z );
         if ( mapDest >= _pyramidDetHessian.size() )
            return false;
         const Volume& m = _pyramidDetHessian[ mapDest ];
         if ( x < 1 || y < 1 || z < 1 || x + 1 >= (int)m.size()[ 0 ] || y + 1 >= (int)m.size()[ 1 ] || z + 1 >= (int)m.size()[ 2 ] )
            return false;

         return 
                // current slice
                val >= m( x + 0, y + 0, z ) &&
                val >= m( x + 1, y + 0, z ) &&
                val >= m( x - 1, y + 0, z ) &&
                val >= m( x + 0, y + 1, z ) &&
                val >= m( x + 1, y + 1, z ) &&
                val >= m( x - 1, y + 1, z ) &&
                val >= m( x + 0, y - 1, z ) &&
                val >= m( x + 1, y - 1, z ) &&
                val >= m( x - 1, y - 1, z ) &&

                // direct z neighbours
                val >= m( x, y, z - 1 ) &&
                val >= m( x, y, z + 1 ) &&

                // upper and lower rest of the slices
                val >= m( x + 0, y + 0, z + 1 ) &&
                val >= m( x + 1, y + 0, z + 1 ) &&
                val >= m( x - 1, y + 0, z + 1 ) &&
                val >= m( x + 0, y + 1, z + 1 ) &&
                val >= m( x + 1, y + 1, z + 1 ) &&
                val >= m( x - 1, y + 1, z + 1 ) &&
                val >= m( x + 0, y - 1, z + 1 ) &&
                val >= m( x + 1, y - 1, z + 1 ) &&
                val >= m( x - 1, y - 1, z + 1 ) &&

                val >= m( x + 0, y + 0, z - 1 ) &&
                val >= m( x + 1, y + 0, z - 1 ) &&
                val >= m( x - 1, y + 0, z - 1 ) &&
                val >= m( x + 0, y + 1, z - 1 ) &&
                val >= m( x + 1, y + 1, z - 1 ) &&
                val >= m( x - 1, y + 1, z - 1 ) &&
                val >= m( x + 0, y - 1, z - 1 ) &&
                val >= m( x + 1, y - 1, z - 1 ) &&
                val >= m( x - 1, y - 1, z - 1 );
      }

      /**
       @brief Computes the hessian of the hessian at position (x, y, z, map)
              using finite difference
       */
      Matrix getHessianHessian( size_t x, size_t y, size_t z, size_t map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && z > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].size()[ 0 ] &&
                 y < _pyramidDetHessian[ map ].size()[ 1 ] &&
                 z < _pyramidDetHessian[ map ].size()[ 2 ] );

         const Volume& mc = _pyramidDetHessian[ map ];
         const Volume& mm = _pyramidDetHessian[ map - 1 ];
         const Volume& mp = _pyramidDetHessian[ map + 1 ];
         const value_type val = mc( x, y, z );

         int xm, ym, zm;
         indexInMap( x, y, z, map, map - 1, xm, ym, zm );  // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xm > 0 && ym > 0 && zm > 0 &&
                 xm < (int)_pyramidDetHessian[ map - 1 ].size()[ 0 ] - 1 &&
                 ym < (int)_pyramidDetHessian[ map - 1 ].size()[ 1 ] - 1 &&
                 zm < (int)_pyramidDetHessian[ map - 1 ].size()[ 2 ] - 1 );

         int xp, yp, zp;
         indexInMap( x, y, z, map, map + 1, xp, yp, zp ); // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xp > 0 && yp > 0 && zp > 0 &&
                 xp < (int)_pyramidDetHessian[ map + 1 ].size()[ 0 ] - 1 &&
                 yp < (int)_pyramidDetHessian[ map + 1 ].size()[ 1 ] - 1 && 
                 zp < (int)_pyramidDetHessian[ map + 1 ].size()[ 2 ] - 1 );

         //std::cout << "check=" << x << " " << y << " " << z << " " << " toMP=" << xp << " " << yp << " " << zp << " sizeM=" << mp.size() << std::endl;
         const value_type dxx = mc( x + 1, y, z ) + mc( x - 1, y, z ) - 2 * val;
         const value_type dyy = mc( x, y + 1, z ) + mc( x, y - 1, z ) - 2 * val;
         const value_type dzz = mc( x, y, z + 1 ) + mc( x, y, z - 1 ) - 2 * val;
         const value_type dss = mp( xp, yp, zp )   + mm( xm, ym, zm ) - 2 * val;

         const value_type dxy = ( mc( x + 1, y + 1, z ) + mc( x - 1, y - 1, z ) -
                                  mc( x + 1, y - 1, z ) - mc( x - 1, y + 1, z ) ) / 4;
         const value_type dxz = ( mc( x + 1, y, z + 1 ) + mc( x - 1, y, z - 1 ) -
                                  mc( x + 1, y, z - 1 ) - mc( x - 1, y, z + 1 ) ) / 4;
         const value_type dyz = ( mc( x, y + 1, z + 1 ) + mc( x, y - 1, z + 1 ) -
                                  mc( x, y - 1, z + 1 ) - mc( x, y + 1, z + 1 ) ) / 4;

         const value_type dxs = ( mp( xp + 1, yp, zp ) + mm( xm - 1, ym, zm ) -
                                  mm( xm + 1, ym, zm ) - mp( xp - 1, yp, zp ) ) / 4;
         const value_type dys = ( mp( xp, yp + 1, zp ) + mm( xm, ym - 1, zm ) -
                                  mm( xm, ym + 1, zm ) - mp( xp, yp - 1, zp ) ) / 4;
         const value_type dzs = ( mp( xp, yp, zp + 1 ) + mm( xm, ym, zm - 1 ) -
                                  mm( xm, ym, zm + 1 ) - mp( xp, yp, zp - 1 ) ) / 4;

         // returns the hessian matrix defined as:
         //     | dxx dxy dxz dxs |
         // H = | dyx dyy dyz dys |
         //     | dzx dzy dzz dzs |
         //     | dsx dsy dsz dss |
         Matrix hs( 4, 4 );
         
         hs( 0, 0 ) = dxx;
         hs( 0, 1 ) = dxy;
         hs( 0, 2 ) = dxz;
         hs( 0, 3 ) = dxs;

         hs( 1, 0 ) = dxy;
         hs( 1, 1 ) = dyy;
         hs( 1, 2 ) = dyz;
         hs( 1, 3 ) = dys;

         hs( 2, 0 ) = dxz;
         hs( 2, 1 ) = dyz;
         hs( 2, 2 ) = dzz;
         hs( 2, 3 ) = dzs;

         hs( 3, 0 ) = dxs;
         hs( 3, 1 ) = dys;
         hs( 3, 2 ) = dzs;
         hs( 3, 3 ) = dss;

         return hs;
      }

      const std::vector<Volume>& getPyramidDetHessian() const
      {
         return _pyramidDetHessian;
      }

      const IntegralImage3d& getIntegralImage() const
      {
         return _integralImage;
      }

   private:
      std::vector<Volume>  _pyramidDetHessian;
      std::vector<size_t>    _scales;
      std::vector<size_t>    _displacements;
      IntegralImage3d      _integralImage;
   };
}
}

#endif