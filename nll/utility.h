#ifndef NLL_UTILITY_H_
# define NLL_UTILITY_H_

# include <vector>
# include <limits>
# include <sstream>
# include "type-traits.h"

#pragma warning( push )
#pragma warning( disable:4996 ) // strdup deprecated

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief helper function for making a std::vector out of a set of values
   */
   template <class T>
   inline std::vector<T> make_vector( typename BestConstArgType<T>::type val1 )
   {
      std::vector<T> vec( 1 );
      vec[ 0 ] = val1;
      return vec;
   }

   /**
    @ingroup core
    @brief helper function for making a std::vector out of a set of values
   */
   template <class T>
   inline std::vector<T> make_vector( typename BestConstArgType<T>::type val1,
                                      typename BestConstArgType<T>::type val2)
   {
      std::vector<T> vec( 2 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      return vec;
   }

   /**
    @ingroup core
    @brief helper function for making a std::vector out of a set of values
   */
   template <class T>
   inline std::vector<T> make_vector( typename BestConstArgType<T>::type val1,
                                      typename BestConstArgType<T>::type val2,
                                      typename BestConstArgType<T>::type val3)
   {
      std::vector<T> vec( 3 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      vec[ 2 ] = val3;
      return vec;
   }

   /**
    @ingroup core
    @brief helper function for making a std::vector out of a set of values
   */
   template <class T>
   inline std::vector<T> make_vector( typename BestConstArgType<T>::type val1,
                                      typename BestConstArgType<T>::type val2,
                                      typename BestConstArgType<T>::type val3,
                                      typename BestConstArgType<T>::type val4)
   {
      std::vector<T> vec( 4 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      vec[ 2 ] = val3;
      vec[ 3 ] = val4;
      return vec;
   }

   /**
    @ingroup core
    @brief helper function for making a std::vector out of a set of values
   */
   template <class T>
   inline std::vector<T> make_vector( typename BestConstArgType<T>::type val1,
                                      typename BestConstArgType<T>::type val2,
                                      typename BestConstArgType<T>::type val3,
                                      typename BestConstArgType<T>::type val4,
                                      typename BestConstArgType<T>::type val5)
   {
      std::vector<T> vec( 5 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      vec[ 2 ] = val3;
      vec[ 3 ] = val4;
      vec[ 4 ] = val5;
      return vec;
   }

   /**
    @ingroup core
    @brief helper function for making a Buffer1D out of a set of values
   */
   template <class T> Buffer1D<T> make_buffer1D( typename BestConstArgType<T>::type val1 )
   {
      Buffer1D<T> buf( 1, false );
      buf( 0 ) = val1;
      return buf;
   }
   
   /**
    @ingroup core
    @brief helper function for making a Buffer1D out of a set of values
   */
   template <class T> Buffer1D<T> make_buffer1D( typename BestConstArgType<T>::type val1,
												 typename BestConstArgType<T>::type val2 )
   {
      Buffer1D<T> buf( 2, false );
      buf( 0 ) = val1;
	   buf( 1 ) = val2;
      return buf;
   }
   
   /**
    @ingroup core
    @brief helper function for making a Buffer1D out of a set of values
   */
   template <class T> Buffer1D<T> make_buffer1D( typename BestConstArgType<T>::type val1,
												 typename BestConstArgType<T>::type val2,
												 typename BestConstArgType<T>::type val3 )
   {
      Buffer1D<T> buf( 3, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      return buf;
   }

   /**
    @ingroup core
    @brief helper function for making a Buffer1D out of a set of values
   */
   template <class T> Buffer1D<T> make_buffer1D( typename BestConstArgType<T>::type val1,
												 typename BestConstArgType<T>::type val2,
												 typename BestConstArgType<T>::type val3,
                                     typename BestConstArgType<T>::type val4 )
   {
      Buffer1D<T> buf( 4, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      return buf;
   }

   /**
    @ingroup core
    @brief helper function for making a Buffer1D out of a set of values
   */
   template <class T> Buffer1D<T> make_buffer1D( typename BestConstArgType<T>::type val1,
												 typename BestConstArgType<T>::type val2,
												 typename BestConstArgType<T>::type val3,
                                     typename BestConstArgType<T>::type val4,
                                     typename BestConstArgType<T>::type val5 )
   {
      Buffer1D<T> buf( 5, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      buf( 4 ) = val5;
      return buf;
   }

   /**
    @ingroup core
    @brief helper function for making a Buffer1D out of a set of values
   */
   inline Buffer1D<i8> make_buffer1D_from_string( const std::string& str )
   {
      Buffer1D<i8> buf( strdup( str.c_str() ), static_cast<ui32> ( str.size() ), true );
      return buf;
   }

   /**
    @ingroup core
    @brief helper function for making a string out of a string embeded in a Buffer1D
   */
   inline std::string string_from_Buffer1D( const Buffer1D<i8>& str )
   {
      std::string s( str.getBuf(), str.size() );
      return s;
   }


   /**
    @ingroup core
    @brief helper function for making a static vector out of a set of values
   */
   template <class T, int SIZE>
   inline StaticVector<T, SIZE> make_static_vector( typename BestConstArgType<T>::type val1 )
   {
      StaticVector<T, SIZE> vec( 1 );
      vec[ 0 ] = val1;
      return vec;
   }

   /**
    @ingroup core
    @brief helper function for making a static vector out of a set of values
   */
   template <class T, int SIZE>
   inline StaticVector<T, SIZE> make_static_vector( typename BestConstArgType<T>::type val1,
                                                    typename BestConstArgType<T>::type val2)
   {
      StaticVector<T, SIZE> vec( 2 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      return vec;
   }

   /**
    @ingroup core
    @brief helper function for making a static vector out of a set of values
   */
   template <class T, int SIZE>
   inline StaticVector<T, SIZE> make_static_vector( typename BestConstArgType<T>::type val1,
                                                    typename BestConstArgType<T>::type val2,
                                                    typename BestConstArgType<T>::type val3)
   {
      StaticVector<T, SIZE> vec( 3 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      vec[ 2 ] = val3;
      return vec;
   }

   /**
    @brief Generate a unique list of identifier of size start - end + 1.

    For example generateUniqueList( 4, 7 ) could generate the list [ 5, 4, 6, 7 ]
    */
   inline std::vector<ui32> generateUniqueList( ui32 start, ui32 end, double permutationRate = 0.2 )
   {
      assert( start <= end );
      ui32 size = end - start + 1;
      std::vector<ui32> list( end - start + 1 );
      for ( ui32 n = 0; n < size; ++n )
         list[ n ] = n + start;
      for ( ui32 n = 0; n < permutationRate * size + 1; ++n )
      {
         ui32 n1 = rand() % size;
         ui32 n2 = rand() % size;
         std::swap( list[ n1 ], list[ n2 ] );
      }

      return list;
   }

   template <class T>
   Buffer1D<T> make_buffer1D( const std::vector<T>& v )
   {
      Buffer1D<T> buf( static_cast<ui32>( v.size() ) );
      for ( ui32 n = 0; n < static_cast<ui32>( v.size() ); ++n )
         buf[ n ] = v[ n ];
      return buf;
   }
}
}

#pragma warning( pop )

#endif
