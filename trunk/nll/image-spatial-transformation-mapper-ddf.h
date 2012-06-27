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

#ifndef NLL_CORE_IMAGE_SPATIAL_TRANSFORMATION_MAPPER_DDF_H
# define NLL_CORE_IMAGE_SPATIAL_TRANSFORMATION_MAPPER_DDF_H

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Efficient mapping of the resampled (usually 'source') pixels to target voxels using a DDF as transformation

    The transformation can be seen as follow:

    For the affine part:
    - compute invert(tfm) so we have a target->source transform
    - compose the invert(tfm) * TargetPst so we have a index->MM transform (2) (i.e., as we don't have the "source space", so we equivalently move the target by inv(affine))
    - compute the resampled origin in the space (2)
    - finally compute the vector director using (2) and the resampled PST

    Similarly for the deformable part, except that we retrieve a deformable
    displacement in the DDF that we add to the affine displacement

    CAUTION: Internally, the mapper will use different threads. So for maximal efficiency, new processors
    will be instanciated for each line from the initial processor. Typically the start/end methods will be called
    on the original processor (this is to allow reduction if necessary), while the process method will be called
    on the replicated processors.

    Processors MUST be threadsafe
    */
   class ImageTransformationMapperDdf
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

       The deformable case is slighly more complex than the affine case as we need to retrieve a displacement
       in the DDF to add to the lookup target index

       @TODO BUG: the DDF returns a displacement in MM!!! So we can't add it directly to the target index lookup
       */
      template <class Processor, class T, class Mapper, class Alloc>
      void run( Processor& procOrig, const ImageSpatial<T, Mapper, Alloc>& target,
               const DeformableTransformationDenseDisplacementField2d& ddfTfm,
               ImageSpatial<T, Mapper, Alloc>& resampled )
      {
         typedef ImageSpatial<T, Mapper, Alloc>   ImageType;

         if ( !target.size() || !resampled.size() )
         {
            return; // nothing to do
         }
         ensure( target.getNbComponents() == resampled.getNbComponents(), "must have the same number of dimensions" );

         const TransformationHelper transformationHelper( target.getInvertedPst() );

         // (1) compute the transformation index target->position MM with affine target->source TFM applied
         core::Matrix<float> targetOriginTfm = ddfTfm.getAffineInvTfm() * target.getPst();
         core::Matrix<float> targetOriginTfmDdf = ddfTfm.getAffineInvTfm() * ddfTfm.getStorage().getPst();

         // compute the origin of the resampled in the geometric space (1)
         const bool success = core::inverse( targetOriginTfm );
         ensure( success, "not affine!" );
         const bool successDdf = core::inverse( targetOriginTfmDdf );
         ensure( successDdf, "Ddf not affine!" );
         const core::vector2f originInTarget = core::transf3( targetOriginTfm, resampled.getOrigin() );
         const core::vector2f originInTargetDdf = core::transf3( targetOriginTfmDdf, resampled.getOrigin() );

         // finally get the axis direction resampled voxel -> target voxel
         const core::Matrix<float> orientation = targetOriginTfm * resampled.getPst();
         const core::Matrix<float> orientationDdf = targetOriginTfmDdf * resampled.getPst();
         const core::vector2f    dx( orientation( 0, 0 ),
                                     orientation( 1, 0 ) );
         const core::vector2f dxDdf( orientationDdf( 0, 0 ),
                                     orientationDdf( 1, 0 ) );
         const core::vector2f    dy( orientation( 0, 1 ),
                                     orientation( 1, 1 ) );
         const core::vector2f dyDdf( orientationDdf( 0, 1 ),
                                     orientationDdf( 1, 1 ) );
      
         procOrig.start();
       
         #if !defined(NLL_NOT_MULTITHREADED) && !defined(NLL_NOT_MULTITHREADED_FOR_QUICK_OPERATIONS)
         # pragma omp parallel for
         #endif
         for ( int y = 0; y < (int)resampled.sizey(); ++y )
         {
            Processor proc = procOrig;
            typename ImageType::DirectionalIterator  lineIt = resampled.getIterator( 0, y, 0 );
            core::vector2f linePosSrc = core::vector2f( originInTarget[ 0 ] + y * dy[ 0 ],
                                                        originInTarget[ 1 ] + y * dy[ 1 ] );
            core::vector2f linePosSrcDdf = core::vector2f( originInTargetDdf[ 0 ] + y * dyDdf[ 0 ],
                                                           originInTargetDdf[ 1 ] + y * dyDdf[ 1 ] );

            typename ImageType::DirectionalIterator  voxelIt = lineIt;
            for ( ui32 x = 0; x < resampled.sizex(); ++x )
            {
               // compute the displacement and add it to the target index
               core::vector2f displacement = ddfTfm.getDeformableDisplacementOnlyIndex( linePosSrcDdf );
               transformationHelper.transform( displacement ); // transform the displacement in MM into a corresponding displacement in index in the target space
               displacement[ 0 ] += linePosSrc[ 0 ];
               displacement[ 1 ] += linePosSrc[ 1 ];

               proc.process( voxelIt, &displacement[ 0 ] );
               linePosSrc[ 0 ] += dx[ 0 ];
               linePosSrc[ 1 ] += dx[ 1 ];

               linePosSrcDdf[ 0 ] += dxDdf[ 0 ];
               linePosSrcDdf[ 1 ] += dxDdf[ 1 ];
               voxelIt.addx();
            }
            lineIt.addy();
         }
         procOrig.end();
      }

   private:
      /**
       @brief Helper class that determine how the deformable displacement should be handled

       In the case of targetPstInv is a scaling and/or translation matrix only (no rotation/shearing) computations can be significantly simplified

       We use it in this context:
       - we have a target index mapped (i.e., without the DDF, the resampled volume at this position would use this target index)
       - we have a deformable displacement in MM that we need to translate the target index
       -> i.e., we need to transform this displacement in MM into the corresponding index displacement
       */
      class TransformationHelper
      {
      public:
         TransformationHelper( const Matrix& targetPstInv ) : _targetPstInv( targetPstInv ), _isMatrixScalingTranslationOnly( core::isScalingTranslationMatrixOnly( targetPstInv ) )
         {
            // remove the translation part as this has already been taken care of...
            _targetPstInv.clone( targetPstInv );
            for ( ui32 n = 0; n + 1 < targetPstInv.sizey(); ++n )
            {
               _targetPstInv( n, targetPstInv.sizex() - 1 ) = 0;
            }

            _scaling = core::getSpacing3x3( targetPstInv );
         }

         void transform( core::vector2f& displacementMm ) const
         {
            // we have the deformable displacement in MM
            // what is the corresponding displacement in index in the target?

            // if the target PST has no rotation/shearing, easy, just apply the spacing
            // else, apply the affine part of the transformation (we don't care about the translation, it is already in the displacement)
            if ( _isMatrixScalingTranslationOnly )
            {
               displacementMm[ 0 ] *= _scaling[ 0 ];
               displacementMm[ 1 ] *= _scaling[ 1 ];
            } else {
               displacementMm = core::transf3( _targetPstInv, displacementMm );
            }
         }

      private:
         Matrix         _targetPstInv;
         bool           _isMatrixScalingTranslationOnly;
         core::vector2f _scaling;
      };
   };
}
}

#endif
