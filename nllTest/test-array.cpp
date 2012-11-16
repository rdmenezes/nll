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
         buf[ n ] += accum;
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
         buf[ n ] *= accum;
      }
      return accum;
   }

   template <int size>
   class ArrayMapperContiguousRowMajor
   {
   public:
      typedef StaticVector<int, size>  Shape;

      ArrayMapperRowMajor( const Shape& dimensions ) : _shape( shape )
      {}

   private:
      Shape                                     _shape;
      std::vector<typename Shape::value_type>   _stides;
   };


   /**
    @brief This class provides manipulation of a multidimentional through iterators only

    Two types of iterators are defined:
    - iterator/const_iterator: provide the fastest access as possible for iterating on all the data,
      iteration order is unspecified
    - directional iterators: probably not as fast, but gives control over the direction to iterate on
    */
   template <class T, int size, class Mapper = ArrayMapperContiguousRowMajor<size>, class AllocatorT = std::allocator<T> >
   class MemoryStoreContiguous
   {
   public:
      typedef StaticVector<int, size>  Shape;
      typedef T                        value_type;
      typedef MapperT                  Mapper;
      typedef AllocatorT               Allocator;
      typedef Buffer1D<T>              Storage;

      MemoryStoreContiguous( const Allocator& allocator = Allocator() ) : _storage( allocator )
      {}

      MemoryStoreContiguous( const Shape& shape, const T& init, const Allocator& allocator = Allocator() ) : _storage( mulList( shape.getBuf(), shape.size() ), false )
      {
         for ( Storage::iterator it = _storage.begin(); it != _storage.end(); ++it )
         {
            *it = init;
         }
      }

      MemoryStoreContiguous( const Shape& shape, const Allocator& allocator = Allocator() ) : _storage( mulList( shape.getBuf(), shape.size() ), false )
      {}

      void clear()
      {
         _storage.clear();
      }

   private:
      Storage  _storage;
   };
}
}

class TestArray
{
public:
   void testInitializerList()
   {
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestArray);
TESTER_TEST(testInitializerList);
TESTER_TEST_SUITE_END();
#endif