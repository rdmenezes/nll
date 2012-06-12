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

#ifndef NLL_UTILITY_H_
# define NLL_UTILITY_H_

#include <cstring>

#pragma warning( push )
#pragma warning( disable:4996 ) // strdup deprecated

namespace nll
{
namespace core
{
   // provide our own strdup for GCC cygwin as described here:
   // http://stackoverflow.com/questions/5573775/strdup-error-on-g-with-c0x
   char* _strdup(const char *str)
   {
      size_t len = std::strlen( str );
      char *x = (char*)std::malloc( len + 1 );
      if( !x )
         return NULL;
      std::memcpy( x, str, len + 1 );
      return x;
   }

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
    @brief helper function for making a std::vector out of a set of values
   */
   template <class T>
   inline std::vector<T> make_vector( typename BestConstArgType<T>::type val1,
                                      typename BestConstArgType<T>::type val2,
                                      typename BestConstArgType<T>::type val3,
                                      typename BestConstArgType<T>::type val4,
                                      typename BestConstArgType<T>::type val5,
                                      typename BestConstArgType<T>::type val6 )
   {
      std::vector<T> vec( 6 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      vec[ 2 ] = val3;
      vec[ 3 ] = val4;
      vec[ 4 ] = val5;
      vec[ 5 ] = val6;
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
                                      typename BestConstArgType<T>::type val5,
                                      typename BestConstArgType<T>::type val6,
                                      typename BestConstArgType<T>::type val7 )
   {
      std::vector<T> vec( 7 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      vec[ 2 ] = val3;
      vec[ 3 ] = val4;
      vec[ 4 ] = val5;
      vec[ 5 ] = val6;
      vec[ 6 ] = val7;
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
                                      typename BestConstArgType<T>::type val5,
                                      typename BestConstArgType<T>::type val6,
                                      typename BestConstArgType<T>::type val7,
                                      typename BestConstArgType<T>::type val8 )
   {
      std::vector<T> vec( 8 );
      vec[ 0 ] = val1;
      vec[ 1 ] = val2;
      vec[ 2 ] = val3;
      vec[ 3 ] = val4;
      vec[ 4 ] = val5;
      vec[ 5 ] = val6;
      vec[ 6 ] = val7;
      vec[ 7 ] = val8;
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
   template <class T> Buffer1D<T> make_buffer1D( typename BestConstArgType<T>::type val1,
												             typename BestConstArgType<T>::type val2,
												             typename BestConstArgType<T>::type val3,
                                                 typename BestConstArgType<T>::type val4,
                                                 typename BestConstArgType<T>::type val5,
                                                 typename BestConstArgType<T>::type val6 )
   {
      Buffer1D<T> buf( 6, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      buf( 4 ) = val5;
      buf( 5 ) = val6;
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
                                                 typename BestConstArgType<T>::type val5,
                                                 typename BestConstArgType<T>::type val6,
                                                 typename BestConstArgType<T>::type val7 )
   {
      Buffer1D<T> buf( 7, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      buf( 4 ) = val5;
      buf( 5 ) = val6;
      buf( 6 ) = val7;
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
                                                 typename BestConstArgType<T>::type val5,
                                                 typename BestConstArgType<T>::type val6,
                                                 typename BestConstArgType<T>::type val7,
                                                 typename BestConstArgType<T>::type val8 )
   {
      Buffer1D<T> buf( 8, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      buf( 4 ) = val5;
      buf( 5 ) = val6;
      buf( 6 ) = val7;
      buf( 7 ) = val8;
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
                                                 typename BestConstArgType<T>::type val5,
                                                 typename BestConstArgType<T>::type val6,
                                                 typename BestConstArgType<T>::type val7,
                                                 typename BestConstArgType<T>::type val8,
                                                 typename BestConstArgType<T>::type val9 )
   {
      Buffer1D<T> buf( 9, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      buf( 4 ) = val5;
      buf( 5 ) = val6;
      buf( 6 ) = val7;
      buf( 7 ) = val8;
      buf( 8 ) = val9;
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
                                                 typename BestConstArgType<T>::type val5,
                                                 typename BestConstArgType<T>::type val6,
                                                 typename BestConstArgType<T>::type val7,
                                                 typename BestConstArgType<T>::type val8,
                                                 typename BestConstArgType<T>::type val9,
                                                 typename BestConstArgType<T>::type val10 )
   {
      Buffer1D<T> buf( 10, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      buf( 4 ) = val5;
      buf( 5 ) = val6;
      buf( 6 ) = val7;
      buf( 7 ) = val8;
      buf( 8 ) = val9;
      buf( 9 ) = val10;
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
                                                 typename BestConstArgType<T>::type val5,
                                                 typename BestConstArgType<T>::type val6,
                                                 typename BestConstArgType<T>::type val7,
                                                 typename BestConstArgType<T>::type val8,
                                                 typename BestConstArgType<T>::type val9,
                                                 typename BestConstArgType<T>::type val10,
                                                 typename BestConstArgType<T>::type val11 )
   {
      Buffer1D<T> buf( 11, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      buf( 4 ) = val5;
      buf( 5 ) = val6;
      buf( 6 ) = val7;
      buf( 7 ) = val8;
      buf( 8 ) = val9;
      buf( 9 ) = val10;
      buf( 10 ) = val11;
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
                                                 typename BestConstArgType<T>::type val5,
                                                 typename BestConstArgType<T>::type val6,
                                                 typename BestConstArgType<T>::type val7,
                                                 typename BestConstArgType<T>::type val8,
                                                 typename BestConstArgType<T>::type val9,
                                                 typename BestConstArgType<T>::type val10,
                                                 typename BestConstArgType<T>::type val11,
                                                 typename BestConstArgType<T>::type val12 )
   {
      Buffer1D<T> buf( 12, false );
      buf( 0 ) = val1;
      buf( 1 ) = val2;
      buf( 2 ) = val3;
      buf( 3 ) = val4;
      buf( 4 ) = val5;
      buf( 5 ) = val6;
      buf( 6 ) = val7;
      buf( 7 ) = val8;
      buf( 8 ) = val9;
      buf( 9 ) = val10;
      buf( 10 ) = val11;
      buf( 11 ) = val12;
      return buf;
   }

   /**
    @ingroup core
    @brief helper function for making a Buffer1D out of a set of values
   */
   inline Buffer1D<i8> make_buffer1D_from_string( const std::string& str )
   {
      Buffer1D<i8> buf( _strdup( str.c_str() ), static_cast<ui32> ( str.size() ), true );
      return buf;
   }

   /**
    @ingroup core
    @brief helper function for making a string out of a string embeded in a Buffer1D
   */
   inline std::string string_from_Buffer1D( const Buffer1D<i8>& str )
   {
      if ( str.size() == 0 )
         return "";

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
    @ingroup core
    @brief Generate a unique list of identifier of size start - end + 1.

    For example generateUniqueList( 4, 7 ) could generate the list [ 5, 4, 6, 7 ]
    */
   inline std::vector<ui32> generateUniqueList( ui32 start, ui32 end )
   {
      assert( start <= end );
      ui32 size = end - start + 1;
      std::vector<ui32> list( end - start + 1 );
      for ( ui32 n = 0; n < size; ++n )
         list[ n ] = n + start;
      core::shuffleFisherYates( list );
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
