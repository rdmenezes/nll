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

                           det(H)= Lxx * | Lyy Lyz | - Lxy * | Lyx Lyz | + Lxz * | Lyx Lyy |
                                         | Lzy Lzz |         | Lzx Lzz |         | Lzx Lzy |
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
         typedef typename VolumeT::value_type   T;
         ensure( displacements.size() == scales.size(), "must be the same size" );

         const size_t nbScales = (size_t)scales.size();

         _scales.clear();
         _displacements.clear();
         _halfScales.clear();
         for ( size_t n = 0; n < nbScales; ++n )
         {
            _halfScales.push_back( static_cast<i32>( scales[ n ] / 2 ) );
            _scales.push_back( static_cast<i32>( scales[ n ] ) );
            _displacements.push_back( static_cast<i32>( displacements[ n ] ) );
         }

         const T max = (T)std::max( abs( *std::max_element( i.begin(), i.end() ) ),
                                    abs( *std::min_element( i.begin(), i.end() ) ) );

         // construct an integral image
         IntegralImage3d image;
         image.process( i );
         _integralImage = image;
         for ( size_t n = 0; n < nbScales; ++n )
         {
            ensure( _scales[ n ] % 2 == 1, "scales must be odd numbers" );
            ensure( _scales[ n ] >= 9, "minimal size" );

            const size_t lobeSize = _scales[ n ] / 3;
            const value_type areaNormalization = max * _scales[ n ] * _scales[ n ] * _scales[ n ]; // we use <max> so that we are independent of the kind of data
            const i32 step = (i32)displacements[ n ];
            i32 resx = ( (i32)i.sizex() ) / step;
            i32 resy = ( (i32)i.sizey() ) / step;
            i32 resz = ( (i32)i.sizez() ) / step;

            // here we want the last (resx, resy) to fully fit inside so that we don't have 
            while ( (int)_getPositionPyramid2IntegralNoShift( (f32)resx, 0, 0, n )[ 0 ] + _scales[ n ] >  (i32)i.sizex() )
               --resx;
            while ( (int)_getPositionPyramid2IntegralNoShift( 0, (f32)resy, 0, n )[ 1 ] + _scales[ n ] >  (i32)i.sizey() )
               --resy;
            while ( (int)_getPositionPyramid2IntegralNoShift( 0, 0, (f32)resz, n )[ 2 ] + _scales[ n ] >  (i32)i.sizez() )
               --resz;
            if ( resx <= 0 || resy <= 0 || resz <= 0 )
               break;   // the scale is too big!

            // now compute the hessians for all points inside
            Volume detHessian( resx, resy, resz );

            // compute the hessian
            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for
            #endif
            for ( int zp = 0; zp < resz; ++zp )
            {
               for ( int yp = 0; yp < resy; ++yp )
               {
                  for ( int xp = 0; xp < resx; ++xp )
                  {
                     // get the coordinates in the original geometry
                     const core::vector3f centerf = getPositionPyramid2Integral( static_cast<f32>( xp ), static_cast<f32>( yp ), static_cast<f32>( zp ), n );
                     const core::vector3i center( static_cast<int>( centerf[ 0 ] ), static_cast<int>(centerf[ 1 ] ), static_cast<int>(centerf[ 2 ] ) );

                     {
                        const double dxx = HaarFeatures3d::getValue( HaarFeatures3d::D2X,
                                                                     image,
                                                                     center,
                                                                     lobeSize ) / areaNormalization;
                        const double dyy = HaarFeatures3d::getValue( HaarFeatures3d::D2Y,
                                                                     image,
                                                                     center,
                                                                     lobeSize ) / areaNormalization;
                        const double dzz = HaarFeatures3d::getValue( HaarFeatures3d::D2Z,
                                                                     image,
                                                                     center,
                                                                     lobeSize ) / areaNormalization;
                        const double dxy = HaarFeatures3d::getValue( HaarFeatures3d::D2XY,
                                                                     image,
                                                                     center,
                                                                     lobeSize ) / areaNormalization;
                        const double dxz = HaarFeatures3d::getValue( HaarFeatures3d::D2XZ,
                                                                     image,
                                                                     center,
                                                                     lobeSize ) / areaNormalization;
                        const double dyz = HaarFeatures3d::getValue( HaarFeatures3d::D2YZ,
                                                                     image,
                                                                     center,
                                                                     lobeSize ) / areaNormalization;
                        
                        // the dxx/dxy filters have a different area, so normalize it with the frobenius norm, L=3
                        // f = (9^3-4*9^2)^1/3 / (9^3-9*(2*9+3*7+2*3)^1/3 = 0.928
                        // see http://mathworld.wolfram.com/FrobeniusNorm.html
                        // det(H)= Lxx * | Lyy Lyz | - Lxy * | Lyx Lyz | + Lxz * | Lyx Lyy |
                        //               | Lzy Lzz |         | Lzx Lzz |         | Lzx Lzy |
                        //
                        static const double NORMALIZATION = 0.928;
                        
                        /*
                        // true determinant, however we dond find the expected points...
                        const double val =                 dxx *  (                                dyy * dzz - NORMALIZATION * NORMALIZATION * dyz * dyz ) -
                                           NORMALIZATION * dxy * ( NORMALIZATION *                 dxy * dzz - NORMALIZATION * NORMALIZATION * dyz * dxz ) +
                                           NORMALIZATION * dxz * ( NORMALIZATION * NORMALIZATION * dxy * dyz - NORMALIZATION                 * dyy * dxz );
                                           */

                        // here we want to find the blobs as this is used by SURF...
                        // TODO: refactor so that we can extract the criteria to be used...
                        const double val = dxx * dxx + dyy * dyy + dzz * dzz - core::sqr( NORMALIZATION * dxy ) - core::sqr( NORMALIZATION * dxz ) - core::sqr( NORMALIZATION * dxz );
                        detHessian( xp, yp, zp ) = static_cast<value_type>( val );
                     }
                  }
               }
            }

            _pyramidDetHessian.push_back( detHessian );
         }
      }

      // computes the index in mapDest the closest from (xRef, yRef, mapDest)
      void indexInMap( i32 xpRef, i32 ypRef, i32 zpRef, size_t mapRef, size_t mapDest, i32& outxp, i32& outyp, i32& outzp ) const
      {
         if ( mapRef == mapDest )
         {
            outxp = xpRef;
            outyp = ypRef;
            outzp = zpRef;
         } else {
            // map a point at a given scale to the image space
            /*
            const core::vector3f posInIntegral = _getPositionPyramid2IntegralNoShift( (f32)xpRef, (f32)ypRef, (f32)zpRef, mapRef );
            const core::vector3f indexInOtherLevel = _getPositionIntegral2PyramidNoShift( posInIntegral[ 0 ], posInIntegral[ 1 ], posInIntegral[ 2 ], mapDest );
            */
            
            // TODO: Here difference with 2D version, however better scaling detection with this version...
            const core::vector3f posInIntegral = getPositionPyramid2Integral( (f32)xpRef, (f32)ypRef, (f32)zpRef, mapRef );
            const core::vector3f indexInOtherLevel = getPositionIntegral2Pyramid( posInIntegral[ 0 ], posInIntegral[ 1 ], posInIntegral[ 2 ], mapDest );
            
            // convert the image space coordinate to the other scale space
            outxp = static_cast<i32>( core::round( indexInOtherLevel[ 0 ] ) );
            outyp = static_cast<i32>( core::round( indexInOtherLevel[ 1 ] ) );
            outzp = static_cast<i32>( core::round( indexInOtherLevel[ 2 ] ) );
         }
      }

      /**
       @brief Computes the gradient of the hessian at position (x, y, z, map)
              using finite difference
       */
      core::vector4d getHessianGradient( int x, int y, int z, size_t map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && z > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].size()[ 0 ] - 1 &&
                 y < _pyramidDetHessian[ map ].size()[ 1 ] - 1 &&
                 z < _pyramidDetHessian[ map ].size()[ 2 ] - 1 );

         i32 xminus, yminus, zminus;
         indexInMap( x, y, z, map, map - 1, xminus, yminus, zminus );  // we need to look up the closed index in a map that has different dimensions

         i32 xplus, yplus, zplus;
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
      bool isDetHessianMax( value_type val, int xRef, int yRef, int zRef, size_t mapRef, size_t mapDest ) const
      {
         i32 x, y, z;

         // if it is outside, then skip it
         indexInMap( xRef, yRef, zRef, mapRef, mapDest, x, y, z );
         if ( mapDest >= _pyramidDetHessian.size() )
            return false;
         const Volume& m = _pyramidDetHessian[ mapDest ];
         if ( x < 1 || y < 1 || z < 1 || x + 1 >= (i32)m.size()[ 0 ] || y + 1 >= (i32)m.size()[ 1 ] || z + 1 >= (i32)m.size()[ 2 ] )
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
      Matrix getHessianHessian( int x, int y, int z, size_t map ) const
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

         i32 xm, ym, zm;
         indexInMap( x, y, z, map, map - 1, xm, ym, zm );  // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xm > 0 && ym > 0 && zm > 0 &&
                 xm < (i32)_pyramidDetHessian[ map - 1 ].size()[ 0 ] - 1 &&
                 ym < (i32)_pyramidDetHessian[ map - 1 ].size()[ 1 ] - 1 &&
                 zm < (i32)_pyramidDetHessian[ map - 1 ].size()[ 2 ] - 1 );

         i32 xp, yp, zp;
         indexInMap( x, y, z, map, map + 1, xp, yp, zp ); // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xp > 0 && yp > 0 && zp > 0 &&
                 xp < (i32)_pyramidDetHessian[ map + 1 ].size()[ 0 ] - 1 &&
                 yp < (i32)_pyramidDetHessian[ map + 1 ].size()[ 1 ] - 1 && 
                 zp < (i32)_pyramidDetHessian[ map + 1 ].size()[ 2 ] - 1 );

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

      /**
       @brief Given an index in the pyramid, retrieve the corresponding index in the original image
              (i.e., the one the pyramid is built from)
       */
      core::vector3f getPositionPyramid2Integral( f32 x, f32 y, f32 z, size_t map ) const
      {
         return core::vector3f( x * _displacements[ map ] + _halfScales[ map ],
                                y * _displacements[ map ] + _halfScales[ map ],
                                z * _displacements[ map ] + _halfScales[ map ] );
      }

      /**
       @brief Given a position in the integral image and a pyramid level, find the corresponding pyramid index at this level
       */
      core::vector3f getPositionIntegral2Pyramid( f32 xp, f32 yp, f32 zp, size_t map ) const
      {
         return core::vector3f( ( xp - _halfScales[ map ] ) / _displacements[ map ],
                                ( yp - _halfScales[ map ] ) / _displacements[ map ],
                                ( zp - _halfScales[ map ] ) / _displacements[ map ] );
      }

   private:
      /**
       @brief Given an index in the pyramid, retrieve the corresponding index in the original image
              (i.e., the one the pyramid is built from) WITHOUT shift by scale / 2
       */
      core::vector3f _getPositionPyramid2IntegralNoShift( f32 x, f32 y, f32 z, size_t map ) const
      {
         return core::vector3f( x * _displacements[ map ],
                                y * _displacements[ map ],
                                z * _displacements[ map ] );
      }

      /**
       @brief Given a position in the integral image and a pyramid level, find the corresponding pyramid index at this level
              WITHOUT shift by scale / 2
       */
      core::vector3f _getPositionIntegral2PyramidNoShift( f32 xp, f32 yp, f32 zp, size_t map ) const
      {
         return core::vector3f( xp / _displacements[ map ],
                                yp / _displacements[ map ],
                                zp / _displacements[ map ] );
      }

   private:
      std::vector<Volume>  _pyramidDetHessian;
      std::vector<i32>     _halfScales;
      std::vector<i32>     _scales;
      std::vector<i32>     _displacements;
      IntegralImage3d      _integralImage;
   };
}
}

#endif