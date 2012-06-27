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
 
#ifndef NLL_MASK_EXPORTER_H_
# define NLL_MASK_EXPORTER_H_

namespace nll
{
namespace utility
{
   /**
    @ingroup utility
    @brief export a mask using random colors for each ID
    */
   template <class T, class Mapper> void maskExport( const core::ImageMask& mask, core::Image<T, Mapper>& out )
   {
      ui32 randColor[ 3 ];
      ui32 randDev[ 3 ];

      randColor[ 0 ] = ( rand() + 1) % 256;
      randColor[ 1 ] = ( rand() + 1) % 256;
      randColor[ 2 ] = ( rand() + 1) % 256;

      randDev[ 0 ] = ( rand() + 1) % 256;
      randDev[ 1 ] = ( rand() + 1) % 256;
      randDev[ 2 ] = ( rand() + 1) % 256;

      core::Image<T, Mapper> i( mask.sizex(), mask.sizey(), 3 );
      for ( ui32 y = 0; y < mask.sizey(); ++y )
         for ( ui32 x = 0; x < mask.sizex(); ++x )
            for ( ui32 c = 0; c < 3; ++c )
               i( x, y, c ) = static_cast<T>( ( randColor[ c ] * mask( x, y, 0 ) + randDev[ c ] ) % 256 );
      out = i;
   }
}
}

#endif
