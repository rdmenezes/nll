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

#ifndef NLL_VECTORIZED_VEC4I_H_
# define NLL_VECTORIZED_VEC4I_H_

namespace nll
{
namespace vectorized
{
   // Select between two sources, byte by byte. Used in various functions and operators
   // Corresponds to this pseudocode:
   // for (int i = 0; i < 16; i++) result[i] = s[i] ? a[i] : b[i];
   // Each byte in s must be either 0 (false) or 0xFF (true). No other values are allowed.
   // The implementation depends on the instruction set: 
   // If SSE4.1 is supported then only bit 7 in each byte of s is checked, 
   // otherwise all bits in s are used.
   static inline __m128i selectb (__m128i const & s, __m128i const & a, __m128i const & b)
   {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41   // SSE4.1 supported
      return _mm_blendv_epi8 (b, a, s);
   #else
      return _mm_or_si128(
         _mm_and_si128(s,a),
         _mm_andnot_si128(s,b));
   #endif
   }

   class Vec4i
   {
   public:
      // Default constructor:
      Vec4i()
      {
      };
      // Constructor to broadcast the same value into all elements:
      Vec4i(int i)
      {
         xmm = _mm_set1_epi32(i);
      };

      // Constructor to build from all elements:
      Vec4i(i32 i0, i32 i1, i32 i2, i32 i3)
      {
         xmm = _mm_setr_epi32(i0, i1, i2, i3);
      };

      // Constructor to convert from type __m128i used in intrinsics:
      Vec4i(__m128i const & x)
      {
         xmm = x;
      };
      // Assignment operator to convert from type __m128i used in intrinsics:
      Vec4i & operator = (__m128i const & x)
      {
         xmm = x;
         return *this;
      };

      // Type cast operator to convert to __m128i used in intrinsics
      operator __m128i() const
      {
         return xmm;
      };

      // Member function to load from array (unaligned)
      Vec4i & load(void const * p)
      {
         xmm = _mm_loadu_si128((__m128i const*)p);
         return *this;
      }

      // Member function to load from array (aligned)
      Vec4i & load_a(void const * p)
      {
         xmm = _mm_load_si128((__m128i const*)p);
         return *this;
      }

      void store(void * p) const
      {
         _mm_storeu_si128((__m128i*)p, xmm);
      }
      // Member function to store into array, aligned by 16
      // "store_a" is faster than "store" on older Intel processors (Pentium 4, Pentium M, Core 1,
      // Merom, Wolfdale) and Atom, but not on other processors from Intel, AMD or VIA.
      // You may use store_a instead of store if you are certain that p points to an address
      // divisible by 16.
      void store_a(void * p) const
      {
         _mm_store_si128((__m128i*)p, xmm);
      }

      // Partial load. Load n elements and set the rest to 0
      Vec4i & load_partial(int n, void const * p)
      {
         switch (n) {
            case 0:
               *this = 0;
               break;
            case 1:
               xmm = _mm_cvtsi32_si128(*(i32*)p);
               break;
            case 2:
               // intrinsic for movq is missing!
               xmm = _mm_setr_epi32(((i32*)p)[0], ((i32*)p)[1], 0, 0);
               break;
            case 3:
               xmm = _mm_setr_epi32(((i32*)p)[0], ((i32*)p)[1], ((i32*)p)[2], 0);
               break;
            case 4:
               load(p);
               break;
            default: 
               break;
         }
         return *this;
      }

      // Partial store. Store n elements
      void store_partial(int n, void * p) const
      {
         union Tmp
         {        
            i32 i[4];
            i64 q[2];
         };
         
         Tmp u;

         switch (n) {
         case 1:
            *(i32*)p = _mm_cvtsi128_si32(xmm);
            break;
         case 2:
            // intrinsic for movq is missing!
            store(u.i);
            *(i64*)p = u.q[0];
            break;
         case 3:
            store(u.i);
            *(i64*)p     = u.q[0];  
            ((i32*)p)[2] = u.i[2];
            break;
         case 4:
           store(p);
           break;
         default:
            break;
         }
      }

      // Member function to change a single element in vector
      // Note: This function is inefficient. Use load function if changing more than one element
      Vec4i const & insert(ui32 index, ui32 value)
      {
         static const ui32 maskl[8] = {0,0,0,0,-1,0,0,0};
         __m128i broad = _mm_set1_epi32(value);  // broadcast value into all elements
         __m128i mask  = _mm_loadu_si128((__m128i const*)(maskl+4-(index & 3))); // mask with FFFFFFFF at index position
         xmm = selectb(mask,broad,xmm);
         return *this;
      };

      // Member function extract a single element from vector
      i32 extract(ui32 index) const
      {
         i32 x[4];
         store(x);
         return x[ index ];
      }

      // Extract a single element. Use store function if extracting more than one element.
      // Operator [] can only read an element, not write.
      i32 operator [] (ui32 index) const
      {
         return extract(index);
      }

   protected:
      __m128i xmm; // Integer vector
   };

   // vector operator + : add element by element
   static inline Vec4i operator + (Vec4i const & a, Vec4i const & b)
   {
      return _mm_add_epi32(a, b);
   }

   // vector operator += : add
   static inline Vec4i & operator += (Vec4i & a, Vec4i const & b)
   {
      a = a + b;
      return a;
   }

   // postfix operator ++
   static inline Vec4i operator ++ (Vec4i & a, int)
   {
      Vec4i a0 = a;
      a = a + 1;
      return a0;
   }

   // prefix operator ++
   static inline Vec4i & operator ++ (Vec4i & a)
   {
      a = a + 1;
      return a;
   }

   // vector operator - : subtract element by element
   static inline Vec4i operator - (Vec4i const & a, Vec4i const & b)
   {
      return _mm_sub_epi32(a, b);
   }

   // vector operator - : unary minus
   static inline Vec4i operator - (Vec4i const & a)
   {
      return _mm_sub_epi32(_mm_setzero_si128(), a);
   }

   // vector operator -= : subtract
   static inline Vec4i & operator -= (Vec4i & a, Vec4i const & b)
   {
      a = a - b;
      return a;
   }

   // postfix operator --
   static inline Vec4i operator -- (Vec4i & a, int)
   {
      Vec4i a0 = a;
      a = a - 1;
      return a0;
   }

   // prefix operator --
   static inline Vec4i & operator -- (Vec4i & a)
   {
      a = a - 1;
      return a;
   }

   // vector operator * : multiply element by element
   static inline Vec4i operator * (Vec4i const & a, Vec4i const & b)
   {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41  // SSE4.1 instruction set
      return _mm_mullo_epi32(a, b);
   #else
      __m128i a13    = _mm_shuffle_epi32(a, 0xF5);          // (-,a3,-,a1)
      __m128i b13    = _mm_shuffle_epi32(b, 0xF5);          // (-,b3,-,b1)
      __m128i prod02 = _mm_mul_epu32(a, b);                 // (-,a2*b2,-,a0*b0)
      __m128i prod13 = _mm_mul_epu32(a13, b13);             // (-,a3*b3,-,a1*b1)
      __m128i prod01 = _mm_unpacklo_epi32(prod02,prod13);   // (-,-,a1*b1,a0*b0) 
      __m128i prod23 = _mm_unpackhi_epi32(prod02,prod13);   // (-,-,a3*b3,a2*b2) 
      return           _mm_unpacklo_epi64(prod01,prod23);   // (ab3,ab2,ab1,ab0)
   #endif
   }

   // vector operator *= : multiply
   static inline Vec4i & operator *= (Vec4i & a, Vec4i const & b)
   {
      a = a * b;
      return a;
   }

   // vector operator / : divide all elements by same integer
   // See bottom of file


   // vector operator << : shift left
   static inline Vec4i operator << (Vec4i const & a, i32 b) {
       return _mm_sll_epi32(a,_mm_cvtsi32_si128(b));
   }

   // vector operator <<= : shift left
   static inline Vec4i & operator <<= (Vec4i & a, i32 b) {
       a = a << b;
       return a;
   }

   // vector operator >> : shift right arithmetic
   static inline Vec4i operator >> (Vec4i const & a, i32 b) {
       return _mm_sra_epi32(a,_mm_cvtsi32_si128(b));
   }

   // vector operator >>= : shift right arithmetic
   static inline Vec4i & operator >>= (Vec4i & a, i32 b) {
       a = a >> b;
       return a;
   }

   // vector operator == : returns true for elements for which a == b
   static inline Vec4i operator == (Vec4i const & a, Vec4i const & b) {
       return _mm_cmpeq_epi32(a, b);
   }

   static inline Vec4i operator ~ (Vec4i const & a) {
      return _mm_xor_si128(a, _mm_set1_epi32(-1));
   }

   // vector operator != : returns true for elements for which a != b
   static inline Vec4i operator != (Vec4i const & a, Vec4i const & b) {
   #ifdef __XOP__  // AMD XOP instruction set
       return _mm_comneq_epi32(a,b);
   #else  // SSE2 instruction set
       return Vec4i (~(a == b));
   #endif
   }
  
   // vector operator > : returns true for elements for which a > b
   static inline Vec4i operator > (Vec4i const & a, Vec4i const & b) {
       return _mm_cmpgt_epi32(a, b);
   }

   // vector operator < : returns true for elements for which a < b
   static inline Vec4i operator < (Vec4i const & a, Vec4i const & b) {
       return b > a;
   }

   // vector operator >= : returns true for elements for which a >= b (signed)
   static inline Vec4i operator >= (Vec4i const & a, Vec4i const & b) {
   #ifdef __XOP__  // AMD XOP instruction set
       return _mm_comge_epi32(a,b);
   #else  // SSE2 instruction set
       return Vec4i (~(b > a));
   #endif
   }

   // vector operator ! : returns true for elements == 0
   static inline Vec4i operator ! (Vec4i const & a) {
       return _mm_cmpeq_epi32(a,_mm_setzero_si128());
   }

   // vector operator <= : returns true for elements for which a <= b (signed)
   static inline Vec4i operator <= (Vec4i const & a, Vec4i const & b) {
       return b >= a;
   }

   // Functions for this class

   // Select between two operands. Corresponds to this pseudocode:
   // for (int i = 0; i < 4; i++) result[i] = s[i] ? a[i] : b[i];
   // Each byte in s must be either 0 (false) or -1 (true). No other values are allowed.
   // (s is signed)
   static inline Vec4i select (Vec4i const & s, Vec4i const & a, Vec4i const & b) {
       return selectb(s,a,b);
   }

   // Horizontal add: Calculates the sum of all vector elements.
   // Overflow will wrap around
   static inline i32 horizontal_add (Vec4i const & a) {
   #ifdef __XOP__       // AMD XOP instruction set
       __m128i sum1  = _mm_haddq_epi32(a);
       __m128i sum2  = _mm_shuffle_epi32(sum1,0x0E);          // high element
       __m128i sum3  = _mm_add_epi32(sum1,sum2);              // sum
       return          _mm_cvtsi128_si32(sum3);               // truncate to 32 bits
   #elif  NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSSE3
       __m128i sum1  = _mm_hadd_epi32(a,a);                   // horizontally add 4 elements in 2 steps
       __m128i sum2  = _mm_hadd_epi32(sum1,sum1);
       return          _mm_cvtsi128_si32(sum2);               // 32 bit sum
   #else                 // SSE2
       __m128i sum1  = _mm_shuffle_epi32(a,0x0E);             // 2 high elements
       __m128i sum2  = _mm_add_epi32(a,sum1);                 // 2 sums
       __m128i sum3  = _mm_shuffle_epi32(sum2,0x01);          // 1 high element
       __m128i sum4  = _mm_add_epi32(sum2,sum3);              // 2 sums
       return          _mm_cvtsi128_si32(sum4);               // 32 bit sum
   #endif
   }

   // Horizontal add extended: Calculates the sum of all vector elements.
   // Elements are sign extended before adding to avoid overflow
   static inline i64 horizontal_add_x (Vec4i const & a) {
   #ifdef __XOP__     // AMD XOP instruction set
       __m128i sum1  = _mm_haddq_epi32(a);
   #else              // SSE2
       __m128i signs = _mm_srai_epi32(a,31);                  // sign of all elements
       __m128i a01   = _mm_unpacklo_epi32(a,signs);           // sign-extended a0, a1
       __m128i a23   = _mm_unpackhi_epi32(a,signs);           // sign-extended a2, a3
       __m128i sum1  = _mm_add_epi64(a01,a23);                // add
   #endif
       __m128i sum2  = _mm_unpackhi_epi64(sum1,sum1);         // high qword
       __m128i sum3  = _mm_add_epi64(sum1,sum2);              // add
   #if defined (__x86_64__)
       return          _mm_cvtsi128_si64(sum3);               // 64 bit mode
   #else
       union {
           __m128i x;  // silly definition of _mm_storel_epi64 requires __m128i
           i64 i;
       } u;
       _mm_storel_epi64(&u.x,sum3);
       return u.i;
   #endif
   }

   // function add_saturated: add element by element, signed with saturation
   static inline Vec4i add_saturated(Vec4i const & a, Vec4i const & b) {
       __m128i sum    = _mm_add_epi32(a, b);                  // a + b
       __m128i axb    = _mm_xor_si128(a, b);                  // check if a and b have different sign
       __m128i axs    = _mm_xor_si128(a, sum);                // check if a and sum have different sign
       __m128i overf1 = _mm_andnot_si128(axb,axs);            // check if sum has wrong sign
       __m128i overf2 = _mm_srai_epi32(overf1,31);            // -1 if overflow
       __m128i asign  = _mm_srli_epi32(a,31);                 // 1  if a < 0
       __m128i sat1   = _mm_srli_epi32(overf2,1);             // 7FFFFFFF if overflow
       __m128i sat2   = _mm_add_epi32(sat1,asign);            // 7FFFFFFF if positive overflow 80000000 if negative overflow
       return  selectb(overf2,sat2,sum);                      // sum if not overflow, else sat2
   }

   // function sub_saturated: subtract element by element, signed with saturation
   static inline Vec4i sub_saturated(Vec4i const & a, Vec4i const & b) {
       __m128i diff   = _mm_sub_epi32(a, b);                  // a + b
       __m128i axb    = _mm_xor_si128(a, b);                  // check if a and b have different sign
       __m128i axs    = _mm_xor_si128(a, diff);               // check if a and sum have different sign
       __m128i overf1 = _mm_and_si128(axb,axs);               // check if sum has wrong sign
       __m128i overf2 = _mm_srai_epi32(overf1,31);            // -1 if overflow
       __m128i asign  = _mm_srli_epi32(a,31);                 // 1  if a < 0
       __m128i sat1   = _mm_srli_epi32(overf2,1);             // 7FFFFFFF if overflow
       __m128i sat2   = _mm_add_epi32(sat1,asign);            // 7FFFFFFF if positive overflow 80000000 if negative overflow
       return  selectb(overf2,sat2,diff);                     // diff if not overflow, else sat2
   }

   // function max: a > b ? a : b
   static inline Vec4i max(Vec4i const & a, Vec4i const & b) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41   // SSE4.1 supported
       return _mm_max_epi32(a,b);
   #else
       __m128i greater = _mm_cmpgt_epi32(a,b);
       return selectb(greater,a,b);
   #endif
   }

   // function min: a < b ? a : b
   static inline Vec4i min(Vec4i const & a, Vec4i const & b) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41   // SSE4.1 supported
       return _mm_min_epi32(a,b);
   #else
       __m128i greater = _mm_cmpgt_epi32(a,b);
       return selectb(greater,b,a);
   #endif
   }

   // function abs: a >= 0 ? a : -a
   static inline Vec4i abs(Vec4i const & a) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSSE3     // SSSE3 supported
       return _mm_sign_epi32(a,a);
   #else                 // SSE2
       __m128i sign = _mm_srai_epi32(a,31);                   // sign of a
       __m128i inv  = _mm_xor_si128(a,sign);                  // invert bits if negative
       return         _mm_sub_epi32(inv,sign);                // add 1
   #endif
   }

   // function abs_saturated: same as abs, saturate if overflow
   static inline Vec4i abs_saturated(Vec4i const & a) {
       __m128i absa   = abs(a);                               // abs(a)
       __m128i overfl = _mm_srai_epi32(absa,31);              // sign
       return           _mm_add_epi32(absa,overfl);           // subtract 1 if 0x80000000
   }

   // horizontal_and. Returns true if all bits are 1
   static inline bool horizontal_and (Vec4i const & a) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41   // SSE4.1 supported. Use PTEST
       return _mm_testc_si128(a,constant4i<-1,-1,-1,-1>()) != 0;
   #else
       __m128i t1 = _mm_unpackhi_epi64(a,a);                  // get 64 bits down
       __m128i t2 = _mm_and_si128(a,t1);                      // and 64 bits
   #ifdef __x86_64__
       int64_t t5 = _mm_cvtsi128_si64(t2);                    // transfer 64 bits to integer
       return  t5 == int64_t(-1);
   #else
       __m128i t3 = _mm_srli_epi64(t2,32);                    // get 32 bits down
       __m128i t4 = _mm_and_si128(t2,t3);                     // and 32 bits
       int     t5 = _mm_cvtsi128_si32(t4);                    // transfer 32 bits to integer
       return  t5 == -1;
   #endif  // __x86_64__
   #endif  // INSTRSET
   }

   // horizontal_or. Returns true if at least one bit is 1
   static inline bool horizontal_or (Vec4i const & a) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41   // SSE4.1 supported. Use PTEST
       return ! _mm_testz_si128(a,a);
   #else
       __m128i t1 = _mm_unpackhi_epi64(a,a);                  // get 64 bits down
       __m128i t2 = _mm_or_si128(a,t1);                       // and 64 bits
   #ifdef __x86_64__
       int64_t t5 = _mm_cvtsi128_si64(t2);                    // transfer 64 bits to integer
       return  t5 != int64_t(0);
   #else
       __m128i t3 = _mm_srli_epi64(t2,32);                    // get 32 bits down
       __m128i t4 = _mm_or_si128(t2,t3);                      // and 32 bits
       int     t5 = _mm_cvtsi128_si32(t4);                    // transfer to integer
       return  t5 != 0;
   #endif  // __x86_64__
   #endif  // INSTRSET
   }
}
}

#endif