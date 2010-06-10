#ifndef NLL_BUFFER1D_H_
# define NLL_BUFFER1D_H_

# include <limits>
# include <limits.h>
# include <assert.h>
# include "types.h"
# include "type-traits.h"
# include "index-mapper.h"
# include "io.h"

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
 @todo ONLY WORKING FOR POD DATATYPES (CHECK COPY CONSTRUCTOR IS CORRECTLY CALLED) -> else memset(0) is used to init data
       which can cause problems for data structures of the STL for example. Quickfix: set zero = false in constructor
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
    @param zero if set to true, the buffer is cleared by zero, else undefined value
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
      _cpt = Allocator::template rebind<i32>::other( _allocator ).allocate( 1 );
      *_cpt = 1; //initialRefCount;
   }

   /**
    @brief decrease the reference count. If zero, the internal buffer is destroyed.
    */
   ~Buffer1D(){ unref(); } // TODO: virtual

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
    */
   inline T* stealBuf()
   {
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
         o << "Buffer1D(" << *_cpt << ") size=" << _size << " ";
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
      // unref with the current allocator
      unref();

      _allocator = cpy._allocator;
      _cpt = cpy._cpt;
      _buffer = cpy._buffer;
      _size = cpy.size();
      _ownsBuffer = cpy._ownsBuffer;

      ref();
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
         #pragma omp atomic
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
         #pragma omp atomic
         --*_cpt;
         if ( !*_cpt )
         {
# ifdef DEBUG_BUFFER1D
            std::cout << "destroy buffer1D=" << _size << std::endl;
# endif
            Allocator::template rebind<i32>::other( _allocator ).deallocate( _cpt, 1 );
            _cpt = 0;
            if ( _ownsBuffer )
               _allocator.deallocate( _buffer, _size );
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
         _allocate( _size, false );
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
         _cpt = Allocator::template rebind<i32>::other( _allocator ).allocate( 1 );
         *_cpt = 0;
      }
      _buffer = _allocator.allocate( size );
      _size = size;

      // TODO : if NON-POD data?
      if ( zero )
         memset( _buffer, 0, sizeof ( T ) * size );

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
#endif
