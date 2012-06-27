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

#ifndef NLL_MATH_H_
# define NLL_MATH_H_

# include "nll.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define the PI constant
    */
   extern const double NLL_API PI;
   extern const float  NLL_API PIf;

   /**
    @ingroup core
    @brief Define the square root function to be used
    */
   template <class T>
   inline T sqr( const T val ){ return val * val; } 

   /**
    @ingroup core
    @brief Round a value to its closes integer
    */
   inline int round( double x )
   {
      return core::floor( x + 0.5 );
   }

   /**
    @ingroup core
    @brief return the sign of a value
    @return 1 for val >= 0, else -1
    */
   template <class T>
   T sign( const T v )
   {
      return ( v >= 0 ) ? 1 : -1;
   }

   /**
    @ingroup core
    @brief randomly generate a sign +/- with equal probability
    */
   inline int NLL_API generateSign()
   {
      ui32 n = rand() % 2;
      return n ? -1 : 1;
   }

   /**
    @ingroup core
    @brief encode a binary number to gray code
    */
   inline unsigned short binaryToGray( unsigned short num )
   {
      return ( num >> 1 ) ^ num;
   }
   
   /**
    @ingroup core
    @brief decode a gray code to binary number
    */
   inline unsigned short grayToBinary( unsigned short num )
   {
      unsigned short temp = num ^ ( num >> 8 );
      temp ^= ( temp >> 4 );
      temp ^= ( temp >> 2 );
      temp ^= ( temp >> 1 );
      return temp;
   }

   /**
    @ingroup core
    @brief compute the log base 2
    */
   template <class T>
   T log2( T v )
   {
      return std::log( v ) / std::log( static_cast<T>( 2 ) );
   }

   /**
    @ingroup core
    @param small a small factor
    @param big a bigger number than small
    @param tolerance a number, expressed in <big> unit
    */
   template <class T>
   bool isMultipleOf( T small, T big, T tolerance )
   {
      STATIC_ASSERT( std::numeric_limits<T>::is_signed );

      const double usmall = fabs( small );
      const double ubig = fabs( big );

      #ifdef NLL_SECURE
      ensure( usmall <= ubig, "small > big" );
      #endif

      const double ratio = ubig / usmall;
      const int ratioi = static_cast<int>( ratio );
      const double diff = ratio - ratioi;
      if ( diff > 0.5 )
      {
         const double reminder = ( 1 - diff ) * usmall;
         return reminder < tolerance;
      } else {
         const double reminder = diff * usmall;
         return reminder < tolerance;
      }
   }
}
}

#endif
