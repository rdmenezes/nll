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

#ifndef NLL_IO_H_
# define NLL_IO_H_

# include <iostream>
# include <vector>
# include "type-traits.h"

namespace nll
{
namespace core
{
   template <class T> void write( typename BestConstArgType<T>::type val, std::ostream& f );
   template <class T> void read( T& val, std::istream& f );

   /**
    @ingroup core
    @brief write plain data [native types only!!] to a stream
    */
   template <class T>
   void writePlainData( const T& data, std::ostream& f )
   {
      f.write( ( i8* )&data, sizeof( T ) );
   }

   /**
    @ingroup core
    @brief read plain data [native types only!!] to a stream
    */
   template <class T>
   void readPlainData( T& out_data, std::istream& f )
   {
      f.read( ( i8* )&out_data, sizeof( T ) );
   }

   template <class T, bool isNative>
   struct _write
   {
      _write( typename BestConstArgType<T>::type val, std::ostream& f )
      {
         val.write( f );
      }
   };

   template <class T>
   struct _write<T, true>
   {
      _write( typename BestConstArgType<T>::type val, std::ostream& f )
      {
         f.write( (i8*)( &val ), sizeof ( val ) );
      }
   };

   template <class T>
   struct _write<std::vector<T>, false>
   {
      _write( const std::vector<T>& val, std::ostream& f )
      {
         size_t size = static_cast<size_t>( val.size() );
         write<size_t>( size, f );
         for ( size_t n = 0; n < val.size(); ++n )
            write<T>( val[ n ], f );
      }
   };

   /**
    @ingroup core
    @brief write data to a stream. If native type, write it using stream functions, else the type needs to provide write()
    */
   template <class T> void write( typename BestConstArgType<T>::type val, std::ostream& f )
   {
      _write<T, IsNativeType<T>::value>(val, f);
   }

   template <class T, bool isNative>
   struct _read
   {
      _read( T& val, std::istream& f)
      {
         val.read( f );
      }
   };

   template <class T>
   struct _read<T, true>
   {
      _read( T& val, std::istream& f)
      {
         f.read( (i8*)( &val ), sizeof ( val ) );
      }
   };

   template <class T>
   struct _read<std::vector<T>, false>
   {
      _read( std::vector<T>& val, std::istream& i )
      {
         size_t size = 0;
         read<size_t>( size, i );
         //assert( size );
         val = std::vector<T>( size );
         for ( size_t n = 0; n < size; ++n )
            read<T>( val[ n ], i );
      }
   };

   template <>
   struct _write<std::string, false>
   {
      _write( const std::string& val, std::ostream& f )
      {
         size_t size = static_cast<size_t>( val.size() );
         write<size_t>( size, f );
         f.write( val.c_str(), size );
      }
   };

   template <>
   struct _read<std::string, false>
   {
      _read( std::string& val, std::istream& i )
      {
         size_t size = 0;
         read<size_t>( size, i );
         val = std::string( size, ' ' );
         i.read( &val[ 0 ], size );
      }
   };

   /**
    @ingroup core
    @brief write data to a stream. If native type, write it using stream functions, else the type needs to provide write()
    */
   template <class T> void read( T& val, std::istream& f )
   {
      _read<T, IsNativeType<T>::value>(val, f);
   }
}
}

#endif
