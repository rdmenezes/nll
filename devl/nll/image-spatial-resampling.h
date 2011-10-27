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
    @brief Processor used for resampling
    */
   template <class ImageSpatial, class Interpolator>
   class ImageTransformationProcessorResampler
   {
   public:
      typedef typename ImageSpatial::DirectionalIterator          DirectionalIterator;
      typedef typename ImageSpatial::ConstDirectionalIterator     ConstDirectionalIterator;

   public:
      ImageTransformationProcessorResampler( const ImageSpatial& source, ImageSpatial& target ) : _source( &source ), _target( &target ), _interpolator( source )
      {}

      // called as soon as the volume mapper started the mapping process
      void start()
      {}

      // called as soon as the volume mapper ended the mapping process
      void end()
      {}

      // called everytime a new voxel in the target volume is reached
      // sourcePosition contains 2 values: [x, y ]
      void process( const DirectionalIterator& targetPos, const float* sourcePosition )
      {
         // it is guaranteed the colors are contiguous
         _interpolator.interpolateValues( sourcePosition[ 0 ], sourcePosition[ 1 ], &(*targetPos) );
      }

   private:
      const ImageSpatial*  _source;
      ImageSpatial*        _target;
      Interpolator         _interpolator;
   };

   template <class T, class Mapper, class Alloc, class Interpolator>
   void resample( const ImageSpatial<T, Mapper, Alloc>& source, const Matrix<float>& tfm, ImageSpatial<T, Mapper, Alloc>& target )
   {
      typedef core::ImageSpatial<T, Mapper, Alloc> ImageSpacialT;
      core::ImageTransformationProcessorResampler<ImageSpacialT, Interpolator> proc( source, target );
      core::ImageTransformationMapper mapper;
      mapper.run( proc, source, tfm, target );
   }

   template <class T, class Mapper, class Alloc>
   void resampleNearest( const ImageSpatial<T, Mapper, Alloc>& source, const Matrix<float>& tfm, ImageSpatial<T, Mapper, Alloc>& target )
   {
      resample< T, Mapper, Alloc, InterpolatorNearestNeighbor2D<T, Mapper, Alloc> >( source, tfm, target );
   }

   template <class T, class Mapper, class Alloc>
   void resampleLinear( const ImageSpatial<T, Mapper, Alloc>& source, const Matrix<float>& tfm, ImageSpatial<T, Mapper, Alloc>& target )
   {
      resample< T, Mapper, Alloc, InterpolatorLinear2D<T, Mapper, Alloc> >( source, tfm, target );
   }
}
}

#endif