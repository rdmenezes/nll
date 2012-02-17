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

#ifndef NLL_IMAGE_CONVERTER_H_
# define NLL_IMAGE_CONVERTER_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief convert from one image type and mapper to another. If target type is too small, the value is truncated.
    */
   template <class Ti, class Mapperi, class To, class Mappero>
   void convert( const Image<Ti, Mapperi>& i, Image<To, Mappero>& out )
   {
      Image<To, Mappero> o( i.sizex(), i.sizey(), i.getNbComponents() );
      for ( ui32 ny = 0; ny < i.sizey(); ++ny )
         for ( ui32 nx = 0; nx < i.sizex(); ++nx )
            for ( ui32 c = 0; c < i.getNbComponents(); ++c )
               o( nx, ny, c ) = static_cast<To> ( NLL_BOUND( i( nx, ny, c ), (To)Bound<To>::min, (To)Bound<To>::max ) );
      out = o;
   }
}
}

#endif
