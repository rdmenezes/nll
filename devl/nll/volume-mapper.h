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

#ifndef NLL_IMAGING_VOLUME_MAPPER_H_
# define NLL_IMAGING_VOLUME_MAPPER_H_

namespace nll
{
namespace imaging
{
   /**
    @brief Outline of a basic processor, just for illustration as it doesn't do anything
    */
   template <class Volume>
   class Processor
   {
   public:
      typedef typename Volume::DirectionalIterator          DirectionalIterator;
      typedef typename Volume::ConstDirectionalIterator     ConstDirectionalIterator;

   public:
      Processor( Volume& source, Volume& target ) : _source( source ), _target( target )
      {}

      // called as soon as the volume mapper started the mapping process
      void start()
      {}

      // called as soon as the volume mapper ended the mapping process
      void end()
      {}

      // called everytime a new voxel in the target volume is reached
      // sourcePosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( const DirectionalIterator& targetPosition, const float* sourcePosition )
      {
      }

   private:
      Volume&  _source;
      Volume&  _target;
   };

   class VolumeMapper
   {
   public:
      /**
       @brief Map a target coordinate system to a source transformed coordinate system
       @param source the <source> volume transformed by <tfm>
       @param tfm a <source> to <target> affine transformation
       @param target the volume to map the coordinate from

       Basically, for each voxel of the target, it finds the corresponding in the transformed source volume.
       */
      template <class Processor, class T, class Storage>
      void run( Processor& proc, const VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& target )
      {
         typedef VolumeSpatial<T, Storage>   VolumeType;
         typedef core::Matrix<f32>  Matrix;

         if ( !source.getSize()[ 0 ] || !source.getSize()[ 1 ] || !source.getSize()[ 2 ] ||
              !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] )
         {
            throw std::runtime_error( "invalid volume" );
         }

         // compute the transformation source voxel -> target voxel
         Matrix transformation = source.getInvertedPst() * tfm.getAffineMatrix() * target.getPst();
         core::vector3f dx( transformation( 0, 0 ),
                            transformation( 1, 0 ),
                            transformation( 2, 0 ) );
         core::vector3f dy( transformation( 0, 1 ),
                            transformation( 1, 1 ),
                            transformation( 2, 1 ) );
         core::vector3f dz( transformation( 0, 2 ),
                            transformation( 1, 2 ),
                            transformation( 2, 2 ) );

         // compute the source origin with the tfm applied
         core::Matrix<float> sourceOriginTfm;
         sourceOriginTfm.clone( tfm.getAffineMatrix() );
         core::inverse( sourceOriginTfm );
         sourceOriginTfm = sourceOriginTfm * source.getPst();
         core::vector3f sourceOrigin2 = transf4( sourceOriginTfm, core::vector3f( 0, 0, 0 ) );

         // create the transformation representing this displacement and compute the target origin in this
         // coordinate system
         Matrix g( 4, 4 );
         for ( ui32 y = 0; y < 3; ++y )
            for ( ui32 x = 0; x < 3; ++x )
               g( y, x ) = sourceOriginTfm(y, x);
         g( 3, 3 ) = 1;
         g( 0, 3 ) = sourceOrigin2[ 0 ];
         g( 1, 3 ) = sourceOrigin2[ 1 ];
         g( 2, 3 ) = sourceOrigin2[ 2 ];

         core::VolumeGeometry geom2( g );
         core::vector3f originInTarget = geom2.positionToIndex( target.getOrigin() );
         core::vector3f slicePosSrc = originInTarget;
         const int sizez = static_cast<int>( target.getSize()[ 2 ] );
         proc.start();
         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < sizez; ++z )
         {
            typename VolumeType::DirectionalIterator  lineIt = target.getIterator( 0, 0, z );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + z * dz[ 0 ],
                                                        originInTarget[ 1 ] + z * dz[ 1 ],
                                                        originInTarget[ 2 ] + z * dz[ 2 ] );
            for ( ui32 y = 0; y < target.getSize()[ 1 ]; ++y )
            {
               typename VolumeType::DirectionalIterator  voxelIt = lineIt;
               
               NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
               { 
                  linePosSrc[ 0 ],
                  linePosSrc[ 1 ],
                  linePosSrc[ 2 ],
                  0
               };

               for ( ui32 x = 0; x < target.getSize()[ 0 ]; ++x )
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
         }
         proc.end();
      }
   };
}
}

#endif