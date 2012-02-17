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

#ifndef NLL_GENERIC_OPERATIONS_H_
# define NLL_GENERIC_OPERATIONS_H_

# include "math.h"
# include "type-traits.h"

# pragma warning( push )
# pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class T2>
   inline void generic_add( typename BestArgType<T>::type dst, typename BestConstArgType<T2>::type src, const int size )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] += src[ n ];
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class T2>
   inline void generic_sub( typename BestArgType<T>::type dst, typename BestConstArgType<T2>::type src, const int size )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] -= src[ n ];
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class T2>
   inline void generic_div( typename BestArgType<T>::type dst, typename BestConstArgType<T2>::type src, const int size )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] /= src[ n ];
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class T2>
   inline void generic_mul( typename BestArgType<T>::type dst, typename BestConstArgType<T2>::type src , const int size )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] *= src[ n ];
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T>
   inline void generic_div_cte( typename BestArgType<T>::type dst, const double val, const int size )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] /= val;
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T>
   inline void generic_mul_cte( typename BestArgType<T>::type dst, const double val, const int size )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] *= val;
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class T2, int size>
   inline void generic_add( typename BestArgType<T>::type dst, typename BestConstArgType<T2>::type src )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] += src[ n ];
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class T2, int size>
   inline void generic_sub( typename BestArgType<T>::type dst, typename BestConstArgType<T2>::type src )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] -= src[ n ];
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class T2, int size>
   inline void generic_div( typename BestArgType<T>::type dst, typename BestConstArgType<T2>::type src )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] /= src[ n ];
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class T2, int size>
   inline void generic_mul( typename BestArgType<T>::type dst, typename BestConstArgType<T2>::type src )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] *= src[ n ];
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, int size>
   inline void generic_div_cte( typename BestArgType<T>::type dst, const double val )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] /= val;
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, int size>
   inline void generic_mul_cte( typename BestArgType<T>::type dst, const double val )
   {
      for ( int n = 0; n < size; ++n )
         dst[ n ] *= val;
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class Result, int size>
   inline Result generic_norm2( typename BestConstArgType<T>::type buf )
   {
      Result accum = 0;
      for ( int n = 0; n < size; ++n )
         accum += static_cast<Result> ( buf[ n ] * buf[ n ] );
      return sqrt( accum );
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class Result, int size>
   inline Result generic_norm2( typename BestConstArgType<T>::type buf1, typename BestConstArgType<T>::type buf2 )
   {
      Result accum = 0;
      for ( int n = 0; n < size; ++n )
      {
         Result val = buf1[ n ] - buf2[ n ];
         accum += val * val;
      }
      return sqrt( accum );
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class Result>
   inline Result generic_norm2( typename BestConstArgType<T>::type buf1, typename BestConstArgType<T>::type buf2, int size )
   {
      Result accum = 0;
      for ( int n = 0; n < size; ++n )
      {
         Result val = buf1[ n ] - buf2[ n ];
         accum += val * val;
      }
      return sqrt( accum );
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class Result>
   inline Result generic_dot( typename BestConstArgType<T>::type buf1, typename BestConstArgType<T>::type buf2, int size )
   {
      Result accum = 0;
      for ( int n = 0; n < size; ++n )
      {
         Result val = buf1[ n ] - buf2[ n ];
         accum += val * val;
      }
      return accum;
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]
    */
   template <class T, class Result>
   inline Result generic_norm2( typename BestConstArgType<T>::type buf, int size )
   {
      Result accum = 0;
      for ( int n = 0; n < size; ++n )
         accum += static_cast<Result> ( buf[ n ] * buf[ n ] );
      return sqrt( accum );
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]. Set each element to null
    */
   template <class T, int size>
   inline T generic_null( )
   {
      T buf( size );
      for ( int n = 0; n < size; ++n )
         buf[ n ] = 0;
      return buf;
   }

   /**
    @ingroup core
    @brief generic operation, target types must define operator[]. Set each element to null
    */
   template <class T>
   inline T generic_null( ui32 size )
   {
      T buf( size );
      for ( ui32 n = 0; n < size; ++n )
         buf[ n ] = 0;
      return buf;
   }
}
}

#pragma warning( pop )

#endif
