/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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

#ifndef NLL_CORE_ALLOCATOR_ALIGNED_H_
# define NLL_CORE_ALLOCATOR_ALIGNED_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Provide a 16 byte aligned memory allocator
    @note T must be a basic type only as malloc/free is used (thus object no constructed)
    */
   template <class T>
   class Allocator16ByteAligned
   {
   public:
      typedef T value_type;
      typedef value_type* pointer;
      typedef const value_type* const_pointer;
      typedef value_type& reference;
      typedef const value_type& const_reference;
      typedef std::size_t size_type;
      typedef std::ptrdiff_t difference_type;

   public:
      template<typename U>
      struct rebind {
         typedef Allocator16ByteAligned<U> other;
      };

   public:
      inline Allocator16ByteAligned()
      {
         STATIC_ASSERT( IsNativeType<T>::value );
      }

      inline ~Allocator16ByteAligned()
      {}

      inline Allocator16ByteAligned( Allocator16ByteAligned const& )
      {
      }

      template<typename U>
      inline Allocator16ByteAligned( Allocator16ByteAligned<U> const& )
      {
         STATIC_ASSERT( IsNativeType<T>::value );
      }

      inline pointer address( reference r )
      {
         return &r;
      }

      inline const_pointer address( const_reference r )
      {
         return &r;
      }

      inline pointer allocate( size_type cnt, typename std::allocator<void>::const_pointer = 0)
      { 
         // we are allocating 15 bytes to align data + a word to store original buffer location
         // we also store at the begining of the buffer the non aligned pointer
         // so we can deallocate it easily
         void* buf = malloc( sizeof( T ) * cnt + 15 + sizeof( void* ) );
         assert( buf );
         pointer ptr = reinterpret_cast<pointer>( reinterpret_cast<size_t> ( reinterpret_cast<char*>( buf ) + sizeof( void* ) + 15 ) & ~ (size_t) 0x0F );
         reinterpret_cast<void**>( ptr)[ -1 ] = buf;
         return ptr;
      }

      inline void deallocate(pointer p, size_type)
      { 
         free( reinterpret_cast<void**>( p )[ -1 ] );
      }
   };
}
}

#endif