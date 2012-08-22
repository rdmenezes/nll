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

#ifndef NLL_VECTORIZED_VEC4FB_H_
# define NLL_VECTORIZED_VEC4FB_H_

namespace nll
{
namespace vectorized
{
   /**
    @brief Vector of 4 Booleans for use with Vec4f
    */
   class Vec4fb {
   protected:
      __m128 xmm; // Float vector
      public:
      // Default constructor:
      Vec4fb()
      {}

      // Constructor to broadcast the same value into all elements:
      Vec4fb(bool b) {
         xmm = _mm_castsi128_ps(_mm_set1_epi32(-(int)b));
      }

      // Constructor to build from all elements:
      Vec4fb(bool b0, bool b1, bool b2, bool b3) {
         xmm = _mm_castsi128_ps(_mm_setr_epi32(-(int)b0, -(int)b1, -(int)b2, -(int)b3)); 
      }

      // Constructor to convert from type __m128 used in intrinsics:
      Vec4fb(__m128 const & x) 
      {
         xmm = x;
      }

      // Assignment operator to convert from type __m128 used in intrinsics:
      Vec4fb & operator = (__m128 const & x) 
      {
         xmm = x;
         return *this;
      }

      // Type cast operator to convert to __m128 used in intrinsics
      operator __m128() const
      {
         return xmm;
      }

      // Constructor to convert from type Vec4i used as Boolean for integer vectors
      Vec4fb(Vec4i const & x)
      {
         xmm = _mm_castsi128_ps(x);
      }

      // Assignment operator to convert from type Vec4i used as Boolean for integer vectors
      Vec4fb & operator = (Vec4i const & x) 
      {
         xmm = _mm_castsi128_ps(x);
         return *this;
      }

      // Type cast operator to convert to type Vec4i used as Boolean for integer vectors
      operator Vec4i() const
      {
         return _mm_castps_si128(xmm);
      }

      // Member function to change a single element in vector
      // Note: This function is inefficient. Use load function if changing more than one element
      Vec4fb const & insert(ui32 index, bool value)
      {
         static const i32 maskl[8] = {0,0,0,0,-1,0,0,0};
         __m128 mask  = _mm_loadu_ps((float const*)(maskl+4-(index & 3))); // mask with FFFFFFFF at index position
         if (value)
         {
            xmm = _mm_or_ps(xmm,mask);
         }
         else
         {
            xmm = _mm_andnot_ps(mask,xmm);
         }
         return *this;
      }

      // Member function extract a single element from vector
      int extract(ui32 index) const 
      {
         return Vec4i(*this).extract(index);
      }
      // Extract a single element. Operator [] can only read an element, not write.
      int operator [] (ui32 index) const
      {
         return extract(index);
      }
   };

   // vector operator & : bitwise and
   static inline Vec4fb operator & (Vec4fb const & a, Vec4fb const & b)
   {
      return _mm_and_ps(a, b);
   }

   static inline Vec4fb operator && (Vec4fb const & a, Vec4fb const & b) 
   {
      return a & b;
   }

   // vector operator &= : bitwise and
   static inline Vec4fb & operator &= (Vec4fb & a, Vec4fb const & b) 
   {
      a = a & b;
      return a;
   }

   // vector operator | : bitwise or
   static inline Vec4fb operator | (Vec4fb const & a, Vec4fb const & b) 
   {
      return _mm_or_ps(a, b);
   }
   static inline Vec4fb operator || (Vec4fb const & a, Vec4fb const & b) 
   {
      return a | b;
   }

   // vector operator |= : bitwise or
   static inline Vec4fb & operator |= (Vec4fb & a, Vec4fb const & b) 
   {
      a = a | b;
      return a;
   }

   // vector operator ^ : bitwise xor
   static inline Vec4fb operator ^ (Vec4fb const & a, Vec4fb const & b) 
   {
      return _mm_xor_ps(a, b);
   }

   // vector operator ^= : bitwise xor
   static inline Vec4fb & operator ^= (Vec4fb & a, Vec4fb const & b) 
   {
      a = a ^ b;
      return a;
   }

   // vector operator ~ : bitwise not
   static inline Vec4fb operator ~ (Vec4fb const & a) 
   {
      return _mm_xor_ps(a, _mm_castsi128_ps(_mm_set1_epi32(-1)));
   }

   // vector operator ! : logical not
   // (operator ! is less efficient than operator ~. Use only where not
   // all bits in an element are the same)
   static inline Vec4fb operator ! (Vec4fb const & a) 
   {
      return Vec4fb( ! Vec4i(a));
   }

   // Functions for Vec4fb

   // andnot: a & ~ b
   static inline Vec4fb andnot(Vec4fb const & a, Vec4fb const & b) 
   {
      return _mm_andnot_ps(b, a);
   }

   // horizontal_and. Returns true if all bits are 1
   static inline bool horizontal_and (Vec4fb const & a)
   {
      return horizontal_and(Vec4i(_mm_castps_si128(a)));
   }

   // horizontal_or. Returns true if at least one bit is 1
   static inline bool horizontal_or (Vec4fb const & a)
   {
      return horizontal_or(Vec4i(_mm_castps_si128(a)));
   }
}
}

#endif