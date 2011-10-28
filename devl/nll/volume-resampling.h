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
      VolumeTransformationProcessorResampler( const Volume& source, Volume& target ) : _source( &source ), _target( &target ), _interpolator( source )
      {}

      // called as soon as the volume mapper started the mapping process
      void start()
      {
         _interpolator.startInterpolation();
      }

      void startSlice( ui32 )
      {
      }

      // called as soon as the volume mapper ended the mapping process
      void end()
      {
         _interpolator.endInterpolation();
      }

      void endSlice( ui32 )
      {
      }

      // called everytime a new voxel in the target volume is reached
      // sourcePosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( DirectionalIterator& it, const float* sourcePosition )
      {
         *it = _interpolator( sourcePosition );
      }

      // called everytime a new voxel in the target volume is reached
      // sourcePosition is guaranteed to be aligned on 16 bytes and to contain 4 values: [x, y, z, 0]
      void process( const core::vector3ui& targetPosition, const float* sourcePosition )
      {
         (*_target)( targetPosition[ 0 ], targetPosition[ 1 ], targetPosition[ 2 ] ) = _interpolator( sourcePosition );
      }

   private:
      const Volume*     _source;
      Volume*           _target;
      Interpolator      _interpolator;
   };

   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& target )
   {
      typedef VolumeSpatial<T, Storage> VolumeT;
      typedef InterpolatorNearestNeighbour<VolumeT>  Interpolator;

      VolumeTransformationProcessorResampler<VolumeT, Interpolator> procResample( source, target );
      VolumeTransformationMapper mapper;
      mapper.run( procResample, source, tfm, target );
   }

   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& source, VolumeSpatial<T, Storage>& target )
   {
      TransformationAffine tfm;
      resampleVolumeNearestNeighbour( source, tfm, target );
   }

   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& target )
   {
      typedef VolumeSpatial<T, Storage> VolumeT;
      typedef InterpolatorTriLinear<VolumeT>  Interpolator;

      VolumeTransformationProcessorResampler<VolumeT, Interpolator> procResample( source, target );
      VolumeTransformationMapper mapper;
      mapper.run( procResample, source, tfm, target );
   }

   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& source, VolumeSpatial<T, Storage>& target )
   {
      TransformationAffine tfm;
      resampleVolumeTrilinear( source, tfm, target );
   }
}
}

#endif
