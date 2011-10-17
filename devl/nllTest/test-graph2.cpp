#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"


using namespace nll;
using namespace nll::core;

#define NLL_GRAPH_SECURE_CHECKS

namespace nll
{
namespace core
{
   class Empty
   {};

   class Directed
   {};

   class Undirected
   {};

   class MapperVector
   {};

   class MapperList
   {};

   class MapperSet
   {};

   /**
    @brief Wrapper on the different possible storage classes <MapperVector> <MapperList> <MapperSet>

    <DataInput> requires a unique id <getId()> method returning a unique identifier

    The class must define types:
       DataInput
       DataStorage
       MapperType
       Container
       iterator
       const_iterator
       Descriptor
    The class must implement:
       const DataStorage& operator[]( const DataInput& d ) const
       DataStorage& operator[]( const DataInput& d )
       void add( const DataInput& d, class DataStorage& data )
       void erase( const DataInput& d )
       typename const_iterator find( const DataInput& d ) const
       typename iterator find( const DataInput& d )
    */
   template <class DataStorageT, class MapperTypeT>
   class Mapper
   {
   public:
      //typedef DataInputT               DataInput;
      typedef DataStorageT             DataStorage;
      typedef MapperTypeT              MapperType;
      typedef std::vector<DataStorage> Container;
      typedef typename Container::iterator iterator;
      typedef typename Container::const_iterator const_iterator;

      class Descriptor
      {
      };
/*
      Descriptor get( const const_iterator& it ) const
      {
         ensure( 0, "data specialization not implemented" );
      }

      const DataStorage& operator[]( const DataInput& d ) const
      {
         ensure( 0, "data specialization not implemented" );
      }

      DataStorage& operator[]( const DataInput& d )
      {
         ensure( 0, "data specialization not implemented" );
      }

      void add( const DataInput& d, const DataStorage& data )
      {
         ensure( 0, "data specialization not implemented" );
      }

      void erase( const DataInput& d )
      {
         ensure( 0, "data specialization not implemented" );
      }

      typename const_iterator find( const DataInput& d ) const
      {
         ensure( 0, "data specialization not implemented" );
      }

      typename iterator find( const DataInput& d )
      {
         ensure( 0, "data specialization not implemented" );
      }*/
   };

   template <class DataStorageT>
   class Mapper<DataStorageT, MapperVector>
   {
   public:
      typedef size_t   Uid;

   private:
      // private support classes
      class DataStorageWrapper
      {
      public:
         DataStorageWrapper( const DataStorageT& d, ui32 uidi, ui32 uidPerVectori ) : data( d ), uid( uidi ), uidPerVector( uidPerVectori )
         {}

         DataStorageT data;
         ui32         uid;              // unique identifier, valid all the time
         ui32         uidPerVector;     // temporary identifier for quick look up, invalid as soon as an element is erased, so it must be updated
      };
      typedef std::vector<DataStorageWrapper>                DataStorageWrappers;
      typedef typename DataStorageWrappers::iterator         iteratorImpl;
      typedef typename DataStorageWrappers::const_iterator   const_iteratorImpl;

   public:
      typedef DataStorageT             DataStorage;
      typedef MapperVector             MapperType;

      class iterator
      {
      public:
         iterator( const_iteratorImpl it ) : _it( it )
         {}

         iterator& operator++()
         {
            ++_it;
            return *this;
         }

         DataStorage& operator*()
         {
            const DataStorage& data = _it->data;
            return const_cast<DataStorage&>( data );
         }

         bool operator!=( const iterator& it ) const
         {
            return _it != it._it;
         }

         const_iteratorImpl& getIterator()
         {
            return _it;
         };

      protected:
         const_iteratorImpl   _it;
      };

      class const_iterator : public iterator
      {
      public:
         const_iterator( const_iteratorImpl it ) : iterator( it )
         {}

         const_iterator( iterator it ) : iterator( it )
         {}

         bool operator!=( const const_iterator& it ) const
         {
            return _it != it._it;
         }

         const DataStorage& operator*() const
         {
            return _it->data;
         }
      };

      // lightweight descriptor pointing to a DataStorage
      class Descriptor
      {
         friend Mapper;

      private:
         Descriptor( ui32 uidi, ui32 uidPerVectori ) : uid( uidi ), uidPerVector( uidPerVectori )
         {}

         ui32 uid;               // unique identifier, valid all the time
         ui32 uidPerVector;      // temporary identifier for quick look up, invalid as soon as an element is erased, so it must be updated
      };

      /**
       @brief Quick access to find the data. Internally using the quick index, wich is invalidated while erasing elements
       */
      DataStorage& getData( const Descriptor& d )
      {
         return *getIterator( d );
      }
      const DataStorage& getData( const Descriptor& d ) const
      {
         return *getIterator( d );
      }

      /**
       @brief Use the quick index to find the corresponding iterator
       */
      iterator getIterator( const Descriptor& d ) const
      {
         return _wrappers.begin() + d.uidPerVector;
      }

      /**
       @brief Given the descriptor, do a full search on the unique id to find an iterator.
              This is to handle cases with iterator invalidation while erasing elements
       */
      iterator find( const Descriptor& d ) const
      {
         for ( iterator it = _wrappers.begin(); it != _wrappers.end(); ++it )
         {
            if ( it.getIterator()->uid == d.uid )
               return it;
         }
         return _wrappers.end();
      }

      Descriptor insert( const DataStorage& data )
      {
         _wrappers.push_back( createWrapper( data ) );
         return get( _wrappers.rbegin() );
      }

      void erase( const DataStorage& d )
      {
         ensure( 0, "data specialization not implemented" );
      }

      iterator begin() const
      {
         return iterator( _wrappers.begin() );
      }

      iterator end() const
      {
         return iterator( _wrappers.end() );
      }

      size_t size() const
      {
         return _wrappers.size();
      }

   private:
      DataStorageWrapper createWrapper( const DataStorage& data )
      {
         return DataStorageWrapper( data, data.getUid(), (ui32)_wrappers.size() );
      }

      template <class iter>
      Descriptor get( const iter& it ) const
      {
         const ui32 uid = it->uid;
         const ui32 uidPerVector = it->uidPerVector;
         return Descriptor( uid, uidPerVector );
      }

   private:
      DataStorageWrappers      _wrappers;
   };

   class Graph
   {
   public:
      class Edge;
      class Vertex;
      typedef Mapper< Vertex, MapperVector > Vertexs;

      class Edge
      {
      public:
         Edge( ui32 uid, Vertexs::Descriptor& from, Vertexs::Descriptor& to ) : _uid( uid ), fromVertex( from ), toVertex( to )
         {}

         ui32 getUid() const
         {
            return _uid;
         }

         Vertexs::Descriptor fromVertex;
         Vertexs::Descriptor toVertex;

      private:
         ui32  _uid;
      };
      typedef Mapper< Edge, MapperVector > Edges;

      class Vertex
      {
      public:
         Vertex( ui32 uidi ) : uid( uidi )
         {}

         ui32 getUid() const
         {
            return uid;
         }

         ui32  uid;
      };

      typedef Vertexs::Descriptor      VertexDescriptor;
      typedef Edges::Descriptor        EdgeDescriptor;

   public:
      VertexDescriptor addVertex()
      {
         return _vertexs.insert( Vertex( _vertexs.size() ) );
      }

   private:
      Vertexs  _vertexs;
   };
}
}


class TestGraph2
{
public:
   template <class T>
   class DataUq
   {
   public:
      DataUq( T v ) : _v( v )
      {
         static ui32 n = 0;
         _uid = ++n;
      }

      ui32 getUid() const
      {
         return _uid;
      }

      bool operator==( const DataUq& d ) const
      {
         return _v == d._v && _uid == d._uid;
      }

      T        _v;
      ui32     _uid;
   };

   void test()
   {
      typedef DataUq<int> DataUq1;
      typedef Mapper<DataUq1, MapperVector> Mapper1;
      Mapper1  mapper1;

      const Mapper1&  mapper2 = mapper1;

      int dav[] =
      {
         1, 2, 3
      };

      DataUq1 da1( dav[ 0 ] );
      Mapper1::Descriptor d1 = mapper1.insert( da1 );
      DataUq1 da2( dav[ 1 ] );
      Mapper1::Descriptor d2 = mapper1.insert( da2 );
      DataUq1 da3( dav[ 2 ] );
      Mapper1::Descriptor d3 = mapper1.insert( da3 );

      TESTER_ASSERT( ( da1 == mapper1.getData( d1 ) ) );
      TESTER_ASSERT( ( da2 == mapper1.getData( d2 ) ) );
      TESTER_ASSERT( ( da3 == mapper1.getData( d3 ) ) );

      TESTER_ASSERT( ( da1 == mapper2.getData( d1 ) ) );
      TESTER_ASSERT( ( da2 == mapper2.getData( d2 ) ) );
      TESTER_ASSERT( ( da3 == mapper2.getData( d3 ) ) );

      Mapper1::iterator it1 = mapper1.find( d1 );
      Mapper1::const_iterator cit1( it1 );
      TESTER_ASSERT( !( Mapper1::const_iterator(it1) != cit1 ) );

      Mapper1::const_iterator it1b = mapper2.find( d1 );
      TESTER_ASSERT( !( Mapper1::const_iterator(it1b) != cit1 ) );

      Mapper1::const_iterator it2 = mapper1.find( d2 );
      Mapper1::const_iterator it3 = mapper1.find( d3 );

      TESTER_ASSERT( ( da1 == *it1 ) );
      TESTER_ASSERT( ( da2 == *it2 ) );
      TESTER_ASSERT( ( da3 == *it3 ) );

      Mapper1::const_iterator cit2 = mapper1.getIterator( d2 );
      TESTER_ASSERT( ( da2 == *cit2 ) );

      int id = 1;
      for ( Mapper1::const_iterator it = mapper1.begin(); it != mapper1.end(); ++it, ++id )
      {
         int res = (*it)._v;
         TESTER_ASSERT( res == id );
      }

      id = 1;
      for ( Mapper1::iterator it = mapper1.begin(); it != mapper1.end(); ++it, ++id )
      {
         int res = (*it)._v;
         TESTER_ASSERT( res == id );
      }
   }

   void testGraph()
   {
      Graph g;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGraph2);
TESTER_TEST( test );
TESTER_TEST( testGraph );
TESTER_TEST_SUITE_END();
#endif