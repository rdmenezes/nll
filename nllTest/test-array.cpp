#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace core
{
   template <class T>
   T sumList( const T*  buf, iint size )
   {
      assert( size );
      STATIC_ASSERT( IsNativeType<T>::value );
      T accum = buf[ 1 ];
      for ( iint n = 1; n < size; ++n )
      {
         accum += buf[ n ];
      }
      return accum;
   }

   template <class T>
   T mulList( const T*  buf, iint size )
   {
      assert( size );
      STATIC_ASSERT( IsNativeType<T>::value );
      T accum = buf[ 0 ];
      for ( iint n = 1; n < size; ++n )
      {
         accum *= buf[ n ];
      }
      return accum;
   }

   template <class T>
   T dotList( const T* v1, const T* v2, iint size )
   {
      T accum = 0;
      const T* v1End = v1 + size;

      for ( ; v1 != v1End; ++v1, ++v2 )
      {
         accum += *v1 * *v2;
      }
      return accum;
   }



   /**
    @brief Mapper based on strides: (a, b, c) -> a * stride[0] + b * stride[1] + c * stride[2]
    */
   template <iint size>
   class ArrayMapperStrideBased
   {
   public:
      typedef iint                                 internal_type;
      typedef StaticVector<internal_type, size>    Index;

      ArrayMapperStrideBased( const Index& strides ) : _strides( strides )
      {
         #ifdef NLL_SECURE
         ensure( strides.size() == 0 || strides.size() == size, "size mismatch" );
         #endif

         typedef std::map<internal_type, internal_type> MapElement;

         // sort the elements from smallest->highest stride
         MapElement elements;
         for ( Index::const_iterator it = strides.begin(); it != strides.end(); ++it )
         {
            elements[ *it ] = static_cast<internal_type>( it - strides.begin() );
         }

         // record the order of lowest index variations: this is how the memory must be accessed
         internal_type count = 0;
         for ( MapElement::const_iterator it = elements.begin(); it != elements.end(); ++it, ++count )
         {
            _memoryAccessOrder[ it->second ] = count;
         }
      } 

      /**
       @brief Computes the linear index corresponding to this multidimentional index
       */
      inline internal_type index( const Index& index ) const
      {
         return dotList( index.getBuf(), _strides.getBuf(), index.size() );
      }

      /**
       @brief Assuming we want to fill a region of the array, this index gives us the order into which this array
              is accessed efficiently in memory
       */
      const Index& getMemoryAccessOrder() const
      {
         return _memoryAccessOrder;
      }

   protected:
      Index _strides;               // stide definition
      Index _memoryAccessOrder;     // get the optimal memory access order (i.e., with minimal jumps in index space)
   };
   
   /**
    @brief Simple wrapper for row major memory layout
    */
   template <iint size>
   class ArrayMapperContiguousRowMajor : public ArrayMapperStrideBased<size>
   {
   public:
      typedef StaticVector<internal_type, size>       Index;
      typedef ArrayMapperStrideBased<size>            Base;

      ArrayMapperContiguousRowMajor( const Index& dimensions ) : Base( getStride( dimensions ) )
      {
         #ifdef NLL_SECURE
         ensure( dimensions.size() == 0 || dimensions.size() == size, "size mismatch" );
         #endif
      }

      template <internal_type dimensionIncrement>
      inline internal_type add( internal_type index, internal_type size ) const
      {
         return index + size * _strides[ dimensionIncrement ];
      }

      template <>
      inline internal_type add<0>( internal_type index, internal_type size ) const
      {
         return index + size;
      }

      template <internal_type dimensionIncrement>
      inline internal_type add( internal_type index ) const
      {
         return index + _strides[ dimensionIncrement ];
      }

      template <>
      inline internal_type add<0>( internal_type index ) const
      {
         return ++index;
      }

   private:
      static Index getStride( const Index& dimensions )
      {
         Index strides( dimensions.size() );

         internal_type accum = 1;
         for ( internal_type n = 0; n < size; ++n )
         {
            strides[ n ] = accum;
            accum *= dimensions[ n ];
         }

         return strides;
      }
   };

   template <iint number>
   struct MinusOne
   {
      enum{Value = number - 1};
   };

   /**
    @brief Simple wrapper for row major memory layout
    */
   template <iint size>
   class ArrayMapperContiguousColumnMajor : public ArrayMapperStrideBased<size>
   {
   public:
      typedef StaticVector<internal_type, size>       Index;
      typedef ArrayMapperStrideBased<size>            Base;

      ArrayMapperContiguousColumnMajor( const Index& dimensions ) : Base( getStride( dimensions ) )
      {
         #ifdef NLL_SECURE
         ensure( dimensions.size() == 0 || dimensions.size() == size, "size mismatch" );
         #endif
      }

      template <internal_type dimensionIncrement>
      inline internal_type add( internal_type index, internal_type size ) const
      {
         return index + size * _strides[ dimensionIncrement ];
      }
      
      template <>
      inline internal_type add<MinusOne<size>::Value>( internal_type index, internal_type size ) const
      {
         return index + size;
      }

      template <internal_type dimensionIncrement>
      inline internal_type add( internal_type index ) const
      {
         return index + _strides[ dimensionIncrement ];
      }
     
      template <>
      inline internal_type add<MinusOne<size>::Value>( internal_type index ) const
      {
         return ++index;
      }

   private:
      static Index getStride( const Index& dimensions )
      {
         Index strides( dimensions.size() );

         internal_type accum = 1;
         for ( internal_type n = size - 1; n >= 0; --n )
         {
            strides[ n ] = accum;
            accum *= dimensions[ n ];
         }

         return strides;
      }
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
   template <class T, iint size, class MapperT = ArrayMapperContiguousRowMajor<size>, class AllocatorT = std::allocator<T> >
   class MemoryStoreContiguous
   {
   public:
      typedef StaticVector<iint, size>             Index;
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

         ConstDirectionalIterator( iint index, T* buf, const Mapper& mapper ) : _index( index ), _buf( buf ), _mapper( mapper )
         {}

         /**
          @brief get the value pointed by the iterator. It is only valid if the iterator is pointing on a voxel!
          */
         const_ref_return_type operator*() const
         {
            return _buf[ _index ];
         }

         /**
          @brief move the iterator on a new x
          */
         template <iint dimensionIncrement>
         ConstDirectionalIterator& add( iint n )
         {
            _index = _mapper.add<dimensionIncrement>( _index, n );
            return *this;
         }

         template <iint dimensionIncrement>
         ConstDirectionalIterator& add()
         {
            _index = _mapper.add<dimensionIncrement>( _index );
            return *this;
         }

         template <iint dimensionIncrement>
         const_return_type pick( iint n )
         {
            const iint index = _mapper.add<dimensionIncrement>( _index, n );
            return _buf[ index ];
         }

         template <iint dimensionIncrement>
         const_return_type pick()
         {
            const iint index = _mapper.add<dimensionIncrement>( _index );
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
         iint                    _index;
         value_type*             _buf;
         Mapper                  _mapper;
      };

      class DirectionalIterator : public ConstDirectionalIterator
      {
      public:
         DirectionalIterator( iint index, const T* buf, const Mapper& mapper ) : ConstDirectionalIterator( index, const_cast<T*>( buf ), mapper )
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

      MemoryStoreContiguous( const Allocator& allocator = Allocator() ) : _storage( allocator ), _mapper( Mapper( Index() ) ),
         // dummy begin/end
         _begin( DirectionalIterator( 0, _storage.getBuf(), _mapper ) ),
         _end( DirectionalIterator( 0, _storage.getBuf(), _mapper ) )
      {}

      MemoryStoreContiguous( const Index& shape, const T& init, const Allocator& allocator = Allocator() ) : _storage( mulList( shape.getBuf(), shape.size() ), false ), _mapper( shape ), _size( shape ),
         _begin( DirectionalIterator( 0, _storage.getBuf(), _mapper ) ),
         _end( DirectionalIterator( mulList( shape.getBuf(), shape.size() ), _storage.getBuf(), _mapper ) )
      {
         for ( Storage::iterator it = _storage.begin(); it != _storage.end(); ++it )
         {
            *it = init;
         }
      }

      MemoryStoreContiguous( const Index& shape, const Allocator& allocator = Allocator() ) : _storage( mulList( shape.getBuf(), shape.size() ), false, allocator ), _mapper( shape ), _size( shape ),
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

      DirectionalIterator getIterator( const Index& index )
      {
         const int linearIndex = _mapper.index( index );
         return DirectionalIterator( linearIndex, _storage.getBuf(), _mapper );
      }

      ConstDirectionalIterator getIterator( const Index& index ) const
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

      const Index& getSize() const
      {
         return _size;
      }

      void print( std::ostream& o )
      {
         o << "[ size=( ";
         for ( Index::const_iterator it = _size.begin() ; it != _size.end(); ++it )
         {
            o << *it << " ";
         }
         o << ") ]" << std::endl;


         //
         // display the data up to 3 dimensions only, it is unlikely to be useful at more dimensions anyway...
         //

         if ( size == 1 )
         {
            for ( const_iterator it = begin() ; it != end(); ++it )
            {
               o << *it << " ";
            }
         }

         if ( size == 2 )
         {
            Index index;
            for ( iint y = 0; y < _size[ 1 ]; ++y )
            {
               index[ 1 ] = y;
               ConstDirectionalIterator it = getIterator( index );
               for ( iint x = 0; x < _size[ 0 ]; ++x, it.add<0>() )
               {
                  o << *it << " ";
               }
               o << std::endl;
            }
         }

         if ( size == 3 )
         {
            Index index;
            for ( iint z = 0; z < _size[ 2 ]; ++z )
            {
               o << "[ z = " << z << " ]" << std::endl;
               index[ 2 ] = z;
               for ( iint y = 0; y < _size[ 1 ]; ++y )
               {
                  index[ 1 ] = y;
                  ConstDirectionalIterator it = getIterator( index );
                  for ( iint x = 0; x < _size[ 0 ]; ++x, it.add<0>() )
                  {
                     o << *it << " ";
                  }
                  o << std::endl;
               }
            }
         }
      }

      const Index& getMemoryAccessOrder() const
      {
         return _mapper.getMemoryAccessOrder();
      }

   private:
      Storage              _storage;
      Mapper               _mapper;
      Index                _size;
      DirectionalIterator  _begin;
      DirectionalIterator  _end;
   };

   /**
    @brief Defines how types are promoted when used with mixed types
    */
   template <class X, class Y>
   struct TraitsPromote
   {
      //typedef type_to_promote   value_type;
   };

   template <class X>
   struct TraitsPromote<X, X>
   {
      typedef X   value_type;
   };

#define DEFINE_TYPE_PROMOTION(T1, T2, TResult)  \
   template<>                                   \
   struct TraitsPromote<T1, T2>                 \
   {                                            \
      typedef TResult   value_type;             \
   };                                           \
                                                \
   template<>                                   \
   struct TraitsPromote<T2, T1>                 \
   {                                            \
      typedef TResult   value_type;             \
   };                                           \

   DEFINE_TYPE_PROMOTION(bool, char,      char);
   DEFINE_TYPE_PROMOTION(bool, short,     short);
   DEFINE_TYPE_PROMOTION(bool, int,       int);
   DEFINE_TYPE_PROMOTION(bool, long,      long);
   DEFINE_TYPE_PROMOTION(bool, float,     float);
   DEFINE_TYPE_PROMOTION(bool, double,    double);

   DEFINE_TYPE_PROMOTION(bool, unsigned char,      unsigned char);
   DEFINE_TYPE_PROMOTION(bool, unsigned short,     unsigned short);
   DEFINE_TYPE_PROMOTION(bool, unsigned int,       unsigned int);
   DEFINE_TYPE_PROMOTION(bool, unsigned long,      unsigned long);

   // type, type
   DEFINE_TYPE_PROMOTION(char, short,     short);
   DEFINE_TYPE_PROMOTION(char, int,       int);
   DEFINE_TYPE_PROMOTION(char, long,      long);
   DEFINE_TYPE_PROMOTION(char, float,     float);
   DEFINE_TYPE_PROMOTION(char, double,    double);

   DEFINE_TYPE_PROMOTION(short, int,       int);
   DEFINE_TYPE_PROMOTION(short, long,      long);
   DEFINE_TYPE_PROMOTION(short, float,     float);
   DEFINE_TYPE_PROMOTION(short, double,    double);

   DEFINE_TYPE_PROMOTION(int, long,      long);
   DEFINE_TYPE_PROMOTION(int, float,     float);
   DEFINE_TYPE_PROMOTION(int, double,    double);

   DEFINE_TYPE_PROMOTION(long, float,    float);
   DEFINE_TYPE_PROMOTION(long, double,   double);

   DEFINE_TYPE_PROMOTION(float, double,  double);

   // unsigned, signed
   DEFINE_TYPE_PROMOTION(unsigned char, short,     short);
   DEFINE_TYPE_PROMOTION(unsigned char, int,       int);
   DEFINE_TYPE_PROMOTION(unsigned char, long,      long);
   DEFINE_TYPE_PROMOTION(unsigned char, float,     float);
   DEFINE_TYPE_PROMOTION(unsigned char, double,    double);

   DEFINE_TYPE_PROMOTION(unsigned short, int,       int);
   DEFINE_TYPE_PROMOTION(unsigned short, long,      long);
   DEFINE_TYPE_PROMOTION(unsigned short, float,     float);
   DEFINE_TYPE_PROMOTION(unsigned short, double,    double);

   DEFINE_TYPE_PROMOTION(unsigned int, long,      long);
   DEFINE_TYPE_PROMOTION(unsigned int, float,     float);
   DEFINE_TYPE_PROMOTION(unsigned int, double,    double);

   DEFINE_TYPE_PROMOTION(unsigned long, float,    float);
   DEFINE_TYPE_PROMOTION(unsigned long, double,   double);

   // signed, unsigned
   DEFINE_TYPE_PROMOTION(char,  unsigned short,     short);
   DEFINE_TYPE_PROMOTION(char,  unsigned int,       int);
   DEFINE_TYPE_PROMOTION(char,  unsigned long,      long);
   DEFINE_TYPE_PROMOTION(short, unsigned int,       int);
   DEFINE_TYPE_PROMOTION(short, unsigned long,      long);
   DEFINE_TYPE_PROMOTION(int,   unsigned long,      long);
  
   DEFINE_TYPE_PROMOTION(unsigned char,   char,    char);
   DEFINE_TYPE_PROMOTION(unsigned short,  short,   short);
   DEFINE_TYPE_PROMOTION(unsigned int,    int,     int);
   DEFINE_TYPE_PROMOTION(unsigned long,   long,    long);


   template<class Type>
   struct VectorizationTraits
   {};

   template<> struct VectorizationTraits<int>
   {
      typedef int        value_type;
      typedef __m128i    vec_type;

      static inline vec_type init(const int x)
      {
         return (vec_type)_mm_set1_epi32(x);
      }
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

         iint size = array.end() - array.begin();
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

         iint size = array.end() - array.begin();
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
      const iint BufferSize = 200000;
      const iint NbIteration = 1000;

      typedef core::MemoryStoreContiguous<iint, 1> Array;
      core::StaticVector<iint, 1> size;
      size[ 0 ] = BufferSize;
      Array array( size );
      array.begin();

      const Array array2 = array;

      iint accumRef = 0;
      for ( Array::iterator it = array.begin(); it != array.end(); ++it )
      {
         const iint val = rand() % 256;
         *it = val;
         accumRef += val;
      }

      // test basic const iterators
      core::Timer timerConst;
      for ( iint iter = 0; iter < NbIteration; ++iter )
      {
         iint accum = 0;
         for ( Array::ConstDirectionalIterator it = array.beginDirectional(); it != array.endDirectional(); it.add<0>() )
         {
            accum += *it;
         }
         TESTER_ASSERT( accumRef == accum );
      }

      const double timeConst = timerConst.getCurrentTime();

      // test basic non const iterators
      core::Timer timerNonConst;
      for ( iint iter = 0; iter < NbIteration; ++iter )
      {
         iint accum = 0;
         for ( Array::DirectionalIterator it = array.beginDirectional(); it != array.endDirectional(); it.add<0>() )
         {
            accum += *it;
         }
         TESTER_ASSERT( accumRef == accum );
      }

      const double timeNonConst = timerNonConst.getCurrentTime();

      // test non const iter
      core::Timer timerNonConstDir0;
      for ( iint iter = 0; iter < NbIteration; ++iter )
      {
         iint accum = 0;
         for ( Array::DirectionalIterator it = array.beginDirectional(); it != array.endDirectional(); )
         {
            accum += *it;
            it.add<0>();
         }
         TESTER_ASSERT( accumRef == accum );
      }

      const double timeNonConstDir0 = timerNonConstDir0.getCurrentTime();

      // test raw pointers
      core::Timer timerRaw;
      for ( iint iter = 0; iter < NbIteration; ++iter )
      {
         iint accum = 0;
         const iint* start = array.begin();
         const iint* end = array.end();
         for ( ; start != end; ++start )
         {
            accum += *start;
         }
         TESTER_ASSERT( accumRef == accum );
      }

      const double timeRaw = timerRaw.getCurrentTime();

      std::cout << "const iterator time=" << timeConst << std::endl;
      std::cout << "non const iterator time=" << timeNonConst << std::endl;
      std::cout << "non const iterator direction0 time=" << timeNonConstDir0 << std::endl;
      std::cout << "Raw pointer time=" << timeRaw << std::endl;

      // we should have exactly the same speed for all three
      const double tol = 0.1 * timeRaw;
      TESTER_ASSERT( core::equal<double>( timeRaw, timeConst, tol ) );
      TESTER_ASSERT( core::equal<double>( timeRaw, timeNonConst, tol ) );
      TESTER_ASSERT( core::equal<double>( timeRaw, timeNonConstDir0, tol ) );
   }

   void testMemoryStoreContiguous_mapping()
   {
      {
         typedef core::MemoryStoreContiguous<float, 2, core::ArrayMapperContiguousColumnMajor<2> > Array;
         Array array( core::vector2i( 2, 3 ) );

         float c = 0;
         for ( Array::iterator it = array.begin(); it != array.end(); ++it )
         {
            *it = ++c;
         }

         array.print( std::cout );

         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 0, 0 ) ), 1.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 0, 1 ) ), 2.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 0, 2 ) ), 3.0f, 1e-5f ) );

         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 1, 0 ) ), 4.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 1, 1 ) ), 5.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 1, 2 ) ), 6.0f, 1e-5f ) );

         TESTER_ASSERT( array.getMemoryAccessOrder()[ 0 ] == 1 );
         TESTER_ASSERT( array.getMemoryAccessOrder()[ 1 ] == 0 );
      }
      
      {
         typedef core::MemoryStoreContiguous<float, 2> Array;
         Array array( core::vector2i( 2, 3 ) );

         float c = 0;
         for ( Array::iterator it = array.begin(); it != array.end(); ++it )
         {
            *it = ++c;
         }

         array.print( std::cout );

         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 0, 0 ) ), 1.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 1, 0 ) ), 2.0f, 1e-5f ) );

         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 0, 1 ) ), 3.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 1, 1 ) ), 4.0f, 1e-5f ) );

         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 0, 2 ) ), 5.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector2i( 1, 2 ) ), 6.0f, 1e-5f ) );

         TESTER_ASSERT( array.getMemoryAccessOrder()[ 0 ] == 0 );
         TESTER_ASSERT( array.getMemoryAccessOrder()[ 1 ] == 1 );
      }

      {
         typedef core::MemoryStoreContiguous<float, 3> Array;
         Array array( core::vector3i( 2, 3, 2 ) );

         float c = 0;
         for ( Array::iterator it = array.begin(); it != array.end(); ++it )
         {
            *it = ++c;
         }

         array.print( std::cout );

         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 0, 0, 0 ) ), 1.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 1, 0, 0 ) ), 2.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 0, 1, 0 ) ), 3.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 1, 1, 0 ) ), 4.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 0, 2, 0 ) ), 5.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 1, 2, 0 ) ), 6.0f, 1e-5f ) );

         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 0, 0, 1 ) ), 7.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 1, 0, 1 ) ), 8.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 0, 1, 1 ) ), 9.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 1, 1, 1 ) ), 10.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 0, 2, 1 ) ), 11.0f, 1e-5f ) );
         TESTER_ASSERT( core::equal( *array.getIterator( core::vector3i( 1, 2, 1 ) ), 12.0f, 1e-5f ) );

         TESTER_ASSERT( array.getMemoryAccessOrder()[ 0 ] == 0 );
         TESTER_ASSERT( array.getMemoryAccessOrder()[ 1 ] == 1 );
         TESTER_ASSERT( array.getMemoryAccessOrder()[ 2 ] == 2 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestArray);
TESTER_TEST(testMemoryStoreContiguous);
TESTER_TEST(testMemoryStoreContiguous_speed_1d);
TESTER_TEST(testMemoryStoreContiguous_mapping);
TESTER_TEST_SUITE_END();
#endif