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

#ifndef NLL_TYPE_TRAITS_H_
# define NLL_TYPE_TRAITS_H_

# include <limits.h>
# include <float.h>
# include "types.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief create a unique type from a value
    */
   template <int N>
   struct Val2Type
   {
      enum{ value = N };
   };

   /**
    @ingroup core
    @brief unconstify a type
   */
   template <class T>
   struct Unconstify
   {
      typedef T   type;
   };

   template <class T>
   struct Unconstify<const T>
   {
      typedef T   type;
   };

   /**
    @ingroup core
    @brief constify a type
   */
   template<class T>
   struct Constify
   {
     typedef const T type;
   };

   template<class T>
   struct Constify<T&>
   {
     typedef const T& type;
   };

   template <class T>
   struct _IsNativeType
   {
      enum {value = false};
   };

   template <class T>
   struct _IsNativeType<T*>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<bool>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<ui8>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<i8>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<ui16>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<i16>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<ui32>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<i32>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<f32>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<f64>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<int>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<unsigned int>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<unsigned long long>
   {
      enum {value = true};
   };

   template <>
   struct _IsNativeType<long long>
   {
      enum {value = true};
   };

   /**
    @ingroup core
    @brief define if the type is a native type

    Value == 1 if T is native, else 0
   */
   template <class T>
   struct IsNativeType
   {
      enum {value = _IsNativeType<typename Unconstify<T>::type>::value};
   };

   template <class T, bool isNative>
   struct _BestReturnType
   {
      typedef  T&    type;
   };

   template <class T>
   struct _BestReturnType<T, true>
   {
      typedef  T    type;
   };

   /**
    @ingroup core
    @brief Determine what type is the best as a return type according to T
   */
   template <class T>
   struct BestReturnType
   {
      typedef typename _BestReturnType<T, IsNativeType<T>::value>::type    type;
   };

   /**
    @ingroup core
    @brief Determine what type is the best as a return type as a constant type according to T
    @todo check! ->seems BestConstReturnType<char*> = char * const and not const char *
   */
   template <class T>
   struct BestConstReturnType
   {
      typedef typename Constify<typename _BestReturnType<T, IsNativeType<T>::value>::type>::type    type;
   };

   template <class T>
   struct BestConstArgType
   {
      typedef typename Constify<typename _BestReturnType<T, IsNativeType<T>::value>::type>::type    type;
   };

   template <class T>
   struct BestArgType
   {
      typedef typename _BestReturnType<T, IsNativeType<T>::value>::type    type;
   };

 
   /**
    @ingroup core
    @brief Determine if 2 types are equal

    value == 1 if equal, else 0
   */
   template <class U, class V>
   struct Equal{enum {value = false};};

   template <class U>
   struct Equal<U, U>{enum {value = true};};

   /**
    @ingroup core
    @brief Define a bigger type of T ans some range values [max is UINT_MAX because returned in an enum]
   */
   template <class T>
   struct BiggerType
   {
	   typedef f64		type;
	   typedef f64		signed_type;
	   //enum {min = 0, max = 255, size = 256};
   };

   template <>
   struct BiggerType<ui8>
   {
	   typedef ui16	type;
	   typedef i16		signed_type;
	   enum {min = 0, max = 255, size = 256};
   };

   template <>
   struct BiggerType<i8>
   {
	   typedef i16		type;
	   typedef i16		signed_type;
	   enum {min = -128, max = 127, size = 256};
   };

   template <>
   struct BiggerType<ui16>
   {
	   typedef ui32	type;
	   typedef i32		signed_type;
	   enum {min = 0, max = 65535, size = 65536};
   };

   template <>
   struct BiggerType<i16>
   {
	   typedef i32	type;
      typedef i32	signed_type;
	   enum {min = -32768, max = 32767, size = 65536};
   };

   template <class U, class V, bool COND>
   struct If
   {
   };

   template <class U, class V>
   struct If<U, V, true>
   {
      typedef U   type;
   };

   template <class U, class V>
   struct If<U, V, false>
   {
      typedef V   type;
   };

   template <class T>
   struct IsFloatingType
   {
      enum {value = false};
   };

   template <>
   struct IsFloatingType<float>
   {
      enum {value = true};
   };

   template <>
   struct IsFloatingType<double>
   {
      enum {value = true};
   };

      template <class T>
   struct IsIntegral
   {
      enum {value = false};
   };

   template <>
   struct IsIntegral<char>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<unsigned char>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<short>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<unsigned short>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<int>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<unsigned int>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<long>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<unsigned long>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<long long>
   {
      enum {value = true};
   };

   template <>
   struct IsIntegral<unsigned long long>
   {
      enum {value = true};
   };

   template <class T>
   struct IsFloating
   {
      enum {value = false};
   };

   template <>
   struct IsFloating<f32>
   {
      enum {value = true};
   };

   template <>
   struct IsFloating<f64>
   {
      enum {value = true};
   };
}
}

#endif
