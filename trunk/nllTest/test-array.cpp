#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace core
{
   template <class T>
   T sumList( const T*  buf, int size )
   {
      assert( size );
      STATIC_ASSERT( IsNativeType<T>::value );
      T accum = buf[ 1 ];
      for ( int n = 1; n < size; ++n )
      {
         accum += buf[ n ];
      }
      return accum;
   }

   template <class T>
   T mulList( const T*  buf, int size )
   {
      assert( size );
      STATIC_ASSERT( IsNativeType<T>::value );
      T accum = buf[ 0 ];
      for ( int n = 1; n < size; ++n )
      {
         accum *= buf[ n ];
      }
      return accum;
   }

   template <class T>
   T dotList( const T* v1, const T* v2, int size )
   {
      T accum = 0;
      const T* v1End = v1 + size;

      for ( ; v1 != v1End; ++v1, ++v2 )
      {
         accum += *v1 * *v2;
      }
      return accum;
   }

   template <int size>
   class ArrayMapperContiguousRowMajor
   {
   public:
      typedef StaticVector<int, size>  Shape;
      typedef StaticVector<int, size>  Index;

      ArrayMapperContiguousRowMajor( const Shape& dimensions )
      {
         #ifdef NLL_SECURE
         ensure( dimensions.size() == 0 || dimensions.size() == size, "size mismatch" );
         #endif

         int accum = 1;
         for ( size_t n = 0; n < size; ++n )
         {
            _strides[ n ] = accum;
            accum *= dimensions[ n ];
         }
      } 

      inline int index( const Index& index ) const
      {
         return dotList( index.getBuf(), _strides.getBuf(), index.size() );
      }

      template <int dimensionIncrement>
      inline int add( int index, int size ) const
      {
         return index + size * _strides[ dimensionIncrement ];
      }

      template <int dimensionIncrement>
      inline int add( int index ) const
      {
         return index + _strides[ dimensionIncrement ];
      }

   private:
      Shape _strides;
   };


   /**
    @brief This class provides manipulation of a multidimentional through iterators only

    Asumptions: memory is stored as a contiguous array (i.e., this is the most common memory layout)
    If other configurations are needed, other memory store may be implemented

    Two types of iterators are defined:
    - iterator/const_iterator: provide the fastest access as possible for iterating on all the data,
      iteration order is unspecified
    - directional iterators: probably not as fast, but gives control over the direction to iterate on
    */
   template <class T, int size, class MapperT = ArrayMapperContiguousRowMajor<size>, class AllocatorT = std::allocator<T> >
   class MemoryStoreContiguous
   {
   public:
      typedef StaticVector<int, size>              Shape;
      typedef T                                    value_type;
      typedef MapperT                              Mapper;
      typedef AllocatorT                           Allocator;
      typedef Buffer1D<T>                          Storage;

      typedef typename Storage::iterator           iterator;
      typedef typename Storage::const_iterator     const_iterator;

      /**
       @brief A directional iterator. It allows to iterate over all dimensions. It is also able to pick without moving
              in one of the 3 possible directions.
       @note addx, addy, addz, pickx, picky, pickz beware of the bounds as they are not checked!
       */
      class ConstDirectionalIterator
      {
      public:
         typedef T                                       value_type;
         typedef typename BestConstReturnType<T>::type   const_return_type;
         typedef T&                                      ref_return_type;
         typedef const T&                                const_ref_return_type;

         ConstDirectionalIterator( int index, T* buf, const Mapper& mapper ) : _index( index ), _buf( buf ), _mapper( mapper )
         {}

         /**
          @brief get the value pointed by the iterator. It is only valid if the iterator is pointing on a voxel!
          */
         const_ref_return_type operator*() const
         {
            return _buf[ _index ];
         }

         /**
          @brief move to the next element
          */
         ConstDirectionalIterator& operator++()
         {
            ++_index;
            return *this;
         }

         ConstDirectionalIterator operator++( int )
         {
            ConstDirectionalIterator ans = *this;
            ++_index;
            return ans;
         }

         /**
          @brief move the iterator on a new x
          */
         template <int dimensionIncrement>
         ConstDirectionalIterator& add( int n )
         {
            _index = _mapper.add<dimensionIncrement>( _index, n );
            return *this;
         }

         template <int dimensionIncrement>
         ConstDirectionalIterator& add()
         {
            _index = _mapper.add<dimensionIncrement>( _index );
            return *this;
         }

         template <int dimensionIncrement>
         const_return_type pick( int n )
         {
            const int index = _mapper.add<dimensionIncrement>( _index, n );
            return _buf[ index ];
         }

         template <int dimensionIncrement>
         const_return_type pick()
         {
            const int index = _mapper.add<dimensionIncrement>( _index );
            return _buf[ index ];
         }

         /**
          @brief test if the iterators are pointing at the same position.
          */
         bool operator==( const ConstDirectionalIterator& i ) const
         {
            #ifdef NLL_SECURE
            ensure( _buf == i._buf, "Wrong buffer!" );
            #endif
            return _index == i._index;
         }

         /**
          @brief test if the iterators are pointing at the same position.
          */
         bool operator!=( const ConstDirectionalIterator& i )  const
         {
            #ifdef NLL_SECURE
            ensure( _buf == i._buf, "Wrong buffer!" );
            #endif
            return _index != i._index;
         }

      protected:
         int                     _index;
         value_type*             _buf;
         Mapper                  _mapper;
      };

      class DirectionalIterator : public ConstDirectionalIterator
      {
      public:
         DirectionalIterator( int index, const T* buf, const Mapper& mapper ) : ConstDirectionalIterator( index, const_cast<T*>( buf ), mapper )
         {}

         DirectionalIterator( const DirectionalIterator& cpy ) : ConstDirectionalIterator( cpy )
         {}

         /**
          @brief get the value pointed by the iterator. It is only valid if the iterator is pointing on a voxel!
          */
         ref_return_type operator*()
         {
            return _buf[ _index ];
         }
      };

      MemoryStoreContiguous( const Allocator& allocator = Allocator() ) : _storage( allocator ), _mapper( Mapper( Shape() ) ),
         // dummy begin/end
         _begin( DirectionalIterator( 0, _storage.getBuf(), _mapper ) ),
         _end( DirectionalIterator( 0, _storage.getBuf(), _mapper ) )
      {}

      MemoryStoreContiguous( const Shape& shape, const T& init, const Allocator& allocator = Allocator() ) : _storage( mulList( shape.getBuf(), shape.size() ), false ), _mapper( shape ), _size( shape ),
         _begin( DirectionalIterator( 0, _storage.getBuf(), _mapper ) ),
         _end( DirectionalIterator( mulList( shape.getBuf(), shape.size() ), _storage.getBuf(), _mapper ) )
      {
         for ( Storage::iterator it = _storage.begin(); it != _storage.end(); ++it )
         {
            *it = init;
         }
      }

      MemoryStoreContiguous( const Shape& shape, const Allocator& allocator = Allocator() ) : _storage( mulList( shape.getBuf(), shape.size() ), false ), _mapper( shape ), _size( shape ),
         _begin( DirectionalIterator( 0, _storage.getBuf(), _mapper ) ),
         _end( DirectionalIterator( mulList( shape.getBuf(), shape.size() ), _storage.getBuf(), _mapper ) )
      {}

      // clear the reference count. It frees memory if it is the last reference (and so iterators become invalid)
      void clear()
      {
         _storage.clear();
      }

      iterator begin()
      {
         return _storage.begin();
      }

      iterator end()
      {
         return _storage.end();
      }

      DirectionalIterator getIterator( const Shape& index )
      {
         const int linearIndex = _mapper.index( index );
         return DirectionalIterator( linearIndex, _storage.getBuf(), _mapper );
      }

      ConstDirectionalIterator getIterator( const Shape& index ) const
      {
         const int linearIndex = _mapper.index( index );
         return ConstDirectionalIterator( linearIndex, _storage.getBuf(), _mapper );
      }

      DirectionalIterator beginDirectional()
      {
         return _begin;
      }

      ConstDirectionalIterator beginDirectional() const
      {
         return ConstDirectionalIterator( _begin );
      }

      DirectionalIterator endDirectional()
      {
         return _end;
      }

      ConstDirectionalIterator endDirectional() const
      {
         return ConstDirectionalIterator( _end );
      }

      const Shape& getSize() const
      {
         return _size;
      }

   private:
      Storage              _storage;
      Mapper               _mapper;
      Shape                _size;
      DirectionalIterator  _begin;
      DirectionalIterator  _end;
   };
}
}

class TestArray
{
public:
   void testMemoryStoreContiguous()
   {
      typedef core::MemoryStoreContiguous<char, 3> Array;
      Array array( core::vector3i( 2, 3, 4 ) );

      // test simple iterators: assign, and ensure it is done correctly
      char c = 0;
      for ( Array::iterator it = array.begin(); it != array.end(); ++it )
      {
         *it = c++;
      }

      c = 0;
      for ( Array::const_iterator it = array.begin(); !(it == array.end()); ++it )
      {
         TESTER_ASSERT( *it == c++ );
      }

      // check directional iterators on known array
      //
      // 4   5 | 10 11 | 16 17 | 22 23
      // 2   3 | 8   9 | 14 15 | 20 21
      // 0   1 | 6   7 | 12 13 | 18 19
      //
      {
         Array::DirectionalIterator it = array.beginDirectional();
         TESTER_ASSERT( *it == 0 );
         TESTER_ASSERT( it.pick<0>() == 1 );
         TESTER_ASSERT( it.pick<0>( 1 ) == 1 );

         TESTER_ASSERT( it.pick<1>() == 2 );
         TESTER_ASSERT( it.pick<1>( 2 ) == 4 );

         TESTER_ASSERT( it.pick<2>() == 6 );
         TESTER_ASSERT( it.pick<2>( 2 ) == 12 );

         it = array.getIterator( core::vector3i( 0, 2, 1 ) );
         TESTER_ASSERT( *it == 10 );

         it = array.getIterator( core::vector3i( 1, 2, 3 ) );
         TESTER_ASSERT( *it == 23 );

         it = array.getIterator( core::vector3i( 1, 2, 1 ) );
         TESTER_ASSERT( *it == 11 );

         // now test pick with a base != 0
         TESTER_ASSERT( it.pick<0>() == 12 );
         TESTER_ASSERT( it.pick<0>( 2 ) == 13 );

         TESTER_ASSERT( it.pick<1>() == 13 );
         TESTER_ASSERT( it.pick<1>( -1 ) == 9 );

         TESTER_ASSERT( it.pick<2>() == 17 );
         TESTER_ASSERT( it.pick<2>( -1 ) == 5 );

         int size = array.end() - array.begin();
         TESTER_ASSERT( size == array.getSize()[ 0 ] * array.getSize()[ 1 ] * array.getSize()[ 2 ] );
      }


      {
         Array::ConstDirectionalIterator it = array.beginDirectional();
         TESTER_ASSERT( *it == 0 );
         TESTER_ASSERT( it.pick<0>() == 1 );
         TESTER_ASSERT( it.pick<0>( 1 ) == 1 );

         TESTER_ASSERT( it.pick<1>() == 2 );
         TESTER_ASSERT( it.pick<1>( 2 ) == 4 );

         TESTER_ASSERT( it.pick<2>() == 6 );
         TESTER_ASSERT( it.pick<2>( 2 ) == 12 );

         it = array.getIterator( core::vector3i( 0, 2, 1 ) );
         TESTER_ASSERT( *it == 10 );

         it = array.getIterator( core::vector3i( 1, 2, 3 ) );
         TESTER_ASSERT( *it == 23 );

         it = array.getIterator( core::vector3i( 1, 2, 1 ) );
         TESTER_ASSERT( *it == 11 );

         // now test pick with a base != 0
         TESTER_ASSERT( it.pick<0>() == 12 );
         TESTER_ASSERT( it.pick<0>( 2 ) == 13 );

         TESTER_ASSERT( it.pick<1>() == 13 );
         TESTER_ASSERT( it.pick<1>( -1 ) == 9 );

         TESTER_ASSERT( it.pick<2>() == 17 );
         TESTER_ASSERT( it.pick<2>( -1 ) == 5 );

         int size = array.end() - array.begin();
         TESTER_ASSERT( size == array.getSize()[ 0 ] * array.getSize()[ 1 ] * array.getSize()[ 2 ] );
      }

      Array::ConstDirectionalIterator itc = array.beginDirectional();
      Array::DirectionalIterator it = array.beginDirectional();
      TESTER_ASSERT( itc == it );

      it = array.getIterator( core::vector3i( 1, 2, 1 ) );
      *it = 42;
      TESTER_ASSERT( *it == 42 );
   }

   void testMemoryStoreContiguous_speed_1d()
   {
      const int BufferSize = 100000;
      const int NbIteration = 500;

      typedef core::MemoryStoreContiguous<int, 1> Array;
      core::StaticVector<int, 1> size;
      size[ 0 ] = BufferSize;
      Array array( size );
      array.begin();

      int accumRef = 0;
      for ( Array::iterator it = array.begin(); it != array.end(); ++it )
      {
         const int val = rand() % 256;
         *it = val;
         accumRef += val;
      }

      // test basic const iterators
      core::Timer timerConst;
      for ( int iter = 0; iter < NbIteration; ++iter )
      {
         int accum = 0;
         for ( Array::ConstDirectionalIterator it = array.beginDirectional(); it != array.endDirectional(); ++it )
         {
            accum += *it;
         }
         TESTER_ASSERT( accumRef == accum );
      }

      const double timeConst = timerConst.getCurrentTime();

      // test basic const iterators
      core::Timer timerNonConst;
      for ( int iter = 0; iter < NbIteration; ++iter )
      {
         int accum = 0;
         for ( Array::DirectionalIterator it = array.beginDirectional(); it != array.endDirectional(); ++it )
         {
            accum += *it;
         }
         TESTER_ASSERT( accumRef == accum );
      }

      const double timeNonConst = timerNonConst.getCurrentTime();

      // test raw pointers
      core::Timer timerRaw;
      for ( int iter = 0; iter < NbIteration; ++iter )
      {
         int accum = 0;
         const int* start = array.begin();
         const int* end = array.end();
         for ( ; start != end; ++start )
         {
            accum += *start;
         }
         TESTER_ASSERT( accumRef == accum );
      }

      const double timeRaw = timerRaw.getCurrentTime();

      std::cout << "const iterator time=" << timeConst << std::endl;
      std::cout << "non iterator time=" << timeNonConst << std::endl;
      std::cout << "Raw pointer time=" << timeRaw << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestArray);
TESTER_TEST(testMemoryStoreContiguous);
TESTER_TEST(testMemoryStoreContiguous_speed_1d);
TESTER_TEST_SUITE_END();
#endif