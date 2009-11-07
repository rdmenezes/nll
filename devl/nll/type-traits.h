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

   /**
    @ingroup core
    @brief Determine the bounding values of a type. [ max is INT_MAX ]
   */
   template <class T>
   struct Bound
   {};

   template <>
   struct Bound<ui8>
   {
      enum {min = 0, max = 255};
   };

   template <>
   struct Bound<i8>
   {
      enum {min = -128, max = 127};
   };

   template <>
   struct Bound<ui16>
   {
      enum {min = 0, max = 65535};
   };

   template <>
   struct Bound<i16>
   {
      enum {min = -32768, max = 32767};
   };

   template <>
   struct Bound<i32>
   {
      enum {min = LONG_MIN, max = LONG_MAX};
   };

   template <>
   struct Bound<ui32>
   {
      enum {min = 0, max = ULONG_MAX};
   };

   template <>
   struct Bound<f32>
   {
      enum {min = LONG_MIN, max = LONG_MAX};
   };

   template <>
   struct Bound<f64>
   {
      enum {min = LONG_MIN, max = LONG_MAX};
   };

   template <>
   struct Bound<int>
   {
      enum {min = INT_MIN, max = INT_MAX};
   };

   template <>
   struct Bound<unsigned int>
   {
      enum {min = 0, max = UINT_MAX};
   };

   template <>
   struct Bound<bool>
   {
      enum {min = 0, max = 1};
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
}
}

#endif
