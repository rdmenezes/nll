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

#ifndef NLL_IMAGING_VOLUME_TRANSFORMATION_MAPPER_H_
# define NLL_IMAGING_VOLUME_MAPPER_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Outline of a basic processor, just for illustration as it doesn't do anything
    */
   template <class Volume>
   class VolumeTransformationProcessor
   {
   public:
      typedef typename Volume::DirectionalIterator          DirectionalIterator;
      typedef typename Volume::ConstDirectionalIterator     ConstDirectionalIterator;

   public:
      VolumeTransformationProcessor( Volume& source, Volume& target ) : _source( source ), _target( target )
      {}

      // called as soon as the volume mapper started the mapping process
      void start()
      {}

      // called for each slice
      void startSlice( size_t sliceId )
      {}

      // called after each slice
      void endSlice( size_t sliceId )
      {}

      // called as soon as the volume mapper ended the mapping process
      void end()
      {}

      // called everytime a new voxel in the target volume is reached
      // sourcePosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( const DirectionalIterator& targetPosition, const float* sourcePosition )
      {
      }

      // called everytime a new voxel in the target volume is reached
      // sourcePosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( const core::vector3ui& targetPosition, const float* sourcePosition )
      {
      }

   private:
      Volume&  _source;
      Volume&  _target;
   };

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
    */
   class VolumeTransformationMapper
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

       it will call Processor( const DirectionalIterator& resampledPosition, const float* targetPosition )
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig, const VolumeSpatial<T, Storage>& target, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled ) const
      {
         run( procOrig, target, tfm, resampled, core::vector3ui( 0, 0, 0 ), resampled.getSize() );
      }

      /**
       @brief Transformation mapper on a subset of the volume
       @param start the begining of the subset
       @param end the end of the subset, NON inclusive
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig,
                const VolumeSpatial<T, Storage>& target,
                const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled,
                const core::vector3ui& start,
                const core::vector3ui& end ) const
      {
         typedef VolumeSpatial<T, Storage>   VolumeType;
         typedef core::Matrix<f32>  Matrix;

         if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
              !resampled.getSize()[ 0 ] || !resampled.getSize()[ 1 ] || !resampled.getSize()[ 2 ] )
         {
            return;
         }

         ensure( start[ 0 ] <= end[ 0 ], "wrong arg!" );
         ensure( start[ 1 ] <= end[ 1 ], "wrong arg!" );
         ensure( start[ 2 ] <= end[ 2 ], "wrong arg!" );

         ensure( end[ 0 ] <= resampled.getSize()[ 0 ], "wrong arg!" );
         ensure( end[ 1 ] <= resampled.getSize()[ 1 ], "wrong arg!" );
         ensure( end[ 2 ] <= resampled.getSize()[ 2 ], "wrong arg!" );

         // (1) compute the transformation index target->position MM with affine target->source TFM applied
         core::Matrix<float> targetOriginTfm = tfm.getInvertedAffineMatrix() * target.getPst();

         // compute the origin of the resampled in the geometric space (1)
         const bool success = core::inverse( targetOriginTfm );
         ensure( success, "not affine!" );
         const core::vector3f originInTarget = core::transf4( targetOriginTfm, resampled.getOrigin() );

         // finally get the axis direction resampled voxel -> target voxel
         const core::Matrix<float> orientation = targetOriginTfm * resampled.getPst();
         const core::vector3f dx( orientation( 0, 0 ),
                                  orientation( 1, 0 ),
                                  orientation( 2, 0 ) );
         const core::vector3f dy( orientation( 0, 1 ),
                                  orientation( 1, 1 ),
                                  orientation( 2, 1 ) );
         const core::vector3f dz( orientation( 0, 2 ),
                                  orientation( 1, 2 ),
                                  orientation( 2, 2 ) );

         // now fast resampling loop
         const int sizez = static_cast<int>( end[ 2 ] );
         const int startz = static_cast<int>( start[ 2 ] );
         procOrig.start();

         #if !defined(NLL_NOT_MULTITHREADED)
         # pragma omp parallel for
         #endif
         for ( int z = startz; z < sizez; ++z )
         {
            Processor proc = procOrig;
            proc.startSlice( z );

            typename VolumeType::DirectionalIterator  lineIt = resampled.getIterator( 0, 0, z );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + z * dz[ 0 ],
                                                        originInTarget[ 1 ] + z * dz[ 1 ],
                                                        originInTarget[ 2 ] + z * dz[ 2 ] );
            for ( size_t y = start[ 1 ]; y < end[ 1 ]; ++y )
            {
               typename VolumeType::DirectionalIterator  voxelIt = lineIt;
               
               NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
               { 
                  linePosSrc[ 0 ],
                  linePosSrc[ 1 ],
                  linePosSrc[ 2 ],
                  0
               };

               for ( size_t x = start[ 0 ]; x < end[ 0 ]; ++x )
               {
                  proc.process( voxelIt, voxelPosSrc );

                  voxelPosSrc[ 0 ] += dx[ 0 ];
                  voxelPosSrc[ 1 ] += dx[ 1 ];
                  voxelPosSrc[ 2 ] += dx[ 2 ];
                  voxelIt.addx();
               }
               linePosSrc += dy;
               lineIt.addy();
            }
            proc.endSlice( z );
         }
         procOrig.end();
      }
   };

   /**
    @ingroup imaging
    @brief Efficiently iterate on all resampled voxels and compute the corresponding position in the transformed
           target volume

    This version of the mapper will use coordinate on the resampled rather than an iterator.

    //
    //
    BEWARE: the input processor will be local to each slice (each slice will have its own processor object),
    this is because when multithreaded we want to have local objects (else huge drop in performance x6)

    => processors must be thread safe! INCLUDING //// operator=  ////
    //
    //

    start() and end() methods will be called on the original processor
    startSlice(), endSlice(), process() will be called on the local processor
    */
   class VolumeTransformationMapperPosition
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

       it will call Processor( const DirectionalIterator& resampledPosition, const float* targetPosition )
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig, const VolumeSpatial<T, Storage>& target, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled ) const
      {
         run( procOrig, target, tfm, resampled, core::vector3ui( 0, 0, 0 ), resampled.getSize() );
      }

      /**
       @brief Transformation mapper on a subset of the volume
       @param start the begining of the subset
       @param end the end of the subset, NON inclusive
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig,
                const VolumeSpatial<T, Storage>& target,
                const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled,
                const core::vector3ui& start,
                const core::vector3ui& end ) const
      {
         typedef VolumeSpatial<T, Storage>   VolumeType;
         typedef core::Matrix<f32>  Matrix;

         if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
              !resampled.getSize()[ 0 ] || !resampled.getSize()[ 1 ] || !resampled.getSize()[ 2 ] )
         {
            return;
         }

         ensure( start[ 0 ] <= end[ 0 ], "wrong arg!" );
         ensure( start[ 1 ] <= end[ 1 ], "wrong arg!" );
         ensure( start[ 2 ] <= end[ 2 ], "wrong arg!" );

         ensure( end[ 0 ] <= resampled.getSize()[ 0 ], "wrong arg!" );
         ensure( end[ 1 ] <= resampled.getSize()[ 1 ], "wrong arg!" );
         ensure( end[ 2 ] <= resampled.getSize()[ 2 ], "wrong arg!" );

         // (1) compute the transformation index target->position MM with affine target->source TFM applied
         core::Matrix<float> targetOriginTfm = tfm.getInvertedAffineMatrix() * target.getPst();

         // compute the origin of the resampled in the geometric space (1)
         const bool success = core::inverse( targetOriginTfm );
         ensure( success, "not affine!" );
         const core::vector3f originInTarget = core::transf4( targetOriginTfm, resampled.getOrigin() );

         // finally get the axis direction resampled voxel -> target voxel
         const core::Matrix<float> orientation = targetOriginTfm * resampled.getPst();
         const core::vector3f dx( orientation( 0, 0 ),
                                  orientation( 1, 0 ),
                                  orientation( 2, 0 ) );
         const core::vector3f dy( orientation( 0, 1 ),
                                  orientation( 1, 1 ),
                                  orientation( 2, 1 ) );
         const core::vector3f dz( orientation( 0, 2 ),
                                  orientation( 1, 2 ),
                                  orientation( 2, 2 ) );

         // now fast resampling loop
         const int sizez = static_cast<int>( end[ 2 ] );
         const int startz = static_cast<int>( start[ 2 ] );
         procOrig.start();

         #if !defined(NLL_NOT_MULTITHREADED)
         # pragma omp parallel for
         #endif
         for ( int z = startz; z < sizez; ++z )
         {
            Processor proc = procOrig;
            proc.startSlice( z );

            typename VolumeType::DirectionalIterator  lineIt = resampled.getIterator( 0, 0, z );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + z * dz[ 0 ],
                                                        originInTarget[ 1 ] + z * dz[ 1 ],
                                                        originInTarget[ 2 ] + z * dz[ 2 ] );
            for ( size_t y = start[ 1 ]; y < end[ 1 ]; ++y )
            {
               typename VolumeType::DirectionalIterator  voxelIt = lineIt;
               
               NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
               { 
                  linePosSrc[ 0 ],
                  linePosSrc[ 1 ],
                  linePosSrc[ 2 ],
                  0
               };

               for ( size_t x = start[ 0 ]; x < end[ 0 ]; ++x )
               {
                  proc.process( core::vector3ui( x, y, z ), voxelPosSrc );

                  voxelPosSrc[ 0 ] += dx[ 0 ];
                  voxelPosSrc[ 1 ] += dx[ 1 ];
                  voxelPosSrc[ 2 ] += dx[ 2 ];
                  voxelIt.addx();
               }
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
