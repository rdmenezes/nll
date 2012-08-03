/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in target and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of target code must retain the above copyright
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
    @brief Use a simple volume processor to compute the resampled volume
    */
   template <class Volume, class InterpolatorT>
   class VolumeTransformationProcessorResampler
   {
   public:
      typedef typename Volume::DirectionalIterator          DirectionalIterator;
      typedef typename Volume::ConstDirectionalIterator     ConstDirectionalIterator;
      typedef typename Volume::value_type value_type;
      typedef InterpolatorT Interpolator;

   public:
      VolumeTransformationProcessorResampler( const Volume& target, Volume& resampled ) : _target( &target ), _resampled( &resampled ), _interpolator( target )
      {}

      // called as soon as the volume mapper started the mapping process
      void start()
      {
         // do not start interpolation here! When multithreaded, we want the processor's thread to be in the correct rounding mode (see <code>InterpolatorTriLinear</code>)
      }

      void startSlice( size_t )
      {
         // for some reason the interpolation start/end must be done each slice due to accuracy problems...
         _interpolator.startInterpolation();
      }

      // called as soon as the volume mapper ended the mapping process
      void end()
      {
         // do not end interpolation here! When multithreaded, we want the processor's thread to be in the correct rounding mode (see <code>InterpolatorTriLinear</code>)
      }

      void endSlice( size_t )
      {
         // for some reason the interpolation start/end must be done each slice due to accuracy problems...
         _interpolator.endInterpolation();
      }

      // called everytime a new voxel in the resampled volume is reached
      // targetPosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( DirectionalIterator& it, const float* targetPosition )
      {
         *it = _interpolator( targetPosition );
      }

      // called everytime a new voxel in the resampled volume is reached
      // targetPosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( const core::vector3ui& resampledPosition, const float* targetPosition )
      {
         (*_resampled)( resampledPosition[ 0 ], resampledPosition[ 1 ], resampledPosition[ 2 ] ) = _interpolator( targetPosition );
      }

   private:
      const Volume*     _target;
      Volume*           _resampled;
      Interpolator      _interpolator;
   };

   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& target, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled )
   {
      typedef VolumeSpatial<T, Storage> VolumeT;
      typedef InterpolatorNearestNeighbour<VolumeT>  Interpolator;

      VolumeTransformationProcessorResampler<VolumeT, Interpolator> procResample( target, resampled );
      VolumeTransformationMapper mapper;
      mapper.run( procResample, target, tfm, resampled );
   }

   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& resampled )
   {
      TransformationAffine tfm;
      resampleVolumeNearestNeighbour( target, tfm, resampled );
   }

   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& target, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled )
   {
      typedef VolumeSpatial<T, Storage> VolumeT;
      typedef InterpolatorTriLinear<VolumeT>  Interpolator;

      VolumeTransformationProcessorResampler<VolumeT, Interpolator> procResample( target, resampled );
      VolumeTransformationMapper mapper;
      mapper.run( procResample, target, tfm, resampled );
   }

   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& target, const TransformationDenseDeformableField& tfm, VolumeSpatial<T, Storage>& resampled )
   {
      typedef VolumeSpatial<T, Storage> VolumeT;
      typedef InterpolatorTriLinear<VolumeT>  Interpolator;

      VolumeTransformationProcessorResampler<VolumeT, Interpolator> procResample( target, resampled );
      VolumeTransformationMapperDdf mapper;
      mapper.run( procResample, target, tfm, resampled );
   }

   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& resampled )
   {
      TransformationAffine tfm;
      resampleVolumeTrilinear( target, tfm, resampled );
   }
}
}

#endif
