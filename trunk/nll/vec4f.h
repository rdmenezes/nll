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

#ifndef NLL_VECTORIZED_VEC4F_H_
# define NLL_VECTORIZED_VEC4F_H_

namespace nll
{
namespace vectorized
{
   /**
    @brief Vector holding 4 32bits floating point values
    */
   class Vec4f
   {
   public:
      /**
       @brief default constructor
       */
      Vec4f()
      {}

      Vec4f( const float * p ) : xmm( _mm_loadu_ps( p ) )
      {}

      /**
       @brief Initialize the 4-vector with the same value
       */
      Vec4f( float f ) : xmm( _mm_set1_ps( f ) )
      {}

      /**
       @brief Constructor to build from 4 values
       */
      Vec4f( float f0, float f1, float f2, float f3 ) : xmm( _mm_setr_ps( f0, f1, f2, f3 ) )
      {}

      /**
       @brief Constructor to convert from type __m128 used in intrinsics
       */
      Vec4f( __m128 const & x ) : xmm( x )
      {}

      explicit Vec4f( const Vec4i& v ) : xmm( _mm_cvtepi32_ps( v ) )
      {}

      /**
       @brief Assignment operator to convert from type __m128 used in intrinsics
       */
      Vec4f& operator=( __m128 const & x )
      {
         xmm = x;
         return *this;
      }

      /**
       @brief Type cast operator to convert to __m128 used in intrinsics
       */
      operator __m128() const
      {
         return xmm;
      }


      /**
       @brief Member function to load from array (unaligned)
       */
      Vec4f& load( float const* p )
      {
         xmm = _mm_loadu_ps( p );
         return *this;
      }

      /**
       @brief Member function to load from array, aligned by 16
       
          "load_a" is faster than "load" on older Intel processors (Pentium 4, Pentium M, Core 1,
          Merom, Wolfdale) and Atom, but not on other processors from Intel, AMD or VIA.
          You may use load_a instead of load if you are certain that p points to an address
          divisible by 16.
        */
      Vec4f& load_a( float const * p )
      {
         xmm = _mm_load_ps( p );
         return *this;
      }

      /**
       @brief Member function to store into array (unaligned)
       */
      void store( float* p ) const
      {
         _mm_storeu_ps(p, xmm);
      }

      /**
       @brief Member function to store into array, aligned by 16

         "store_a" is faster than "store" on older Intel processors (Pentium 4, Pentium M, Core 1,
         Merom, Wolfdale) and Atom, but not on other processors from Intel, AMD or VIA.
         You may use store_a instead of store if you are certain that p points to an address
         divisible by 16.
       */
      void store_a( float * p ) const
      {
         _mm_store_ps( p, xmm );
      }

      /**
       @brief Partial load. Load n elements and set the rest to 0
       */
      Vec4f& load_partial( int n, float const * p )
      {
         __m128 t1, t2;
         switch ( n )
         {
            case 1:
               xmm = _mm_load_ss(p);
               break;
            case 2:
               xmm = _mm_castpd_ps( _mm_load_sd( (double*) p ) );
               break;
            case 3:
               t1 = _mm_castpd_ps( _mm_load_sd( (double*) p ) );
               t2 = _mm_load_ss( p + 2 );
               xmm = _mm_movelh_ps( t1, t2 );
               break;
            case 4:
               load( p );
               break;
            default:
               xmm = _mm_setzero_ps();
         }
         return *this;
      }

      /**
       @brief Partial store. Store n elements
       */
      void store_partial( int n, float * p ) const
      {
      __m128 t1;
         switch ( n )
         {
            case 1:
               _mm_store_ss( p, xmm );
               break;
            case 2:
               _mm_store_sd( (double*) p, _mm_castps_pd( xmm ) );
               break;
            case 3:
               _mm_store_sd( (double*)p, _mm_castps_pd( xmm ) );
               t1 = _mm_movehl_ps( xmm, xmm );
               _mm_store_ss( p + 2, t1 );
               break;
            case 4:
               store( p );
               break;
            default:
               ;
         }
      }
        
      /**
       @brief Member function extract a single element from vector
       */
      float extract( size_t index ) const
      {
         assert( index < 4 );
         float x[ 4 ];
         store( x );
         return x[ index ];
      }

   protected:
      __m128 xmm; // Float vector
   };

    // vector operator + : add element by element
   static inline Vec4f operator+ ( Vec4f const & a, Vec4f const & b )
   {
      return _mm_add_ps(a, b);
   }

   // vector operator + : add vector and scalar
   static inline Vec4f operator+ (Vec4f const & a, float b)
   {
      return a + Vec4f( b );
   }

   static inline Vec4f operator+ ( float a, Vec4f const & b )
   {
      return Vec4f( a ) + b;
   }

   // vector operator += : add
   static inline Vec4f& operator+= ( Vec4f & a, Vec4f const & b )
   {
      a = a + b;
      return a;
   }

   // postfix operator ++
   static inline Vec4f operator++ ( Vec4f & a, int )
   {
      Vec4f a0 = a;
      a = a + 1.0f;
      return a0;
   }

   // prefix operator ++
   static inline Vec4f& operator++ (Vec4f & a)
   {
      a = a + 1.0f;
      return a;
   }

   // vector operator - : subtract element by element
   static inline Vec4f operator-( Vec4f const & a, Vec4f const & b )
   {
      return _mm_sub_ps( a, b );
   }

   // vector operator - : subtract vector and scalar
   static inline Vec4f operator- ( Vec4f const & a, float b )
   {
      return a - Vec4f( b );
   }

   static inline Vec4f operator- ( float a, Vec4f const & b )
   {
      return Vec4f( a ) - b;
   }

   // vector operator - : unary minus
   // Change sign bit, even for 0, INF and NAN
   static inline Vec4f operator- ( Vec4f const & a )
   {
      return _mm_xor_ps( a, _mm_castsi128_ps( _mm_set1_epi32( 0x80000000 ) ) );
   }

   // vector operator -= : subtract
   static inline Vec4f& operator-= ( Vec4f & a, Vec4f const & b )
   {
      a = a - b;
      return a;
   }

   // postfix operator --
   static inline Vec4f operator-- ( Vec4f & a, int )
   {
      Vec4f a0 = a;
      a = a - 1.0f;
      return a0;
   }

   // prefix operator --
   static inline Vec4f& operator-- ( Vec4f & a )
   {
      a = a - 1.0f;
      return a;
   }

   // vector operator * : multiply element by element
   static inline Vec4f operator* ( Vec4f const & a, Vec4f const & b )
   {
      return _mm_mul_ps( a, b );
   }

   // vector operator * : multiply vector and scalar
   static inline Vec4f operator* ( Vec4f const & a, float b )
   {
      return a * Vec4f( b );
   }
   static inline Vec4f operator* (float a, Vec4f const & b)
   {
      return Vec4f( a ) * b;
   }

   // vector operator *= : multiply
   static inline Vec4f& operator*= (Vec4f& a, Vec4f const & b)
   {
      a = a * b;
      return a;
   }

   // vector operator / : divide all elements by same integer
   static inline Vec4f operator/ ( Vec4f const & a, Vec4f const & b )
   {
      return _mm_div_ps( a, b );
   }

   // vector operator / : divide vector and scalar
   static inline Vec4f operator/ ( Vec4f const & a, float b )
   {
      return a / Vec4f( b );
   }
   static inline Vec4f operator / ( float a, Vec4f const & b )
   {
      return Vec4f( a ) / b;
   }

   // vector operator /= : divide
   static inline Vec4f& operator /= ( Vec4f & a, Vec4f const & b )
   {
      a = a / b;
      return a;
   }

   // vector operator == : returns true for elements for which a == b
   static inline Vec4fb operator == (Vec4f const & a, Vec4f const & b)
   {
      return _mm_cmpeq_ps(a, b);
   }

   // vector operator != : returns true for elements for which a != b
   static inline Vec4fb operator != (Vec4f const & a, Vec4f const & b)
   {
      return _mm_cmpneq_ps(a, b);
   }

   // vector operator < : returns true for elements for which a < b
   static inline Vec4fb operator < (Vec4f const & a, Vec4f const & b) 
   {
      return _mm_cmplt_ps(a, b);
   }

   // vector operator <= : returns true for elements for which a <= b
   static inline Vec4fb operator <= (Vec4f const & a, Vec4f const & b) 
   {
      return _mm_cmple_ps(a, b);
   }

   // vector operator > : returns true for elements for which a > b
   static inline Vec4fb operator > (Vec4f const & a, Vec4f const & b) 
   {
      return b < a;
   }

   // vector operator >= : returns true for elements for which a >= b
   static inline Vec4fb operator >= (Vec4f const & a, Vec4f const & b) 
   {
      return b <= a;
   }

   // Bitwise logical operators

   // vector operator & : bitwise and
   static inline Vec4f operator & (Vec4f const & a, Vec4f const & b) 
   {
      return _mm_and_ps(a, b);
   }

   // vector operator &= : bitwise and
   static inline Vec4f & operator &= (Vec4f & a, Vec4f const & b) 
   {
      a = a & b;
      return a;
   }

   // vector operator | : bitwise or
   static inline Vec4f operator | (Vec4f const & a, Vec4f const & b) 
   {
      return _mm_or_ps(a, b);
   }

   // vector operator |= : bitwise or
   static inline Vec4f & operator |= (Vec4f & a, Vec4f const & b) 
   {
      a = a | b;
      return a;
   }

   // vector operator ^ : bitwise xor
   static inline Vec4f operator ^ (Vec4f const & a, Vec4f const & b) 
   {
      return _mm_xor_ps(a, b);
   }

   // vector operator ^= : bitwise xor
   static inline Vec4f& operator ^= (Vec4f & a, Vec4f const & b) 
   {
      a = a ^ b;
      return a;
   }

   // vector operator ! : logical not. Returns Boolean vector
   static inline Vec4fb operator ! (Vec4f const & a) 
   {
      return a == 0.0f;
   }

   // Horizontal add: Calculates the sum of all vector elements.
   static inline float horizontal_add ( Vec4f const & a )
   {
   #if  NLL_INSTRUCTION_SET >= 3  // SSE3
      __m128 t1 = _mm_hadd_ps(a,a);
      __m128 t2 = _mm_hadd_ps(t1,t1);
      return _mm_cvtss_f32(t2);        
   #else
      __m128 t1 = _mm_movehl_ps(a,a);
      __m128 t2 = _mm_add_ps(a,t1);w
      __m128 t3 = _mm_shuffle_ps(t2,t2,1);
      __m128 t4 = _mm_add_ss(t2,t3);
      return _mm_cvtss_f32(t4);
   #endif
   }

   // function max: a > b ? a : b
   static inline Vec4f max(Vec4f const & a, Vec4f const & b)
   {
      return _mm_max_ps(a,b);
   }

   // function min: a < b ? a : b
   static inline Vec4f min(Vec4f const & a, Vec4f const & b)
   {
      return _mm_min_ps(a,b);
   }

   // function abs: absolute value
   // Removes sign bit, even for -0.0f, -INF and -NAN
   static inline Vec4f abs(Vec4f const & a)
   {
      __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
      return _mm_and_ps(a,mask);
   }

   // function sqrt: square root
   static inline Vec4f sqrt(Vec4f const & a)
   {
       return _mm_sqrt_ps(a);
   }

   // function square: a * a
   static inline Vec4f square(Vec4f const & a)
   {
       return a * a;
   }

   // pow(Vec4f, int):
   // Raise floating point numbers to integer power n
   static inline Vec4f pow(Vec4f const & a, int n)
   {
      Vec4f x = a;                       // a^(2^i)
      Vec4f y(1.0f);                     // accumulator
      if (n >= 0)
      {                                // make sure n is not negative
         while (true)
         {                             // loop for each bit in n
            if (n & 1) y *= x;         // multiply if bit = 1
            n >>= 1;                   // get next bit of n
            if (n == 0) return y;      // finished
            x *= x;                    // x = a^2, a^4, a^8, etc.
         }
      }
      else
      {                                   // n < 0
         return Vec4f(1.0f)/pow(x,-n);    // reciprocal
      }
   }

   // Raise floating point numbers to integer power n, where n is a compile-time constant
   template <int n>
   static inline Vec4f pow_n(Vec4f const & a) {
       if (n < 0)    return Vec4f(1.0f) / pow_n<-n>(a);
       if (n == 0)   return Vec4f(1.0f);
       if (n >= 256) return pow(a, n);
       Vec4f x = a;                       // a^(2^i)
       Vec4f y;                           // accumulator
       const int lowest = n - (n & (n-1));// lowest set bit in n
       if (n & 1) y = x;
       if (n < 2) return y;
       x = x*x;                           // x^2
       if (n & 2) {
           if (lowest == 2) y = x; else y *= x;
       }
       if (n < 4) return y;
       x = x*x;                           // x^4
       if (n & 4) {
           if (lowest == 4) y = x; else y *= x;
       }
       if (n < 8) return y;
       x = x*x;                           // x^8
       if (n & 8) {
           if (lowest == 8) y = x; else y *= x;
       }
       if (n < 16) return y;
       x = x*x;                           // x^16
       if (n & 16) {
           if (lowest == 16) y = x; else y *= x;
       }
       if (n < 32) return y;
       x = x*x;                           // x^32
       if (n & 32) {
           if (lowest == 32) y = x; else y *= x;
       }
       if (n < 64) return y;
       x = x*x;                           // x^64
       if (n & 64) {
           if (lowest == 64) y = x; else y *= x;
       }
       if (n < 128) return y;
       x = x*x;                           // x^128
       if (n & 128) {
           if (lowest == 128) y = x; else y *= x;
       }
       return y;
   }

   // avoid unsafe optimization in function round
   #if defined(__GNUC__) && !defined(__INTEL_COMPILER) && NLL_INSTRUCTION_SET < NLL_INSTRUCTION_SET_SSE41
   static inline Vec4f round(Vec4f const & a) __attribute__ ((optimize("-fno-unsafe-math-optimizations")));
   #elif (defined (_MSC_VER) || defined(__INTEL_COMPILER)) && NLL_INSTRUCTION_SET < NLL_INSTRUCTION_SET_SSE41
   #pragma float_control(push) 
   #pragma float_control(precise,on)
   #endif

   // function round: round to nearest integer (even). (result as float vector)
   static inline Vec4f round(Vec4f const & a) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41
       return _mm_round_ps(a, 0);
   #else // SSE2. Use magic number method
       // Note: assume MXCSR control register is set to rounding
       // (don't use conversion to int, it will limit the value to +/- 2^31)
       Vec4f signmask    = _mm_castsi128_ps(constant4i<0x80000000,0x80000000,0x80000000,0x80000000>());  // -0.0
       Vec4f magic       = _mm_castsi128_ps(constant4i<0x4B000000,0x4B000000,0x4B000000,0x4B000000>());  // magic number = 2^23
       Vec4f sign        = _mm_and_ps(a, signmask);                                    // signbit of a
       Vec4f signedmagic = _mm_or_ps(magic, sign);                                     // magic number with sign of a
       return a + signedmagic - signedmagic;                                           // round by adding magic number
   #endif
   }
   #if (defined (_MSC_VER) || defined(__INTEL_COMPILER)) && NLL_INSTRUCTION_SET < NLL_INSTRUCTION_SET_SSE41
   #pragma float_control(pop)
   #endif

   // function truncate: round towards zero. (result as float vector)
   static inline Vec4f truncate(Vec4f const & a) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41
       return _mm_round_ps(a, 3);
   #else  // SSE2. Use magic number method (conversion to int would limit the value to 2^31)
       ui32 t1 = _mm_getcsr();        // MXCSR
       ui32 t2 = t1 | (3 << 13);      // bit 13-14 = 11
       _mm_setcsr(t2);                    // change MXCSR
       Vec4f r = round(a);                // use magic number method
       _mm_setcsr(t1);                    // restore MXCSR
       return r;
   #endif
   }

   // truncate but returns integers
   static inline Vec4i truncatei(Vec4f const & a)
   {
      return Vec4i( _mm_cvttps_epi32( a ) );
   }

   // function floor: round towards minus infinity. (result as float vector)
   static inline Vec4f floor(Vec4f const & a) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41
       return _mm_round_ps(a, 1);
   #else  // SSE2. Use magic number method (conversion to int would limit the value to 2^31)
       ui32 t1 = _mm_getcsr();        // MXCSR
       ui32 t2 = t1 | (1 << 13);      // bit 13-14 = 01
       _mm_setcsr(t2);                    // change MXCSR
       Vec4f r = round(a);                // use magic number method
       _mm_setcsr(t1);                    // restore MXCSR
       return r;
   #endif
   }

   // function ceil: round towards plus infinity. (result as float vector)
   static inline Vec4f ceil(Vec4f const & a) {
   #if NLL_INSTRUCTION_SET >= NLL_INSTRUCTION_SET_SSE41
       return _mm_round_ps(a, 2);
   #else  // SSE2. Use magic number method (conversion to int would limit the value to 2^31)
       ui32 t1 = _mm_getcsr();        // MXCSR
       ui32 t2 = t1 | (2 << 13);      // bit 13-14 = 10
       _mm_setcsr(t2);                    // change MXCSR
       Vec4f r = round(a);                // use magic number method
       _mm_setcsr(t1);                    // restore MXCSR
       return r;
   #endif
   }
}
}

#endif
