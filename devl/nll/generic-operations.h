#ifndef NLL_GENERIC_OPERATIONS_H_
# define NLL_GENERIC_OPERATIONS_H_

# include "math.h"
# include "type-traits.h"

#pragma warning( push )
#pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

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
