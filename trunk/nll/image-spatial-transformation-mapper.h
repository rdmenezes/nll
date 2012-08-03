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

#ifndef NLL_CORE_IMAGE_SPATIAL_TRANSFORMATION_MAPPER_H
# define NLL_CORE_IMAGE_SPATIAL_TRANSFORMATION_MAPPER_H

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Outline of a basic processor, just for illustration as it doesn't do anything

    CAUTION: all processor must be threadsafe! Typically, for each line, a new processor will
    be instanciated from the one given at the transformationMapper
    (as for the imaging::VolumeTransformationMapper)
    */
   template <class ImageSpatial>
   class ImageTransformationProcessor
   {
   public:
      typedef typename ImageSpatial::DirectionalIterator          DirectionalIterator;
      typedef typename ImageSpatial::ConstDirectionalIterator     ConstDirectionalIterator;

   public:
      ImageTransformationProcessor( ImageSpatial& source, ImageSpatial& target ) : _source( source ), _target( target )
      {}

      // called as soon as the volume mapper started the mapping process
      void start()
      {}

      // called as soon as the volume mapper ended the mapping process
      void end()
      {}

      // called everytime a new voxel in the target volume is reached
      // sourcePosition contains 2 values: [x, y ]
      void process( const DirectionalIterator& targetPosition, const float* sourcePosition )
      {
      }

   private:
      ImageSpatial&  _source;
      ImageSpatial&  _target;
   };

   /**
    @ingroup core
    @brief Efficient mapping of the target pixels to source voxels

    For each pixel of the target, apply the tfm on the source volume and find the corresponding source pixel

    CAUTION: Internally, the mapper will use different threads. So for maximal efficiency, new processors
    will be instanciated for each line from the initial processor. Typically the start/end methods will be called
    on the original processor (this is to allow reduction if necessary), while the process method will be called
    on the replicated processors.

    Processors MUST be threadsafe
    */
   class ImageTransformationMapper
   {
   public:
      typedef core::Matrix<float>   Matrix;

      /**
       @brief Map a transformed target coordinate system to a <resampled> coordinate system
       @param target the <target> volume
       @param tfm an affine transformattion defined as <source> to <target>, inv(tfm) will be applied on the target volume
       @param resampled the volume to map the coordinate from

       Basically, for each voxel of the resampled, it finds the corresponding pixels in the transformed target volume.

       A typical use case is:
       - compute registration between source and target. It will be returned the a source->target matrix
       - resample the moving volume (target) in the source geometry with correct registraton
       */
      template <class Processor, class T, class Mapper, class Alloc>
      void run( Processor& procOrig, const ImageSpatial<T, Mapper, Alloc>& target, const Matrix& tfm, ImageSpatial<T, Mapper, Alloc>& resampled )
      {
         
         typedef ImageSpatial<T, Mapper, Alloc>   ImageType;

         if ( !target.size() || !resampled.size() )
         {
            return; // nothing to do
         }

         Matrix invTfm;
         invTfm.clone( tfm );
         const bool success0 = core::inverse( invTfm );
         ensure( success0, "not affine tfm!" );

         ensure( target.getNbComponents() == resampled.getNbComponents(), "must have the same number of dimensions" );

         // (1) compute the transformation index target->position MM with affine target->source TFM applied
         core::Matrix<float> targetOriginTfm = invTfm * target.getPst();

         // compute the origin of the resampled in the geometric space (1)
         const bool success = core::inverse( targetOriginTfm );
         ensure( success, "not affine!" );
         const core::vector2f originInTarget = transf2d4( targetOriginTfm, resampled.getOrigin() );

         // finally get the axis direction resampled voxel -> target voxel
         const core::Matrix<float> orientation = targetOriginTfm * resampled.getPst();
         const core::vector2f dx( orientation( 0, 0 ),
                                  orientation( 1, 0 ) );
         const core::vector2f dy( orientation( 0, 1 ),
                                  orientation( 1, 1 ) );
         procOrig.start();
       
         #if !defined(NLL_NOT_MULTITHREADED) && !defined(NLL_NOT_MULTITHREADED_FOR_QUICK_OPERATIONS)
         # pragma omp parallel for
         #endif
         for ( int y = 0; y < (int)resampled.sizey(); ++y )
         {
            Processor proc = procOrig;
            typename ImageType::DirectionalIterator  lineIt = resampled.getIterator( 0, y, 0 );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + y * dy[ 0 ],
                                                        originInTarget[ 1 ] + y * dy[ 1 ],
                                                        0 );
            typename ImageType::DirectionalIterator  voxelIt = lineIt;
            for ( size_t x = 0; x < resampled.sizex(); ++x )
            {
               proc.process( voxelIt, &linePosSrc[0] );
               linePosSrc[ 0 ] += dx[ 0 ];
               linePosSrc[ 1 ] += dx[ 1 ];
               voxelIt.addx();
            }
            lineIt.addy();
         }
         procOrig.end();
      }

   private:
      template <class T, class Mapper, class Allocator, class Vector>
      static Vector transf2d4( const core::Matrix<T, Mapper, Allocator>& m, const Vector& v )
      {
         assert( m.sizex() == 3 && m.sizey() == 3 );
         return Vector( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + m( 0, 2 ),
                        v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + m( 1, 2 ) );
      }
   };
}
}

#endif