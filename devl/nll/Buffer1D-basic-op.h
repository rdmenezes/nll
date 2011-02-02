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

#ifndef NLL_CORE_BUFFER1D_BASIC_OP_H_
# define NLL_CORE_BUFFER1D_BASIC_OP_H_

namespace nll
{
namespace core
{
   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator>& operator+=( Buffer1D<T, Mapper, Allocator>& dst, const Buffer1D<T, Mapper, Allocator>& src )
   {
      assert( dst.size() == src.size() );
      generic_add<T*, const T*>( dst.getBuf(), src.getBuf(), dst.size() );
      return dst;
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator> operator+( const Buffer1D<T, Mapper, Allocator>& src1, const Buffer1D<T, Mapper, Allocator>& src2 )
   {
      assert( src1.size() == src2.size() );
      Buffer1D<T, Mapper, Allocator> res( src1.size(), false, src1.getAllocator() );

      T* bufDst = res.getBuf();
      const T* bufSrc1 = src1.getBuf();
      const T* bufSrc2 = src2.getBuf();

      for ( ui32 n = 0; n < src1.size(); ++n )
      {
         bufDst[ n ] = bufSrc1[ n ] + bufSrc2[ n ];
      }

      return res;
   }

   template <class T, class Mapper, class Allocator>
   double norm2( const Buffer1D<T, Mapper, Allocator>& src1 )
   {
      return generic_norm2<const T*, double> ( src1.getBuf(), src1.size() );
   }

   template <class T, class Mapper, class Allocator>
   double dot( const Buffer1D<T, Mapper, Allocator>& src1, const Buffer1D<T, Mapper, Allocator>& src2 )
   {
      assert( src1.size() == src2.size() );
      const T* bufSrc1 = src1.getBuf();
      const T* bufSrc2 = src2.getBuf();

      double accum = 0;
      for ( ui32 n = 0; n < src1.size(); ++n )
      {
         accum += bufSrc1[ n ] * bufSrc2[ n ];
      }

      return accum;
   }

   template <class T>
   double dot( const std::vector<T>& src1, const std::vector<T>& src2 )
   {
      assert( src1.size() == src2.size() );

      double accum = 0;
      for ( ui32 n = 0; n < src1.size(); ++n )
      {
         accum += src1[ n ] * src2[ n ];
      }

      return accum;
   }

   template <class T, class Mapper, class Allocator>
   double norm2( const Buffer1D<T, Mapper, Allocator>& src1, const Buffer1D<T, Mapper, Allocator>& src2 )
   {
      assert( src1.size() == src2.size() );
      return generic_norm2<const T*, double> ( src1.getBuf(), src2.getBuf(), src1.size() );
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator> operator-( const Buffer1D<T, Mapper, Allocator>& src1, const Buffer1D<T, Mapper, Allocator>& src2 )
   {
      assert( src1.size() == src2.size() );
      Buffer1D<T, Mapper, Allocator> res( src1.size(), false, src1.getAllocator() );

      T* bufDst = res.getBuf();
      const T* bufSrc1 = src1.getBuf();
      const T* bufSrc2 = src2.getBuf();

      for ( ui32 n = 0; n < src1.size(); ++n )
      {
         bufDst[ n ] = bufSrc1[ n ] - bufSrc2[ n ];
      }

      return res;
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator>& operator-=( Buffer1D<T, Mapper, Allocator>& dst, const Buffer1D<T, Mapper, Allocator>& src )
   {
      assert( dst.size() == src.size() );
      generic_sub<T*, const T*>( dst.getBuf(), src.getBuf(), dst.size() );
      return dst;
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator>& operator*=( Buffer1D<T, Mapper, Allocator>& dst, T val )
   {
      generic_mul_cte<T*>( dst.getBuf(), val, dst.size() );
      return dst;
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator>& operator/=( Buffer1D<T, Mapper, Allocator>& dst, T val )
   {
      assert( val != 0 );
      generic_div_cte<T*>( dst.getBuf(), val, dst.size() );
      return dst;
   }
}
}

#endif