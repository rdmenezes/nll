#ifndef NLL_STATIC_VECTOR_H_
# define NLL_STATIC_VECTOR_H_

# include <assert.h>
# include <iostream>
# include "generic-operations.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief implement static vectors. Memory is allocated on the heap. Memory is not shared accross instances.
   */
   template <class T, int SIZE>
   class StaticVector
   {
   public:
      typedef T      value_type;
      enum{size = SIZE};

   public:
      /**
       @brief Fake constructor tp encure compatibility with the dynamic one
       */
      StaticVector( const ui32 size )
      {
         assert( size == SIZE );
      }
      /**
       @brief copy constructor
       */
      StaticVector( const StaticVector& cpy )
      {
         memcpy( _buffer, cpy._buffer, sizeof ( T ) * SIZE );
      }

      /**
       @brief instanciate a vector
       */
      StaticVector()
      {
         memset( _buffer, 0, sizeof ( T ) * SIZE );
      }

      /**
       @brief instanciate a static vector from a pointer. Memory is copied.
       */
      template <class T2>
      StaticVector( const T2* buf )
      {
         for ( int n = 0; n < SIZE; ++n )
            _buffer[ n ] = static_cast<T>( buf[ n ] );
      }

      /**
       @brief return the internal buffer of the vector
       */
      inline T* getBuf(){ return _buffer; }

      /**
       @brief return the internal buffer of the vector
       */
      inline const T* getBuf() const { return _buffer; }

      /**
       @brief return the value at the specified index
       */
      inline T at( const ui32 index ) const
      {
         assert( index < SIZE );
         return _buffer[ index ];
      }

      /**
       @brief return the value at the specified index
       */
      inline T& at( const ui32 index )
      {
         assert( index < SIZE );
         return _buffer[ index ];
      }

      /**
       @brief return the value at the specified index
       */
      inline T operator[]( const ui32 index ) const
      {
         return at( index );
      }

      /**
       @brief return the value at the specified index
       */
      inline T& operator[]( const ui32 index )
      {
         return at( index );
      }

      /**
       @brief return the value at the specified index
       */
      inline T operator()( const ui32 index ) const
      {
         return at( index );
      }

      /**
       @brief return the value at the specified index
       */
      inline T& operator()( const ui32 index )
      {
         return at( index );
      }

      /**
       @brief define operation on the static vector
       */
      inline void add( const StaticVector& op )
      {
         generic_add<T*, const T*, SIZE>(_buffer, op._buffer);
      }

      /**
       @brief define operation on the static vector
       */
      inline void sub( const StaticVector& op )
      {
         generic_sub<T*, const T*, SIZE>(_buffer, op._buffer);
      }

      /**
       @brief define operation on the static vector
       */
      inline void mul( const T val )
      {
         generic_mul_cte<T*, SIZE>(_buffer, val);
      }

      /**
       @brief define operation on the static vector
       */
      inline void div( const T val )
      {
         generic_div_cte<T*, SIZE>(_buffer, val);
      }

      /**
       @brief define operation on the static vector
       */
      StaticVector operator+( const StaticVector& op2 ) const
      {
         StaticVector res( *this );
         res.add( op2 );
         return res;
      }

      /**
       @brief define operation on the static vector
       */
      StaticVector operator-( const StaticVector& op2 ) const
      {
         StaticVector res( *this );
         res.sub( op2 );
         return res;
      }

      /**
       @brief define operation on the static vector
       */
      StaticVector operator*( const T val ) const
      {
         StaticVector res( *this );
         res.mul( val );
         return res;
      }

      /**
       @brief define operation on the static vector
       */
      StaticVector operator/( const T val ) const
      {
         assert( val != 0 );
         StaticVector res( *this );
         res.div( val );
         return res;
      }

      /**
       @brief define operation on the static vector
       */
      T dot( const StaticVector& op )
      {
         T accum = 0;
         for ( int n = 0; n < SIZE; ++n )
            accum += op[ n ] * at( n );
         return accum;
      }

      /**
       @brief copy the vector
       */
      StaticVector& operator=( const StaticVector& cpy )
      {
         memcpy( _buffer, cpy._buffer, sizeof ( T ) * SIZE );
         return *this;
      }

      /**
       @brief return the euclidian norm of the vector
       */
      double norm2() const
      {
         return generic_norm2<const T*, double, SIZE> ( _buffer );
      }

      /**
       @brief print the vector to a stream
       */
      void print( std::ostream& o ) const
      {
         o << "static vector, size = " << SIZE << std::endl;
         for ( int n = 0; n < SIZE; ++n )
            std::cout << _buffer[ n ] << " ";
         std::cout << std::endl;
      }

   private:
      T     _buffer[ SIZE ];
   };

   /**
    @ingroup core
    @brief specific implementation with custom constructor
    */
   class vector2i : public StaticVector<i32, 2>
   {
      typedef StaticVector<i32, 2> BaseClass;
   public:
      vector2i( )
      {
         at(0) = 0;
         at(1) = 0;
      }
      vector2i( BaseClass::value_type x, BaseClass::value_type y )
      {
         at(0) = x;
         at(1) = y;
      }
   };

   /**
    @ingroup core
    @brief specific implementation with custom constructor
    */
   class vector2f : public StaticVector<f32, 2>
   {
      typedef StaticVector<f32, 2> BaseClass;
   public:
      vector2f( BaseClass::value_type x, BaseClass::value_type y )
      {
         at(0) = x;
         at(1) = y;
      }
   };

   typedef StaticVector<int,     3> vector3i;
   typedef StaticVector<float,   3> vector3f;
   typedef StaticVector<double,  2> vector2d;
   typedef StaticVector<double,  3> vector3d;

}
}

#endif
