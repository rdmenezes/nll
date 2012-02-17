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

#ifndef NLL_IMAGING_VOLUME_BARYCENTRE_H_
# define NLL_IMAGING_VOLUME_BARYCENTRE_H_

namespace nll
{
namespace imaging
{
   /**
    @brief Computes the barycentre of the volume, using this specific LUT
    */
   template <class Volume, class Lut>
   core::vector3f computeBarycentre( const Volume& vol, const Lut& lut )
   {
      // by default, it is the centre of the volume
      core::VolumeGeometry geometry( vol.getPst() );

      // compute the barycentre
      float nb = 0;
      const ui32 lutSize = lut.getSize();
      core::vector3f tmp( 0, 0, 0 );
      for ( ui32 z = 0; z < vol.size()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < vol.size()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < vol.size()[ 0 ]; ++x )
            {
               const float value = vol( x, y, z );
               const float computed = lut.transform( value )[ 0 ];
               const float ratio = computed / lutSize;
               tmp += core::vector3f( static_cast<f32>( x ),
                                      static_cast<f32>( y ), 
                                      static_cast<f32>( z ) ) * ratio;
               nb += ratio;
            }
         }
      }
      return geometry.indexToPosition( tmp / nb );
   }
}
}

#endif