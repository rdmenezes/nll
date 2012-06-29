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

#ifndef NLL_IMAGING_VOLUME_TRANSFORMATION_MAPPER_DDF_H_
# define NLL_IMAGING_VOLUME_TRANSFORMATION_MAPPER_DDF_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Efficiently iterate on all resampled voxels and compute the corresponding position in the transformed
           target volume

    //
    //
    BEWARE: the input processor will be local to each slice (each slice will have its own processor object),
    this is because when multithreaded we want to have local objects (else huge drop in performance x6)

    => processors must be thread safe! INCLUDING //// operator=  ////
    //
    //

    start() and end() methods will be called on the original processor
    startSlice(), endSlice(), process() will be called on the local processor

    @note on average, it is about 3 times slower than the pure affine transformation mapper.
    */
   class VolumeTransformationMapperDdf
   {
   public:
      /**
       @brief Map a resampled coordinate system to a target transformed coordinate system
       @param target the <target> volume transformed by inverse of <tfm>
       @param tfm a <source> to <target> affine transformation, consequently, the target volume will be moved by the inverse of <tfm>
       @param resampled the volume to map the coordinate from

       The transformation can be seen as follow:
	    - compute invert(tfm) so we have a target->source transform
	    - compose the invert(tfm) * TargetPst so we have a index->MM transform (2) (i.e., as we don't have the "source space", so we equivalently move the target by inv(affine))
	    - compute the resampled origin in the space (2)
	    - finally compute the vector director using (2) and the resampled PST
       Then for each voxel in resampled space, find its equivalent index in the target volume.

       We are doing exactly the same thing for the DDF volume

       it will call Processor( const DirectionalIterator& resampledPosition, const float* targetPosition )
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig, const VolumeSpatial<T, Storage>& target, const TransformationDenseDeformableField& tfm, VolumeSpatial<T, Storage>& resampled ) const
      {
         typedef VolumeSpatial<T, Storage>   VolumeType;
         typedef core::Matrix<f32>  Matrix;

         if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
              !resampled.getSize()[ 0 ] || !resampled.getSize()[ 1 ] || !resampled.getSize()[ 2 ] )
         {
            return;
         }

         const impl::TransformationHelper transformationHelper( target.getInvertedPst() );

         // (1) compute the transformation index target->position MM with affine target->source TFM applied
         core::Matrix<float> targetOriginTfm = tfm.getInvertedAffineMatrix() * target.getPst();
         core::Matrix<float> targetOriginTfmDdf = tfm.getInvertedAffineMatrix() * tfm.getStorage().getPst();

         // compute the origin of the resampled in the geometric space (1)
         const bool success = core::inverse( targetOriginTfm );
         ensure( success, "not affine!" );
         const bool successDdf = core::inverse( targetOriginTfmDdf );
         ensure( successDdf, "Ddf not affine!" );
         const core::vector3f originInTarget = core::transf4( targetOriginTfm, resampled.getOrigin() );
         const core::vector3f originInTargetDdf = core::transf4( targetOriginTfmDdf, resampled.getOrigin() );

         // finally get the axis direction resampled voxel -> target voxel
         const core::Matrix<float> orientation = targetOriginTfm * resampled.getPst();
         const core::Matrix<float> orientationDdf = targetOriginTfmDdf * resampled.getPst();
         const core::vector3f    dx( orientation( 0, 0 ),
                                     orientation( 1, 0 ),
                                     orientation( 2, 0 ) );
         const core::vector3f dxDdf( orientationDdf( 0, 0 ),
                                     orientationDdf( 1, 0 ),
                                     orientationDdf( 2, 0 ) );
         const core::vector3f    dy( orientation( 0, 1 ),
                                     orientation( 1, 1 ),
                                     orientation( 2, 1 ) );
         const core::vector3f dyDdf( orientationDdf( 0, 1 ),
                                     orientationDdf( 1, 1 ),
                                     orientationDdf( 2, 1 ) );
         const core::vector3f    dz( orientation( 0, 2 ),
                                     orientation( 1, 2 ),
                                     orientation( 2, 2 ) );
         const core::vector3f dzDdf( orientationDdf( 0, 2 ),
                                     orientationDdf( 1, 2 ),
                                     orientationDdf( 2, 2 ) );

         // now fast resampling loop
         const int sizez = static_cast<int>( resampled.getSize()[ 2 ] );
         procOrig.start();

         #if !defined(NLL_NOT_MULTITHREADED)
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < sizez; ++z )
         {
            Processor proc = procOrig;
            proc.startSlice( z );

            NLL_ALIGN_16 float position[ 4 ];

            typename VolumeType::DirectionalIterator  lineIt = resampled.getIterator( 0, 0, z );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + z * dz[ 0 ],
                                                        originInTarget[ 1 ] + z * dz[ 1 ],
                                                        originInTarget[ 2 ] + z * dz[ 2 ] );
            core::vector3f linePosSrcDdf = core::vector3f( originInTargetDdf[ 0 ] + z * dzDdf[ 0 ],
                                                           originInTargetDdf[ 1 ] + z * dzDdf[ 1 ],
                                                           originInTargetDdf[ 2 ] + z * dzDdf[ 2 ] );
            for ( size_t y = 0; y < resampled.sizey(); ++y )
            {
               typename VolumeType::DirectionalIterator  voxelIt = lineIt;
               
               core::vector3f voxelPosSrcDdf = linePosSrcDdf;
               core::vector3f voxelPosSrc = linePosSrc;
               for ( size_t x = 0; x < resampled.sizex(); ++x )
               {
                  core::vector3f displacement = tfm.transformDeformableOnlyIndex( voxelPosSrcDdf ); // this is the displacement in MM
                  transformationHelper.transform( displacement ); // translate the displacement in MM into the corresponding target index

                  position[ 0 ] = displacement[ 0 ] + voxelPosSrc[ 0 ];
                  position[ 1 ] = displacement[ 1 ] + voxelPosSrc[ 1 ];
                  position[ 2 ] = displacement[ 2 ] + voxelPosSrc[ 2 ];
                  proc.process( voxelIt, position );

                  voxelPosSrc += dx;
                  voxelPosSrcDdf += dxDdf;
                  voxelIt.addx();
               }
               linePosSrcDdf += dyDdf;
               linePosSrc += dy;
               lineIt.addy();
            }
            proc.endSlice( z );
         }
         procOrig.end();
      }
   };
}
}

#endif
