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

#ifndef NLL_BUFFER1D_H_
# define NLL_BUFFER1D_H_

# include <limits>
# include <limits.h>
# include <assert.h>
# include "types.h"
# include "type-traits.h"
# include "index-mapper.h"
# include "io.h"


#ifdef NLL_FIND_MEMORY_LEAK
#undef new
#endif

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression is constant

//#define DEBUG_BUFFER1D

namespace nll
{
namespace core
{

/**
 @ingroup core
 @brief Define a 1D buffer. The buffers are shared using a reference counter.

 @param T type of the buffer
 @param IndexMapper1D define how the buffer is internally mapped in memory. By default it is simply linear it must
        define the function static ui32 index(const ui32)
 */
template <class T, class IndexMapper1D = IndexMapperFlat1D, class AllocatorT = std::allocator<T> >
class Buffer1D
{
public:
   /// the value of one element
   typedef  T              value_type;

   /// the index mapper
   typedef  IndexMapper1D  IndexMapper;

   /// the allocator used by this buffer
   typedef  AllocatorT     Allocator;

public:
   /**
    @brief contructs a buffer with a specified size
    @param zero is IsPOS<T>::value and if set to true, the buffer is cleared by zero, else undefined value.
                if !IsPOS<T>::value, this parameter is not used
    @param size the number of elements of the buffer
    */
   explicit Buffer1D( ui32 size, bool zero = true, Allocator allocator = Allocator() ) : _cpt( 0 ), _buffer ( 0 ), _size( 0 ), _ownsBuffer( true ), _allocator( allocator ) { _allocate( size, zero ); }

   /**
    @brief constructs an empty buffer.
    */
   Buffer1D( Allocator allocator = Allocator() ) : _cpt( 0 ), _buffer( 0 ), _size( 0 ), _ownsBuffer( true ), _allocator( allocator ){}

   /**
    @brief copy constructor.
    */
   Buffer1D( const Buffer1D& cpy ) : _cpt( 0 ), _buffer( 0 ), _size( 0 ) { copy( cpy ); }

   /**
    @brief construcs a buffer from a pointer.
    @param buf the source buffer
    @param size the size of the buffer
    @param ownsBuffer if true, the pointer will be used and deleted at the end of life of the object, else it is used
                      but not deleted. Ensure the parameter is valid until Buffer1D is used.
    */
   Buffer1D( T* buf, ui32 size, bool ownsBuffer, Allocator allocator = Allocator() ) : _buffer( buf ), _size( size ), _ownsBuffer( ownsBuffer ), _allocator( allocator )
   {
      _cpt = typename Allocator::template rebind<i32>::other( _allocator ).allocate( 1 );
      *_cpt = 1; //initialRefCount;
   }

   /**
    @brief decrease the reference count. If zero, the internal buffer is destroyed.
    */
   virtual ~Buffer1D(){ unref(); }

   /**
    @return the index in the buffer of its i th element.
    */
   inline ui32 index( const ui32 i ){ return IndexMapper1D::index( i ); }

   /**
    @return the number of instances sharing this buffer.
    */
   inline ui32 getRefCount() const {if (!_cpt) return 0; return *_cpt; }

   /**
    @return the internal buffer.
    */
   inline const T* getBuf() const { return _buffer; }

   /**
    @return the internal buffer. The memory management is now deferred to the user.
            It is the responsability of the user to destroy the object.
    @note CAUTION as if several buffers pointing to the same object, only one object will be affected!
    */
   inline T* stealBuf()
   {
      ref(); // make sure it will not be destroyed by any containers...
      _ownsBuffer = false;
      return _buffer;
   }

   /**
    @return the internal buffer.
    */
   inline T* getBuf() { return _buffer; }

   /**
    @return the allocator used by this object
    */
   Allocator getAllocator() const
   {
      return _allocator;
   }

   /**
    @return the idx th value of the buffer.
    */
   inline typename BestConstReturnType<T>::type at( const ui32 idx ) const
   {
      assert( IndexMapper::index( idx ) < _size );
      return _buffer[ IndexMapper::index( idx ) ];
   }

   /**
    @return the idx th value of the buffer.
    */
   inline T& at( const ui32 idx )
   {
      assert( IndexMapper::index( idx ) < _size );
      return _buffer[ IndexMapper::index( idx ) ];
   }

   /**
    @return the idx th value of the buffer.
    */
   inline typename BestConstReturnType<T>::type operator()( const ui32 idx ) const
   {
      return at( idx );
   }

   /**
    @return the idx th value of the buffer.
    */
   inline T& operator()( const ui32 idx )
   {
      return at( idx );
   }

   /**
    @return the idx th value of the buffer.
    */
   inline typename BestConstReturnType<T>::type operator[]( const ui32 idx ) const
   {
      return at( idx );
   }

   /**
    @return the idx th value of the buffer.
    */
   inline T& operator[]( const ui32 idx )
   {
      return at( idx );
   }

   /**
    @brief print the vector on a stream.
    */
   inline void print( std::ostream& o ) const
   {
      if ( _buffer )
      {
         o << "Buffer1D(" << *_cpt << ") size=" << _size << " values=";
         for (ui32 n = 0; n < _size; ++n)
            o << at( n ) << " ";
         o << std::endl;
      } else {
         o << "Buffer1D(NULL)" << std::endl;
      }
   }

   /**
    @return the size of the vector.
    */
   inline ui32 size() const {return _size;}

   /**
    @brief import from a raw buffer (clone it: memory is copied).
    */
   void import( const T* buf, ui32 size )
   {
      unref();

      _allocate( size, false );
      for (ui32 n = 0; n < size; ++n)
         at( n ) = buf[ n ];
   }

   /**
    @brief clone a buffer. The memory is copied.
    */
   template <class Vector>
   void clone(const Vector& cpy)
   {
# ifdef DEBUG_BUFFER1D
      std::cout << "clone buffer1D" << std::endl;
# endif
      _allocate( static_cast<ui32>( cpy.size() ), false );
      for (ui32 n = 0; n < cpy.size(); ++n)
         at( n ) = cpy[ n ];
   }

   /**
    @brief share a buffer. The buffer is shared with this instance.
    */
   void copy(const Buffer1D& cpy)
   {
# ifdef DEBUG_BUFFER1D
      std::cout << "copy buffer1D" << std::endl;
# endif

      if ( cpy._buffer != _buffer )
      {
         // unref with the current allocator
         unref();

         _allocator = cpy._allocator;
         _cpt = cpy._cpt;
         _buffer = cpy._buffer;
         _size = cpy.size();
         _ownsBuffer = cpy._ownsBuffer;

         ref();
      }
   }

   /**
    @brief increment the reference count.
    */
   inline void ref()
   {
# ifdef DEBUG_BUFFER1D
      std::cout << "ref buffer1D" << std::endl;
# endif
      if ( _buffer )
      {
#ifndef NLL_NOT_MULTITHREADED
         #pragma omp atomic
#endif
         ++*_cpt;
      }
   }

   /**
    @brief decrement the reference count.
    */
   void unref()
   {
      if ( _buffer )
      {
# ifdef DEBUG_BUFFER1D
         std::cout << "unref buffer1D" << std::endl;
# endif
#ifndef NLL_NOT_MULTITHREADED
         #pragma omp atomic
#endif
         --*_cpt;
         if ( !*_cpt )
         {
# ifdef DEBUG_BUFFER1D
            std::cout << "destroy buffer1D=" << _size << std::endl;
# endif
            typename Allocator::template rebind<i32>::other( _allocator ).deallocate( _cpt, 1 );
            _cpt = 0;
            if ( _ownsBuffer )
            {
               if ( !IsPOD<T>::value )
               {
                  for ( ui32 n = 0; n < _size; ++n )
                  {
                     (_buffer + n )->~T();
                  }
               }
               _allocator.deallocate( _buffer, _size );
            }
            _buffer = 0;
         }
      }
   }

   /**
    @brief write the vector to a stream.
    */
   void write( std::ostream& o ) const
   {
      nll::core::write<ui32>( _size, o );
      for ( ui32 n = 0; n < _size; ++n )
         nll::core::write<T>( _buffer[ n ], o );
   }

   /**
    @brief read the vector from a stream.
    */
   void read( std::istream& i )
   {
      nll::core::read<ui32>( _size, i );
      if ( _size )
      {
         _allocate( _size, true );  // true! in case of recursive Buffer1D<Buffer1D<>> we need to set to the zero the pointers!
         for ( ui32 n = 0; n < _size; ++n )
            nll::core::read<T>( _buffer[ n ], i );
      }
   }

   /**
    @brief copy the buffer.
    */
   Buffer1D& operator=( const Buffer1D& cpy )
   {
      copy( cpy );
      return *this;
   }

   /**
    @brief Decrease the ref count if > 0, and unshare this object.
    */
   void clear()
   {
      if ( _cpt && *_cpt )
      {
         unref();
      }
      _buffer = 0;
      _cpt = 0;
      _size = 0;
   }

   /**
    @brief check if the buffers are semantically equal.
    */
   bool operator==( const Buffer1D& b ) const
   {
      if ( _size != b._size )
         return false;
      if ( _buffer == b._buffer )
         return true;
      for ( ui32 n = 0; n < _size; ++n )
         if ( !core::equal( at( n ), b.at( n ) ) )
            return false;
      return true;
   }

   /**
    @brief check if the buffers are semantically equal with a fixed tolerance.
    */
   template <class T2, class IMapper2, class Alloc>
   inline bool equal( const Buffer1D<T2, IMapper2, Alloc>& op, T tolerance = std::numeric_limits<T>::epsilon() ) const
   {
      if ( _size != op.size() )
         return false;
      if ( ! op.getBuf() || !_buffer )
         return false;
      if ( (void*)op.getBuf() == (void*)_buffer )
         return true;
      for ( ui32 n = 0; n < _size; ++n )
         if ( absolute( op( n ) - at( n ) ) > tolerance )
            return false; 
      return true;
   }

   public:
      /// iterator
      typedef T*        iterator;

      /// const iterator
      typedef const T*  const_iterator;

      /**
       @brief return an iterator on the begining of the vector
       */
      iterator begin() { return _buffer;}

      /**
       @brief return an iterator on the begining of the vector
       */
      const_iterator begin() const { return _buffer;}

      /**
       @brief return an iterator on the end of the vector
       */
      iterator end() { return _buffer + _size;}

      /**
       @brief return an iterator on the end of the vector
       */
      const_iterator end() const { return _buffer + _size;}

protected:
   void _allocate( ui32 size, bool zero )
   {
# ifdef DEBUG_BUFFER1D
      std::cout << "allocate buffer1D=" << size << std::endl;
# endif
      unref();
      if ( !size )
         return;

      _ownsBuffer = true;
      if (!_cpt)
      {
         _cpt = typename Allocator::template rebind<i32>::other( _allocator ).allocate( 1 );
         *_cpt = 0;
      }
      _buffer = _allocator.allocate( size );
      _size = size;
      if ( !IsPOD<T>::value )
      {
         for ( ui32 n = 0; n < size; ++n )
         {
            char* addr = (char*)(_buffer + n);
            PLACEMENT_NEW (addr)T();
         }
      } else {
         if ( zero )
            memset( _buffer, 0, sizeof ( T ) * size );
      }

      ref();
   }

protected:
   mutable i32*    _cpt;
   T*             _buffer;
   ui32           _size;
   bool           _ownsBuffer;
   Allocator      _allocator;
};

}
}

#ifdef NLL_FIND_MEMORY_LEAK
# define new DEBUG_NEW
#endif

# pragma warning( pop )

#endif
