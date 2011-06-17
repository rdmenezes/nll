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

#ifndef NLL_IMAGING_VOLUME_RESAMPLE_H_
# define NLL_IMAGING_VOLUME_RESAMPLE_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Resample a target volume to an arbitrary source geometry
    @param target the volume that will be resampled
    @param source the volume into wich it will be resampled.
    @param tfm a transformation defined from source to target (easier to see if we transform first the source, and continue as if no transformation...)

    The source must already be allocated.
    */
   template <class T, class Storage, class Interpolator>
   void resampleVolume( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm )
   {
      typedef VolumeSpatial<T, Storage>   VolumeType;
      typedef core::Matrix<f32>  Matrix;

      if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
           !source.getSize()[ 0 ] || !source.getSize()[ 1 ] || !source.getSize()[ 2 ] )
      {
         throw std::runtime_error( "invalid volume" );
      }

      // compute the transformation target voxel -> source voxel
      Matrix transformation = source.getInvertedPst() *
                              tfm.getAffineMatrix() *
                              target.getPst();
      core::vector3f dx( transformation( 0, 0 ),
                         transformation( 1, 0 ),
                         transformation( 2, 0 ) );
      core::vector3f dy( transformation( 0, 1 ),
                         transformation( 1, 1 ),
                         transformation( 2, 1 ) );
      core::vector3f dz( transformation( 0, 2 ),
                         transformation( 1, 2 ),
                         transformation( 2, 2 ) );

      // we transform the origin (voxel index=(0, 0, 0)) to the correponding index in source
      core::vector3f originInTarget = transf4( transformation, core::vector3f( 0, 0, 0 ) );

      Interpolator interpolator( target );
      typename VolumeType::DirectionalIterator  sliceIt = source.getIterator( 0, 0, 0 );
      core::vector3f slicePosSrc = originInTarget;
      
      interpolator.startInterpolation();
      for ( ui32 z = 0; z < source.getSize()[ 2 ]; ++z )
      {
         typename VolumeType::DirectionalIterator  lineIt = sliceIt;
         core::vector3f linePosSrc = slicePosSrc;
         for ( ui32 y = 0; y < source.getSize()[ 1 ]; ++y )
         {
            typename VolumeType::DirectionalIterator  voxelIt = lineIt;
            
            NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
            { 
               linePosSrc[ 0 ],
               linePosSrc[ 1 ],
               linePosSrc[ 2 ],
               0
            };

            for ( ui32 x = 0; x < source.getSize()[ 0 ]; ++x )
            {
               *voxelIt = interpolator( voxelPosSrc );

               voxelPosSrc[ 0 ] += dx[ 0 ];
               voxelPosSrc[ 1 ] += dx[ 1 ];
               voxelPosSrc[ 2 ] += dx[ 2 ];
               voxelIt.addx();
            }
            linePosSrc += dy;
            lineIt.addy();
         }
         slicePosSrc += dz;
         sliceIt.addz();
      }
      interpolator.endInterpolation();
   }

   template <class T, class Storage, class Interpolator>
   void resampleVolume( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source )
   {
      typedef core::Matrix<f32>  Matrix;

      Matrix id = core::identityMatrix<Matrix>( 4 );
      resampleVolume<T, Storage, Interpolator>( target, source, TransformationAffine( id ) );
   }

   /**
    @ingroup imaging
    @brief Resample a target volume to an arbitrary source geometry. Use a default trilinear interpolation for resampling.

    The source must already be allocated.
    */
   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source )
   {
      resampleVolume<T, Storage, InterpolatorTriLinear< VolumeSpatial<T, Storage> > >( target, source );
   }

   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm )
   {
      resampleVolume<T, Storage, InterpolatorTriLinear< VolumeSpatial<T, Storage> > >( target, source, tfm );
   }

   /**
    @ingroup imaging
    @brief Resample a target volume to an arbitrary source geometry. Use a default nearest neighbour interpolation for resampling.

    The source must already be allocated.
    */
   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source )
   {
      resampleVolume<T, Storage, InterpolatorNearestNeighbour< VolumeSpatial<T, Storage> > >( target, source );
   }

   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm )
   {
      resampleVolume<T, Storage, InterpolatorNearestNeighbour< VolumeSpatial<T, Storage> > >( target, source, tfm );
   }
}
}

#endif
