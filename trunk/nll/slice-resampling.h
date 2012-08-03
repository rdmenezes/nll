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

#ifndef NLL_IMAGING_SLICE_RESAMPLING_H_
# define NLL_IMAGING_SLICE_RESAMPLING_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Resample an input slice to an arbitrary output geometry
    @param input the input slice
    @param interpolator2D the interpolator to be used for resampling
    @param output the output slice. It must be already allocated. The input will be resampled to the output
           geometry
    @note the input & output slices must share the same normal
    */
   template <class T, class Interpolator2D>
   void resampling( const Slice<T>& input, Slice<T>& output )
   {
      typedef Slice<T>  SliceType;
      ensure( output.size()[ 0 ] && output.size()[ 1 ] && output.size()[ 2 ], "the output slice is invalid!" );
      ensure( ( core::equal<float>( input.getNormal()[ 0 ], output.getNormal()[ 0 ], 1e-5f ) &&
                core::equal<float>( input.getNormal()[ 1 ], output.getNormal()[ 1 ], 1e-5f ) &&
                core::equal<float>( input.getNormal()[ 2 ], output.getNormal()[ 2 ], 1e-5f ) ) || 
              ( core::equal<float>( -input.getNormal()[ 0 ], output.getNormal()[ 0 ], 1e-5f ) &&
                core::equal<float>( -input.getNormal()[ 1 ], output.getNormal()[ 1 ], 1e-5f ) &&
                core::equal<float>( -input.getNormal()[ 2 ], output.getNormal()[ 2 ], 1e-5f ) ), "Input & output slice must be in the same plane" );
      ensure( output.size()[ 2 ] == input.size()[ 2 ], "input and output values must be of the same type" );
      ensure( output.isInPlane( input.getOrigin() ), "input and output slices must be in the same plan" );
      
      // compute ( ( 1, 0 ) - ( 0, 0 ) ) and ( ( 0, 1 ) - ( 0, 0 ) ) in input slice coordinate
      // compute doutputx = u1 * dinputx + v1 * dinputy
      // compute doutputy = u2 * dinputx + v2 * dinputy
      const core::vector2f outOriginInInput = input.worldToSliceCoordinate( output.getOrigin() );
      const core::vector2f dx = input.worldToSliceCoordinate( output.sliceToWorldCoordinate( core::vector2f( 1, 0 ) ) ) - outOriginInInput;
      const core::vector2f dy = input.worldToSliceCoordinate( output.sliceToWorldCoordinate( core::vector2f( 0, 1 ) ) ) - outOriginInInput;


      Interpolator2D interpolator( input.getStorage() );
      typename SliceType::DirectionalIterator it = output.getIterator( 0, 0 );

      // find the top left corner of the output slice in world coordinate
      const core::vector3f cornerInWorldSpace = output.sliceToWorldCoordinate( core::vector2f( - static_cast<float>( output.size()[ 0 ] ) / 2,
                                                                                               - static_cast<float>( output.size()[ 1 ] ) / 2 ) );
      const core::vector2f cornerInInput = input.worldToSliceCoordinate( cornerInWorldSpace );
      core::vector2f start = cornerInInput + core::vector2f( static_cast<float>( input.size()[ 0 ] ) / 2, static_cast<float>( input.size()[ 1 ] ) / 2 );
      for ( size_t y = 0; y < output.sizey(); ++y )
      {
         core::vector2f pixelIter = start;
         typename SliceType::DirectionalIterator itline = it;
         it.addy();
         while ( itline != it ) 
         {
            interpolator.interpolateValues( pixelIter[ 0 ], pixelIter[ 1 ], &( *itline ) );
            itline.addx();
            pixelIter += dx;
         }
         start += dy;
      }
   }
}
}

#endif