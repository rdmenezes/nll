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

#ifndef NLL_IMAGE_SNR_H_
# define NLL_IMAGE_SNR_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief compute the PSNR of 2 images

    @param signalDynamic max possible value of the pixel (char=256...)
    */
   template <class T, class Mapper, class Allocator>
   double psnr( const Image<T, Mapper, Allocator>& orig, const Image<T, Mapper, Allocator>& reconstructed, const T signalDynamic = Bound<T>::max )
   {
      assert( orig.sizex() == reconstructed.sizex() );
      assert( orig.sizey() == reconstructed.sizey() );
      assert( orig.getNbComponents() == reconstructed.getNbComponents() );

      double eqm = 0;
      for ( ui32 ny = 0; ny < orig.sizey(); ++ny )
         for ( ui32 nx = 0; nx < orig.sizex(); ++nx )
            // only computed on luminance values
            // TODO : bad cast
            eqm += ( generic_norm2<T*, f64>( (T*)orig.point( nx, ny ), (T*)reconstructed.point( nx, ny ), orig.getNbComponents() ) / orig.getNbComponents() );
      eqm /= orig.sizex() * orig.sizey(); 
      return 10 * log10( signalDynamic * signalDynamic / eqm );
   }
}
}

#endif
