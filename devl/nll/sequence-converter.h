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

#ifndef NLL_SEQUENCE_CONVERTER_H_
# define NLL_SEQUENCE_CONVERTER_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief convert a sequence type to another. T2 needs to provide value_type, operator[] and T2(unsigned int) which allocate memory for N elements
   */
   template <class T1, class T2>
   inline T2 convert( const T1& arg, ui32 size )
   {
      T2 buf( size );
      for ( ui32 n = 0; n < size; ++n )
         buf[ n ] = static_cast<typename T2::value_type>( arg[ n ] );
      return buf;
   }

   /**
    @ingroup core
    @brief convert a sequence type to another. T2 needs to provide value_type, operator[] and T2(unsigned int) which allocate memory for N elements
   */
   template <class T1, class T2>
   inline void convert( const T1& arg, T2& out, ui32 size )
   {
      out = convert<T1, T2>( arg, size );
   }
}
}

#endif
