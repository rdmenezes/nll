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

#ifndef NLL_IMAGING_VOLUME_INTERPOLATOR_H_
# define NLL_IMAGING_VOLUME_INTERPOLATOR_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Nearest neighbour interpolator. (0,0) is the center of the voxel.

    Volume must be of a volume type.
    */
   template <class Volume>
   class InterpolatorNearestNeighbour
   {
   public:
      typedef Volume                            VolumeType;
      typedef typename VolumeType::value_type   value_type;

   public:
      /**
       @brief This method must be called before any interpolation is made
       */
      void startInterpolation()
      {
         // nothing to do
      }

      /**
       @brief This method must be called after any interpolation is made
       */
      void endInterpolation()
      {
         // nothing to do
      }

      /**
       @brief Construct an interpolator for the volume v. 

       v must remain valid until the end of the calls to the interpolator
       */
      InterpolatorNearestNeighbour( const VolumeType& v ) : _volume( &v )
      {}

      /**
       @brief (x, y, z, PADDING) must be an index. It returns background if the point is outside the volume. (0,0) is the center of the voxel.
       */
      value_type operator()( const float* pos ) const
      {
         const int ix = core::floor( pos[ 0 ] + 0.5f );
         const int iy = core::floor( pos[ 1 ] + 0.5f );
         const int iz = core::floor( pos[ 2 ] + 0.5f );

         if ( _volume->inside( ix, iy, iz ) )
            return (*_volume)( ix, iy, iz );
         return _volume->getBackgroundValue();
      }

   protected:
      const VolumeType* _volume;
   };


   /**
    @ingroup imaging
    @brief Trilinear interpolator of a volume. (0,0) is the center of the voxel.

    Volume must be of a volume type or derived.

    See http://en.wikipedia.org/wiki/Trilinear_interpolation for equations

    @note if VolumeT::value_type is a vector type, it must implement operator *, +
    */
   template <class VolumeT>
   class InterpolatorTriLinearDummy
   {
   public:
      typedef typename VolumeT::value_type   value_type;
      typedef VolumeT                        VolumeType;

      // if value_type::value_type is a floating value, simply take this type, else default to "float"
      typedef typename core::If<value_type, float, core::IsFloatingType<value_type>::value >::type value_type_floating;

   public:
      /**
       @brief This method must be called before any interpolation is made
       */
      void startInterpolation()
      {
         iix = -1;
      }

      /**
       @brief This method must be called after any interpolation is made
       */
      void endInterpolation()
      {
         // nothing to do
      }

      /**
       @brief Construct an interpolator for the volume v. 

       v must remain valid until the end of the calls to the interpolator
       */
      InterpolatorTriLinearDummy( const VolumeType& v ) : _volume( &v ), _sizeCheck( static_cast<int>( v.sizex() ) - 1,
                                                                                     static_cast<int>( v.sizey() ) - 1,
                                                                                     static_cast<int>( v.sizez() ) - 1 )
      {}

      /**
       @brief Compute the weights associated to this position on the 8 neighboring voxels
       @param pos a vector encoded as (x, y, z, 0) specififying the position in voxel
       @param weights assumed already allocated 8-vector storing the weights 0123 as (p, p+i, p+i+j, p+j)
              and 4567 as (p+k, p+i+k, p+i+j+k, p+j+k) where (i,j,k) vector director (1,0,0) (0,1,0) (0,0,1)
       */
      void computeWeights( const float* pos, float* weights, int& ix, int& iy, int& iz )
      {
         ix = core::floor( pos[ 0 ] );
         iy = core::floor( pos[ 1 ] );
         iz = core::floor( pos[ 2 ] );

         const value_type_floating dx = fabs( pos[ 0 ] - ix );
         const value_type_floating dy = fabs( pos[ 1 ] - iy );
         const value_type_floating dz = fabs( pos[ 2 ] - iz );

         const value_type_floating dxdy = dx * dy;
         const value_type_floating dydz = dy * dz;
         /*
         weights[7] = dxdy * dz;
         weights[6] = dx * dz          - weights[ 7 ];
         weights[5] = dxdy             - weights[ 7 ];
         weights[3] = dydz             - weights[ 7 ];
         weights[4] = dx - dxdy        - weights[ 6 ];
         weights[2] = dz - dydz        - weights[ 6 ];
         weights[1] = dy - dydz        - weights[ 5 ];
         weights[0] = 1 -dy -dz + dydz - weights[ 4 ]; 
         */

         weights[6] = dxdy * dz;

         weights[5] = dx * dz          - weights[ 6 ];
         weights[2] = dxdy             - weights[ 6 ];
         weights[7] = dydz             - weights[ 6 ];
         weights[1] = dx - dxdy        - weights[ 5 ];
         weights[4] = dz - dydz        - weights[ 5 ];
         weights[3] = dy - dydz        - weights[ 2 ];
         weights[0] = 1 -dy -dz + dydz - weights[ 1 ]; 
      }

      /**
       @brief (x, y, z, PADDING) must be an index. It returns background if the point is outside the volume
       @note always align the pointer using NLL_ALIGN_16 in case SSE2 optim are used...
       */
      value_type operator()( const float* pos ) const
      {
         const int ix = core::floor( pos[ 0 ] );
         const int iy = core::floor( pos[ 1 ] );
         const int iz = core::floor( pos[ 2 ] );

         // 0 <-> size - 1 as we need an extra sample for linear interpolation
         if ( ix < 0 || ix >= _sizeCheck[ 0 ] ||
              iy < 0 || iy >= _sizeCheck[ 1 ] ||
              iz < 0 || iz >= _sizeCheck[ 2 ] )
         {
            return _volume->getBackgroundValue();;
         }

         const value_type_floating dx = fabs( pos[ 0 ] - ix );
         const value_type_floating dy = fabs( pos[ 1 ] - iy );
         const value_type_floating dz = fabs( pos[ 2 ] - iz );

         // Often in the same neighbourhood, we are using the same voxel, but at a slightly different
         // position, so we are caching the previous result, and reuse it if necessary
         if ( ix != iix || iy != iiy || iz != iiz )
         {
            // update the position to possibly use the cached values next iteration
            iix = ix;
            iiy = iy;
            iiz = iz;

            // case we are not using the cached values
            typename VolumeType::ConstDirectionalIterator it = _volume->getIterator( ix, iy, iz );
            typename VolumeType::ConstDirectionalIterator itz( it );
            itz.addz();

            v000 = *it;

            v100 = it.pickx();
            v101 = itz.pickx();
            v010 = it.picky();
            v011 = itz.picky();
            v001 = it.pickz();
            v110 = *it.addx().addy();
            v111 = it.pickz();            
         }

         const value_type i1 = v000 + ( v001 - v000 ) * dz;
         const value_type i2 = v010 + ( v011 - v010 ) * dz;
         const value_type j1 = v100 + ( v101 - v100 ) * dz;
         const value_type j2 = v110 + ( v111 - v110 ) * dz;

         const value_type w1 = i1 * ( 1 - dy ) + i2 * dy;
         const value_type w2 = j1 * ( 1 - dy ) + j2 * dy;

         const value_type value = w1 * ( 1 - dx ) + w2 * dx;
         return value;
      }

   protected:
      const VolumeType*       _volume;
      core::vector3i          _sizeCheck;

      mutable value_type v000;
      mutable value_type v001, v010, v011, v100, v110, v101, v111;
      mutable int iix, iiy, iiz;
   };


   /**
    @ingroup imaging
    @brief Trilinear interpolator of a volume. (0,0) is the center of the voxel.

    Volume must be of a volume type or derived.

    See http://en.wikipedia.org/wiki/Trilinear_interpolation for equations
    */
   template <class Volume>
   class InterpolatorTriLinear
   {
   public:
      typedef Volume   VolumeType;

      // if the volume is a floating point type, the interpolation is the same type
      // else a float
      typedef typename core::If<typename Volume::value_type, float, core::IsFloatingType<typename Volume::value_type>::value >::type value_type;

   public:
      /**
       @brief This method must be called before any interpolation is made
       */
      void startInterpolation()
      {
         _interpolator.startInterpolation();
      }

      /**
       @brief This method must be called after any interpolation is made
       */
      void endInterpolation()
      {
         _interpolator.endInterpolation();
      }

      /**
       @brief Construct an interpolator for the volume v. 

       v must remain valid until the end of the calls to the interpolator
       */
      InterpolatorTriLinear( const VolumeType& v ) : _interpolator( v )
      {
      }

      /**
       @brief (x, y, z, PADDING) must be an index. It returns background if the point is outside the volume.
              pos must be 16bytes aligned
       */
      value_type operator()( const float* pos ) const
      {
         return _interpolator( pos );
      }

   protected:
      InterpolatorTriLinearDummy<Volume>  _interpolator;
   };
}
}

#endif
