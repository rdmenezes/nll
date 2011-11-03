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

#ifndef NLL_CORE_IMAGE_SPATIAL_RESAMPLING_H
# define NLL_CORE_IMAGE_SPATIAL_RESAMPLING_H

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Processor used for resampling
    */
   template <class ImageSpatial, class Interpolator>
   class ImageTransformationProcessorResampler
   {
   public:
      typedef typename ImageSpatial::DirectionalIterator          DirectionalIterator;
      typedef typename ImageSpatial::ConstDirectionalIterator     ConstDirectionalIterator;

   public:
      ImageTransformationProcessorResampler( const ImageSpatial& target, ImageSpatial& resampled ) : _target( &target ), _resampled( &resampled ), _interpolator( target )
      {}

      // called as soon as the volume mapper started the mapping process
      void start()
      {}

      // called as soon as the volume mapper ended the mapping process
      void end()
      {}

      // called everytime a new voxel in the resampled volume is reached
      // targetPosition contains 2 values: [x, y ]
      void process( const DirectionalIterator& resampledPos, const float* targetPosition )
      {
         // it is guaranteed the colors are contiguous
         _interpolator.interpolateValues( targetPosition[ 0 ], targetPosition[ 1 ], &(*resampledPos) );
      }

   private:
      const ImageSpatial*  _target;
      ImageSpatial*        _resampled;
      Interpolator         _interpolator;
   };

   /**
    @ingroup core
    @brief Resample a target volume transformed with a tfm source->target to a resampled geometry
    */
   template <class T, class Mapper, class Alloc, class Interpolator>
   void resample( const ImageSpatial<T, Mapper, Alloc>& target, const Matrix<float>& tfm, ImageSpatial<T, Mapper, Alloc>& resampled )
   {
      typedef core::ImageSpatial<T, Mapper, Alloc> ImageSpacialT;
      core::ImageTransformationProcessorResampler<ImageSpacialT, Interpolator> proc( target, resampled );
      core::ImageTransformationMapper mapper;
      mapper.run( proc, target, tfm, resampled );
   }

   /**
    @ingroup core
    @brief Resample a target volume transformed with a tfm source->target to a resampled geometry
    */
   template <class T, class Mapper, class Alloc>
   void resampleNearest( const ImageSpatial<T, Mapper, Alloc>& target, const Matrix<float>& tfm, ImageSpatial<T, Mapper, Alloc>& resampled )
   {
      resample< T, Mapper, Alloc, InterpolatorNearestNeighbor2D<T, Mapper, Alloc> >( target, tfm, resampled );
   }

   /**
    @ingroup core
    @brief Resample a target volume transformed with a tfm source->target to a resampled geometry
    */
   template <class T, class Mapper, class Alloc>
   void resampleLinear( const ImageSpatial<T, Mapper, Alloc>& target, const Matrix<float>& tfm, ImageSpatial<T, Mapper, Alloc>& resampled )
   {
      resample< T, Mapper, Alloc, InterpolatorLinear2D<T, Mapper, Alloc> >( target, tfm, resampled );
   }

   /**
    @brief Resample a 2D spatial image using a 2 DDF deformable transformation
    @param target the target image to map from
    @param ddf a transformation defined source->target applied to the target image (so internally the affine TFM will be inverted before applying it to the target)
    @param source the image to map to
    */
   template <class T, class Mapper, class Allocator>
   void resampleLinear( const ImageSpatial<T, Mapper, Allocator>& target, const DeformableTransformationDenseDisplacementField2d& ddf, ImageSpatial<T, Mapper, Allocator>& source )
   {
      typedef core::ImageSpatial<T, Mapper, Allocator>                                 ImageSpatial;
      typedef core::InterpolatorNearestNeighbor2D<T, Mapper, Allocator>                Interpolator;
      typedef core::ImageTransformationProcessorResampler<ImageSpatial, Interpolator>  Processor;

      core::ImageTransformationMapperDdf ddfTransformationMapper;
      Processor proc( target, source );
      ddfTransformationMapper.run( proc, target, ddf, source );
   }
}
}

#endif