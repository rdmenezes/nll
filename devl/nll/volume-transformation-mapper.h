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

#ifndef NLL_IMAGING_VOLUME_TRANSFORMATION_MAPPER_H_
# define NLL_IMAGING_VOLUME_MAPPER_H_

namespace nll
{
namespace imaging
{
   /**
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
      void startSlice( ui32 sliceId )
      {}

      // called after each slice
      void endSlice( ui32 sliceId )
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
       @param tfm a <target> to <resampled> affine transformation, the transformation is defined as source->target
       @param resampled the volume to map the coordinate from

       Basically, for each voxel of the resampled, it finds the corresponding voxel in the transformed target volume.
       it will call Processor( const DirectionalIterator& resampledPosition, const float* targetPosition )
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig, const VolumeSpatial<T, Storage>& target, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled ) const
      {
         typedef VolumeSpatial<T, Storage>   VolumeType;
         typedef core::Matrix<f32>  Matrix;

         if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
              !resampled.getSize()[ 0 ] || !resampled.getSize()[ 1 ] || !resampled.getSize()[ 2 ] )
         {
            return;
         }

         // compute the transformation target voxel -> resampled voxel
         Matrix transformation = target.getInvertedPst() * tfm.getAffineMatrix() * resampled.getPst();
         core::vector3f dx( transformation( 0, 0 ),
                            transformation( 1, 0 ),
                            transformation( 2, 0 ) );
         core::vector3f dy( transformation( 0, 1 ),
                            transformation( 1, 1 ),
                            transformation( 2, 1 ) );
         core::vector3f dz( transformation( 0, 2 ),
                            transformation( 1, 2 ),
                            transformation( 2, 2 ) );

         // compute the target origin with the tfm applied
         core::Matrix<float> targetOriginTfm;
         targetOriginTfm.clone( tfm.getAffineMatrix() );
         core::inverse( targetOriginTfm );
         targetOriginTfm = targetOriginTfm * target.getPst();
         core::vector3f targetOrigin2 = transf4( targetOriginTfm, core::vector3f( 0, 0, 0 ) );

         // create the transformation representing this displacement and compute the resampled origin in this
         // coordinate system
         Matrix g( 4, 4 );
         for ( ui32 y = 0; y < 3; ++y )
            for ( ui32 x = 0; x < 3; ++x )
               g( y, x ) = targetOriginTfm(y, x);
         g( 3, 3 ) = 1;
         g( 0, 3 ) = targetOrigin2[ 0 ];
         g( 1, 3 ) = targetOrigin2[ 1 ];
         g( 2, 3 ) = targetOrigin2[ 2 ];

         core::VolumeGeometry geom2( g );
         core::vector3f originInTarget = geom2.positionToIndex( resampled.getOrigin() );
         core::vector3f slicePosSrc = originInTarget;
         const int sizez = static_cast<int>( resampled.getSize()[ 2 ] );
         procOrig.start();

         #if !defined(NLL_NOT_MULTITHREADED)
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < sizez; ++z )
         {
            Processor proc = procOrig;
            proc.startSlice( z );

            typename VolumeType::DirectionalIterator  lineIt = resampled.getIterator( 0, 0, z );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + z * dz[ 0 ],
                                                        originInTarget[ 1 ] + z * dz[ 1 ],
                                                        originInTarget[ 2 ] + z * dz[ 2 ] );
            for ( ui32 y = 0; y < resampled.getSize()[ 1 ]; ++y )
            {
               typename VolumeType::DirectionalIterator  voxelIt = lineIt;
               
               NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
               { 
                  linePosSrc[ 0 ],
                  linePosSrc[ 1 ],
                  linePosSrc[ 2 ],
                  0
               };

               for ( ui32 x = 0; x < resampled.getSize()[ 0 ]; ++x )
               {
                  proc.process( voxelIt, voxelPosSrc );
                  //*voxelIt = interpolator( voxelPosSrc );

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
       @param target the <target> volume transformed by <tfm>
       @param tfm a <target> to <resampled> affine transformation
       @param resampled the volume to map the coordinate from

       Exactly the same version, except that it will call the Processor::process with position index instead of iterator
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig, const VolumeSpatial<T, Storage>& target, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled ) const
      {
         typedef VolumeSpatial<T, Storage>   VolumeType;
         typedef core::Matrix<f32>  Matrix;

         if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
              !resampled.getSize()[ 0 ] || !resampled.getSize()[ 1 ] || !resampled.getSize()[ 2 ] )
         {
            throw std::runtime_error( "invalid volume" );
         }

         // compute the transformation target voxel -> resampled voxel
         Matrix transformation = target.getInvertedPst() * tfm.getAffineMatrix() * resampled.getPst();
         core::vector3f dx( transformation( 0, 0 ),
                            transformation( 1, 0 ),
                            transformation( 2, 0 ) );
         core::vector3f dy( transformation( 0, 1 ),
                            transformation( 1, 1 ),
                            transformation( 2, 1 ) );
         core::vector3f dz( transformation( 0, 2 ),
                            transformation( 1, 2 ),
                            transformation( 2, 2 ) );

         // compute the target origin with the tfm applied
         core::Matrix<float> targetOriginTfm;
         targetOriginTfm.clone( tfm.getAffineMatrix() );
         core::inverse( targetOriginTfm );
         targetOriginTfm = targetOriginTfm * target.getPst();
         core::vector3f targetOrigin2 = transf4( targetOriginTfm, core::vector3f( 0, 0, 0 ) );

         // create the transformation representing this displacement and compute the resampled origin in this
         // coordinate system
         Matrix g( 4, 4 );
         for ( ui32 y = 0; y < 3; ++y )
            for ( ui32 x = 0; x < 3; ++x )
               g( y, x ) = targetOriginTfm(y, x);
         g( 3, 3 ) = 1;
         g( 0, 3 ) = targetOrigin2[ 0 ];
         g( 1, 3 ) = targetOrigin2[ 1 ];
         g( 2, 3 ) = targetOrigin2[ 2 ];

         core::VolumeGeometry geom2( g );
         core::vector3f originInTarget = geom2.positionToIndex( resampled.getOrigin() );
         core::vector3f slicePosSrc = originInTarget;
         const int sizez = static_cast<int>( resampled.getSize()[ 2 ] );
         procOrig.start();

         #if !defined(NLL_NOT_MULTITHREADED)
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < sizez; ++z )
         {
            Processor proc = procOrig;
            proc.startSlice( z );

            typename VolumeType::DirectionalIterator  lineIt = resampled.getIterator( 0, 0, z );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + z * dz[ 0 ],
                                                        originInTarget[ 1 ] + z * dz[ 1 ],
                                                        originInTarget[ 2 ] + z * dz[ 2 ] );
            for ( ui32 y = 0; y < resampled.getSize()[ 1 ]; ++y )
            {
               typename VolumeType::DirectionalIterator  voxelIt = lineIt;
               
               NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
               { 
                  linePosSrc[ 0 ],
                  linePosSrc[ 1 ],
                  linePosSrc[ 2 ],
                  0
               };

               for ( ui32 x = 0; x < resampled.getSize()[ 0 ]; ++x )
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