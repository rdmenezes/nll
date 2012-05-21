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
         const Matrix tfm = ddfTfm.getAffineTfm();

         typedef ImageSpatial<T, Mapper, Alloc>   ImageType;

         if ( !target.size() || !resampled.size() )
         {
            return; // nothing to do
         }

         ensure( target.getNbComponents() == resampled.getNbComponents(), "must have the same number of dimensions" );

         // compute the transformation target voxel -> resampled voxel
         const Matrix transformation    = target.getInvertedPst() * tfm * resampled.getPst();
         const Matrix transformationDdf = ddfTfm.getInvertedPst() * tfm * resampled.getPst();
         core::vector2f dx( transformation( 0, 0 ),
                            transformation( 1, 0 ) );
         core::vector2f dy( transformation( 0, 1 ),
                            transformation( 1, 1 ) );
         core::vector2f dxddf( transformationDdf( 0, 0 ),
                               transformationDdf( 1, 0 ) );
         core::vector2f dyddf( transformationDdf( 0, 1 ),
                               transformationDdf( 1, 1 ) );

         // compute the target origin with the tfm applied
         core::Matrix<float> targetOriginTfm;
         core::Matrix<float> targetOriginTfmDdf;
         targetOriginTfm.clone( tfm );
         core::inverse( targetOriginTfm );
         targetOriginTfmDdf = targetOriginTfm * ddfTfm.getPst();  // beware // <targetOriginTfm> is reassigned!
         targetOriginTfm    = targetOriginTfm * target.getPst();
         core::vector2f targetOrigin2    = transf2d4( targetOriginTfm, core::vector2f( 0, 0 ) );
         core::vector2f targetOrigin2ddf = transf2d4( targetOriginTfmDdf, core::vector2f( 0, 0 ) );

         // create the transformation representing this displacement and compute the resampled origin in this
         // coordinate system
         Matrix g( 3, 3 );
         for ( ui32 y = 0; y < 2; ++y )
            for ( ui32 x = 0; x < 2; ++x )
               g( y, x ) = targetOriginTfm(y, x);
         g( 2, 2 ) = 1;
         g( 0, 2 ) = targetOrigin2[ 0 ];
         g( 1, 2 ) = targetOrigin2[ 1 ];

         Matrix gddf( 3, 3 );
         for ( ui32 y = 0; y < 2; ++y )
            for ( ui32 x = 0; x < 2; ++x )
               gddf( y, x ) = targetOriginTfm(y, x);
         gddf( 2, 2 ) = 1;
         gddf( 0, 2 ) = targetOrigin2ddf[ 0 ];
         gddf( 1, 2 ) = targetOrigin2ddf[ 1 ];

         core::VolumeGeometry2d geom2( g );
         core::VolumeGeometry2d geom2ddf( gddf );
         core::vector2f originInTarget    = geom2.positionToIndex( resampled.getOrigin() );
         core::vector2f originInTargetDdf = geom2ddf.positionToIndex( resampled.getOrigin() );

         const TransformationHelper transformationHelper( target.getInvertedPst() );

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
            core::vector2f linePosSrcDdf = core::vector2f( originInTargetDdf[ 0 ] + y * dyddf[ 0 ],
                                                           originInTargetDdf[ 1 ] + y * dyddf[ 1 ] );

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

               linePosSrcDdf[ 0 ] += dxddf[ 0 ];
               linePosSrcDdf[ 1 ] += dxddf[ 1 ];
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
