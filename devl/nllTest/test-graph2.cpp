#include <nll/nll.h>
#include <stack>
#include <tester/register.h>
#include "config.h"


using namespace nll;
using namespace nll::core;

#define NLL_GRAPH_SECURE_CHECKS

namespace nll
{
namespace core
{
   class MapperVector
   {};

   class MapperList
   {};

   class MapperSet
   {};

   /**
    @brief Wrapper on the different possible storage classes <MapperVector> <MapperList> <MapperSet>

    The <Mapper> must define the types:
      Descriptor
      iterator
      const_iterator
      Uid
      DataStorage

    <Mapper> must define the operations:
      Mapper()
      DataStorage& getData( const Descriptor& d )
      const DataStorage& getData( const Descriptor& d ) const
      const_iterator getIterator( const Descriptor& d ) const
      iterator getIterator( const Descriptor& d )
      const_iterator find( const Descriptor& d ) const
      iterator find( const Descriptor& d )
      Descriptor insert( const DataStorage& data )
      iterator erase( const iterator& it )
      const_iterator begin() const
      iterator begin()
      const_iterator end() const
      iterator end()
      size_t size() const
      Descriptor get( const iterator& it ) const

    <Descriptor> must define the operations:
      Uid getUid() const      // Uids need not be compact

    Concept:
      Descriptor/iterators must be lightweight as they will be often copied by value
    */
   template <class DataStorageT, class MapperTypeT>
   class Mapper
   {
      // need specialization
   };

   /**
    @brief Mapper implemented in terms of std::set

    N = nb elements
    Insertion/deletion is o(log N)
    get iterators, find and iterator->descriptor conversions are o(1)
    descriptors and iterators are never invalidated if deletion/insertion
    */
   template <class DataStorageT>
   class Mapper<DataStorageT, MapperSet>
   {
   public:
      typedef size_t             Uid;
      typedef DataStorageT       DataStorage;
      typedef MapperVector       MapperType;

   private:
      // private support classes
      class CompareData
      {
      public:
         bool operator()( const DataStorage& d1, const DataStorage& d2 ) const
         {
            return d1.getUid() < d2.getUid();
         }
      };
      typedef std::set<DataStorage, CompareData>             DataStore;

   public:
      typedef typename DataStore::iterator         iterator;
      typedef typename DataStore::const_iterator   const_iterator;

      // we can directly hold an iterator as they are not invalidated by insertion/deletion
      class Descriptor
      {
         friend Mapper;

      public:
         Descriptor( iterator it ) : _it( it )
         {}

      private:
         iterator _it;
      };

      Descriptor insert( const DataStorage& data )
      {
         std::pair<iterator, bool> p = _store.insert( data );
         return Descriptor( p.first );
      }

      DataStorage& getData( const Descriptor& d )
      {
         return *d._it;
      }

      const DataStorage& getData( const Descriptor& d ) const
      {
         return *d._it;
      }

      iterator find( const Descriptor& d )
      {
         return d._it;
      }

      const_iterator find( const Descriptor& d ) const
      {
         return d._it;
      }

      const_iterator getIterator( const Descriptor& d ) const
      {
         return find( d );
      }

      iterator getIterator( const Descriptor& d )
      {
         return find( d );
      }

      const_iterator begin() const
      {
         return _store.begin();
      }

      iterator begin()
      {
         return _store.begin();
      }

      const_iterator end() const
      {
         return _store.end();
      }

      iterator end()
      {
         return _store.end();
      }

      size_t size() const
      {
         return _store.size();
      }

      iterator erase( const iterator& it )
      {
         return _store.erase( it );
      }

      Descriptor get( const iterator& it ) const
      {
         Descriptor( it );
      }

   private:
      DataStore _store;
   };

   /**
    @brief Create a mapper with a contiguous vector implementation

    Note: there are two mecanism to access data via:
             - direct access with a cached UID, this is assuming the descriptor was not invalidated
             - finding a generic UID, this always work even if the descriptor is invalidated. This is the typical
               way of handling invalidated descriptor
          When a data is removed from the structure, it will invalidate all descriptors and iterators! If this is not
          acceptable then another implementation should be used (e.g., MapperList or MapperSet)
    */
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
         DataStorageWrapper( const DataStorageT& d, Uid uidi, Uid uidPerVectori ) : data( d ), uid( uidi ), uidPerVector( uidPerVectori )
         {}

         DataStorageT data;
         Uid          uid;              // unique identifier, valid all the time
         mutable Uid  uidPerVector;     // temporary identifier for quick look up, invalid as soon as an element is erased, so it must be updated
      };
      typedef std::vector<DataStorageWrapper>                DataStorageWrappers;
      typedef typename DataStorageWrappers::iterator         iteratorImpl;
      typedef typename DataStorageWrappers::const_iterator   const_iteratorImpl;

   public:
      typedef DataStorageT             DataStorage;
      typedef MapperVector             MapperType;

      class const_iterator;
      class iterator
      {
         friend const_iterator;
         friend Mapper;

      public:
         explicit iterator( iteratorImpl it ) : _it( it )
         {}

         iterator& operator++()
         {
            ++_it;
            return *this;
         }

         DataStorage& operator*() const
         {
            const DataStorage& data = _it->data;
            return const_cast<DataStorage&>( data );
         }

         bool operator!=( const iterator& it ) const
         {
            return _it != it._it;
         }

      private:
         iteratorImpl   _it;
      };

      class const_iterator
      {
      public:
         const_iterator( const_iteratorImpl it ) : _it( it )
         {}

         const_iterator( iterator it ) : _it( it._it )
         {}

         bool operator!=( const const_iterator& it ) const
         {
            return _it != it._it;
         }

         const_iterator& operator++()
         {
            ++_it;
            return *this;
         }

         const DataStorage& operator*() const
         {
            return _it->data;
         }

      private:
         const_iteratorImpl   _it;
      };

      // lightweight descriptor pointing to a DataStorage
      class Descriptor
      {
         friend Mapper;

      public:
         Uid getUid() const
         {
            return uid;
         }

      private:
         Descriptor( Uid uidi, Uid uidPerVectori ) : uid( uidi ), uidPerVector( uidPerVectori )
         {}

         Uid uid;                   // unique identifier, valid all the time
         mutable Uid uidPerVector;  // temporary identifier for quick look up, invalid as soon as an element is erased, so it must be updated
      };

      Mapper( size_t size, const DataStorageT val ) : _wrappers( size, createWrapper( val ) )
      {
      }

      Mapper()
      {
      }

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
      const_iterator getIterator( const Descriptor& d ) const
      {
         return const_iterator( _wrappers.begin() + d.uidPerVector );
      }

      iterator getIterator( const Descriptor& d )
      {
         return iterator( _wrappers.begin() + d.uidPerVector );
      }

      /**
       @brief Given the descriptor, do a full search on the unique id to find an iterator.
              This is to handle cases with iterator invalidation while erasing elements
       */
      const_iterator find( const Descriptor& d ) const
      {
         for ( const_iteratorImpl it = _wrappers.begin(); it != _wrappers.end(); ++it )
         {
            const DataStorageWrapper& wrapper = *it;
            if ( wrapper.uid == d.uid )
            {
               return const_iterator( it );
            }
         }
         return const_iterator( _wrappers.end() );
      }

      iterator find( const Descriptor& d )
      {
         for ( iteratorImpl it = _wrappers.begin(); it != _wrappers.end(); ++it )
         {
            DataStorageWrapper& wrapper = *it;
            if ( wrapper.uid == d.uid )
            {
               return iterator( it );
            }
         }
         return iterator( _wrappers.end() );
      }

      Descriptor insert( const DataStorage& data )
      {
         _wrappers.push_back( createWrapper( data ) );

         const Uid uid = _wrappers.rbegin()->uid;
         const Uid uidPerVector = _wrappers.rbegin()->uidPerVector;
         return Descriptor( uid, uidPerVector );
      }

      iterator erase( const iterator& it )
      {
         iteratorImpl newIt = _wrappers.erase( it._it );
         return iterator( newIt );
      }

      const_iterator begin() const
      {
         return const_iterator( _wrappers.begin() );
      }

      iterator begin()
      {
         return iterator( _wrappers.begin() );
      }

      const_iterator end() const
      {
         return const_iterator( _wrappers.end() );
      }

      iterator end()
      {
         return iterator( _wrappers.end() );
      }

      size_t size() const
      {
         return _wrappers.size();
      }

      Descriptor get( const iterator& it ) const
      {
         const Uid uid = it->uid;
         const Uid uidPerVector = it->uidPerVector;
         return Descriptor( uid, uidPerVector );
      }

   private:
      DataStorageWrapper createWrapper( const DataStorage& data )
      {
         return DataStorageWrapper( data, data.getUid(), _wrappers.size() );
      }

   private:
      DataStorageWrappers      _wrappers;
   };

   /**
    @brief Graph with adjacency list implementation

    It is assuming the UID generation for the Edges and Vertexes are compact (i.e. there is no holes,
    or they will be filled at the next insertion). This is to ensure that the data can be retrieved
    in o(1) if necessary (but at the cost of more memory consumption)

    N = nb vertices, E = nb edges, mE = mean nb edges
    VertexType = MapperVector | EdgeType = MapperVector
      vertex add,          complexity time on average o(1)
      vertex erase,        complexity time o(E+N)
      edge add,            complexity time on average o(1)
      edge erase,          complexity time on average o(1)
      vertex iterator get, complexity time o(1)
      edge iterator get,   complexity time o(1)

    VertexType = MapperSet | EdgeType = MapperSet
      vertex add,          complexity time o(log(N))
      vertex erase,        complexity time o(log(N)+E)
      edge add,            complexity time on average o(log mE)
      edge erase,          complexity time on average o(log mE)
      vertex iterator get, complexity time on average o(log N)
      edge iterator get,   complexity time on average o(log mE)

    Adding/retriving data using data mapper if o(1) on average. This is achieved by using a global ID (hence
    the constraint of UID compactness). The data mapper will only grow and never decrease...

    iterator invalidation:
      VertexType = MapperVector
        add vertex         all iterator invalidated
        erase vertex       all iterator invalidated after erase position
      VertexType = MapperSet
        add vertex         all iterator valid
        erase vertex       all iterator valid
      EdgeType = MapperVector
        add edge           all iterator invalidated
        erase edge         all iterator invalidated after erase position
      EdgeType = MapperSet
        add edge           all iterator valid
        erase edge         all iterator valid

    When an iterator is invalidated, the corresponding descriptor is also invalidated, except
    for the method <getSafe> which will "find" the corresponding element Indeed, for some mapper
    type, some indexing will be done but won't be valid anymore after iterator invalidation
    and must be recalculated
    */
   template <class VertexStorageTypeT = MapperVector, class EdgeStorageTypeT = MapperVector>
   class Graph
   {
   public:
      typedef VertexStorageTypeT             VertexStorageType;
      typedef EdgeStorageTypeT               EdgeStorageType;
      class Edge;
      class Vertex;
      typedef Mapper< Vertex, MapperVector > Vertexs;
      typedef typename Vertexs::iterator              vertex_iterator;
      typedef typename Vertexs::const_iterator        const_vertex_iterator;
      typedef typename Vertexs::Uid                   Uid;

   private:
      /**
       @brief Data mapper main interface to allow the data mapper observe graph modifications
       */
      class DataMapperInterface
      {
         friend Graph;

      public:
         virtual ~DataMapperInterface()
         {}

      protected:
         // if the graph is destroyed before the mapper, stop observing...
         virtual void setUnobserved() = 0;

         // will be called by the graph when a UID is being destroyed
         virtual void erase( Uid uid ) = 0;

         // will be called by the graph when a UID is being added
         virtual void add( Uid uid ) = 0;
      };
      typedef std::list<DataMapperInterface*> DataMapperInterfaces;

   public:
      class Edge
      {
         friend Graph;

         Edge( Uid uid, const typename Vertexs::Descriptor& source, const typename Vertexs::Descriptor& destination ) : _uid( uid ), src( source ), dst( destination )
         {}

      public:
         Uid getUid() const
         {
            return _uid;
         }

      private:
         typename Vertexs::Descriptor src;
         typename Vertexs::Descriptor dst;

      private:
         Uid  _uid;
      };
      typedef Mapper< Edge, MapperVector >   Edges;
      typedef typename Edges::iterator       edge_iterator;
      typedef typename Edges::const_iterator const_edge_iterator;

      class Vertex
      {
         friend Graph;

         Vertex( Uid uid ) : _uid( uid )
         {}

      public:
         Uid getUid() const
         {
            return _uid;
         }

         const_edge_iterator begin() const
         {
            return _edges.begin();
         }

         edge_iterator begin()
         {
            return _edges.begin();
         }

         edge_iterator end()
         {
            return _edges.end();
         }

         const_edge_iterator end() const
         {
            return _edges.end();
         }

      private:
         Uid   _uid;
         Edges _edges;
      };

      typedef typename Vertexs::Descriptor      VertexDescriptor;
      typedef typename Edges::Descriptor        EdgeDescriptorImpl;

      class EdgeDescriptor
      {
         friend Graph;

      public:
         EdgeDescriptor( const VertexDescriptor& src, const EdgeDescriptorImpl& d ) : _src( src ), _desc( d )
         {}

      private:
         VertexDescriptor     _src;
         EdgeDescriptorImpl   _desc;
      };

   public:
      //
      // Here we handle the observer methods and notify them everytime the graph has been updated
      //
      void registerEdgeObserver( DataMapperInterface* i )
      {
         _edgesObserver.push_back( i );
      }

      void registerVertexObserver( DataMapperInterface* i )
      {
         _vertexsObserver.push_back( i );
      }

      void unregisterObserver( DataMapperInterface* i )
      {
         DataMapperInterfaces::iterator it = std::find( _vertexsObserver.begin(), _vertexsObserver.end(), i );
         if ( it != _vertexsObserver.end() )
         {
            _vertexsObserver.erase( it );
            return;
         }

         it = std::find( _edgesObserver.begin(), _edgesObserver.end(), i );
         if ( it != _edgesObserver.end() )
         {
            _edgesObserver.erase( it );
         }
      }

      /**
       @brief Object to retrieve efficiently data associated to vertices and edges
       */
      template <class T, class StorageImpl>
      class DataMapper : public DataMapperInterface
      {
      protected:
         typedef std::vector<T>     Storage;

      public:
         DataMapper( Graph& g, const T val = T() ) : _g( &g ), _storage( g.size(), val ), _default( val )
         {
         }

         virtual ~DataMapper()
         {
            if ( _g )
            {
               _g->unregisterObserver( this );
            }
         }

      protected:
         // means the graph has been destroyed before the data mapper...
         virtual void setUnobserved()
         {
            _g = 0;
         }

         template <class Descriptor>
         const T& operator[]( const Descriptor& desc ) const
         {
            Uid uid = desc.getUid();
            return _storage[ uid ];
         }

         template <class Descriptor>
         T& operator[]( const Descriptor& desc )
         {
            Uid uid = desc.getUid();
            return _storage[ uid ];
         }

      protected:
         Graph*      _g;
         Storage     _storage;
         T           _default;
      };

      /**
       @brief Specialization for the vertex data, complexity o(1) op for accessing o(N) in memory
       */
      template <class T>
      class VertexMapper : public DataMapper<T, std::vector<T> >
      {
         typedef DataMapper<T, std::vector<T> > Base;

      public:
         VertexMapper( Graph& g, const T val = T() ) : DataMapper( g, val )
         {
            _g->registerVertexObserver( this );
         }

         const T& operator[]( const VertexDescriptor& desc ) const
         {
            return Base::operator[]( desc );
         }

         T& operator[]( const VertexDescriptor& desc )
         {
            return Base::operator[]( desc );
         }

         virtual void erase( Uid uid ) 
         {
            Storage::iterator it = _storage.begin() + uid;
            _storage.erase( it );
         }

         virtual void add( Uid uid )
         {
            // if the UID is >= _storage.size it means the UID the compactness assumption is wrong,
            // or it is the wrong container
            assert( uid <= _storage.size() );
            if ( uid == _storage.size() )
            {
               _storage.push_back( _default );
            }
         }
      };

      /**
       @brief Specialization for the edge data, complexity o(1) op for accessing o(N) in memory
       */
      template <class T>
      class EdgeMapper : public DataMapper<T, std::vector<T> >
      {
         typedef DataMapper<T, std::vector<T> > Base;

      public:
         EdgeMapper( Graph& g, const T val = T() ) : DataMapper( g, val )
         {
            _g->registerEdgeObserver( this );
         }

         const T& operator[]( const EdgeDescriptor& desc ) const
         {
            return Base::operator[]( desc._desc );
         }

         T& operator[]( const EdgeDescriptor& desc )
         {
            return Base::operator[]( desc._desc );
         }

         virtual void erase( Uid uid ) 
         {
            Storage::iterator it = _storage.begin() + uid;
            _storage.erase( it );
         }

         virtual void add( Uid uid )
         {
            // if the UID is >= _storage.size it means the UID the compactness assumption is wrong,
            // or it is the wrong container
            assert( uid <= _storage.size() );
            if ( uid == _storage.size() )
            {
               _storage.push_back( _default );
            }
         }
      };

      Graph() : _uidVertexGenerator( 0 ), _uidEdgeGenerator( 0 )
      {
      }

      ~Graph()
      {
         // we need to unregister the observers so that it doesn't cause problems if the graph is destroyed first...
         for ( DataMapperInterfaces::iterator it = _vertexsObserver.begin(); it != _vertexsObserver.end(); ++it )
         {
            (*it)->setUnobserved();
         }

         for ( DataMapperInterfaces::iterator it = _edgesObserver.begin(); it != _edgesObserver.end(); ++it )
         {
            (*it)->setUnobserved();
         }
      }

      //
      // Graph modification operations
      //

      VertexDescriptor addVertex()
      {
         Uid uid;
         if ( _uidVertexRecyling.empty() )
         {
            // generate a new UID
            uid = _uidVertexGenerator;
            ++_uidVertexGenerator;
         } else {
            // recycle a UID
            uid = _uidVertexRecyling.top();
            _uidVertexRecyling.pop();
         }

         // notify oberservers
         for ( DataMapperInterfaces::iterator it = _vertexsObserver.begin(); it != _vertexsObserver.end(); ++it )
         {
            (*it)->add( uid );
         }
         return _vertexs.insert( Vertex( uid ) );
      }

      edge_iterator erase( const edge_iterator& it )
      {
         // get the source vertex
         Edge& e = *it;
         Vertex& v = _vertexs.getData( e.src );
         const Uid uid = e.getUid();
         _uidEdgeRecyling.push( uid );   // here we recycle the UID to be reused for next <add>
         return v._edges.erase( it );
      }

      /*
      vertex_iterator erase( const vertex_iterator& it )
      {
         Vertex& v = *it;

      }*/

      edge_iterator erase( const EdgeDescriptor& desc )
      {
         // from the descriptor we need to know the source so that we can lookup the edge iterator
         edge_iterator it = get( desc );
         return erase( it );
      }

      EdgeDescriptor addEdge( const VertexDescriptor& src, const VertexDescriptor& dst )
      {
         Uid uid;
         if ( _uidEdgeRecyling.empty() )
         {
            // generate a new UID
            uid = _uidEdgeGenerator;
            ++_uidEdgeGenerator;
         } else {

            // reuse a UID
            uid = _uidEdgeRecyling.top();
            _uidEdgeRecyling.pop();
         }

         // notify the observers
         for ( DataMapperInterfaces::iterator it = _edgesObserver.begin(); it != _edgesObserver.end(); ++it )
         {
            (*it)->add( uid );
         }

         // finally add a new edge
         vertex_iterator it = _vertexs.getIterator( src );
         return EdgeDescriptor( src, (*it)._edges.insert( Edge( uid, src, dst ) ) );
      }

      //
      // Accessors
      //

      const_vertex_iterator begin() const
      {
         return _vertexs.begin();
      }

      const_vertex_iterator end() const
      {
         return _vertexs.end();
      }

      vertex_iterator begin()
      {
         return _vertexs.begin();
      }

      vertex_iterator end()
      {
         return _vertexs.end();
      }

      size_t size() const
      {
         return _vertexs.size();
      }

      //
      // Converters descriptor->iterator
      //

      edge_iterator get( const EdgeDescriptor& desc )
      {
         // first get the source vertex
         vertex_iterator it = _vertexs.getIterator( desc._src );
         assert( it != _vertexs.end() );

         return (*it)._edges.getIterator( desc._desc );
      }

      const_edge_iterator get( const EdgeDescriptor& desc ) const
      {
         // first get the source vertex
         const_vertex_iterator it = _vertexs.getIterator( desc._src );
         assert( it != _vertexs.end() );

         return (*it)._edges.getIterator( desc._desc );
      }

      vertex_iterator get( const VertexDescriptor& desc )
      {
         return _vertexs.getIterator( desc );
      }

      const_vertex_iterator get( const VertexDescriptor& desc ) const
      {
         return _vertexs.getIterator( desc );
      }

      //
      // TODO GET DESC FROM ITER
      //

   private:
      Vertexs                 _vertexs;
      Uid                     _uidVertexGenerator;
      std::stack<Uid>         _uidVertexRecyling;
      Uid                     _uidEdgeGenerator;
      std::stack<Uid>         _uidEdgeRecyling;
      DataMapperInterfaces    _vertexsObserver;
      DataMapperInterfaces    _edgesObserver;
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
         static size_t n = 0;
         _uid = ++n;
      }

      size_t getUid() const
      {
         return _uid;
      }

      bool operator==( const DataUq& d ) const
      {
         return _v == d._v && _uid == d._uid;
      }

      T        _v;
      size_t   _uid;
   };

   template <class MapperType>
   void testImpl()
   {
      typedef DataUq<int> DataUq1;
      typedef Mapper<DataUq1, MapperType> Mapper1;
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

      mapper1.erase( it1 );
      id = 2;
      for ( Mapper1::iterator it = mapper1.begin(); it != mapper1.end(); ++it, ++id )
      {
         int res = (*it)._v;
         TESTER_ASSERT( res == id );
      }
   }

   void testVector()
   {
      testImpl<MapperVector>();

      typedef DataUq<int> DataUq1;
      typedef Mapper<DataUq1, MapperVector> Mapper1;
      Mapper1  mapper1;

      int dav[] =
      {
         1, 2, 3
      };

      Mapper1 m2( 10, DataUq1( dav[ 0 ] ) );
   }

   void testSet()
   {
      testImpl<MapperSet>();
   }

   template <class VertexType, class EdgeType>
   void testGraphImpl()
   {
      typedef Graph<VertexType, EdgeType>   Graph1;
      Graph1 g;
      Graph1::VertexDescriptor v1 = g.addVertex();
      Graph1::VertexDescriptor v2 = g.addVertex();
      Graph1::VertexDescriptor v3 = g.addVertex();
      Graph1::VertexDescriptor v4 = g.addVertex();
      Graph1::VertexDescriptor v5 = g.addVertex();
      Graph1::VertexDescriptor v6 = g.addVertex();

      Graph1::EdgeDescriptor e1 = g.addEdge( v1, v2 );
      Graph1::EdgeDescriptor e2 = g.addEdge( v1, v5 );
      g.addEdge( v1, v3 );
      g.addEdge( v2, v3 );
      g.addEdge( v2, v4 );
      g.addEdge( v4, v5 );
      g.addEdge( v4, v6 );
      g.addEdge( v4, v1 );
      g.addEdge( v1, v4 );

      for ( Graph1::const_vertex_iterator v = g.begin(); v != g.end(); ++v )
      {
         for ( Graph1::const_edge_iterator e = (*v).begin(); e != (*v).end(); ++e )
         {
            std::cout << "AA" << std::endl;
         }
      }

      Graph1::VertexMapper<int> intmap( g, -1 );
      TESTER_ASSERT( intmap[ v1 ] == -1 );
      std::cout << "DATA=" << intmap[ v1 ] << std::endl;
      intmap[ v3 ] = 2;
      TESTER_ASSERT( intmap[ v3 ] == 2 );

      Graph1* g2 = new Graph1();
      Graph1::VertexMapper<int> intmap2( *g2, -2 );
      Graph1::VertexDescriptor v1b = g2->addVertex();
      Graph1::VertexDescriptor v2b = g2->addVertex();
      Graph1::VertexDescriptor v3b = g2->addVertex();
      delete g2;

      TESTER_ASSERT( intmap2[ v1b ] == -2 );
      TESTER_ASSERT( intmap2[ v2b ] == -2 );
      TESTER_ASSERT( intmap2[ v3b ] == -2 );
      std::cout << "DATA=" << intmap2[ v1b ] << std::endl;

      Graph1::edge_iterator ie1n = g.erase( (*g.begin()).begin() );
      TESTER_ASSERT( (*ie1n).getUid() == 1 );

      Graph1::edge_iterator ie2n = g.erase( (*g.begin()).begin() );
      TESTER_ASSERT( (*ie2n).getUid() == 2 );

      Graph1::EdgeDescriptor e1b = g.addEdge( v1, v2 );
      Graph1::EdgeDescriptor e2b = g.addEdge( v1, v5 );
      Graph1::edge_iterator ie2b = g.get( e2b );
      size_t uid = (*ie2b).getUid();

      // test the const method
      const Graph1& gc = g;
      g.get( e1b );
      gc.get( e1b );
      g.get( v3 );
      gc.get( v3 );

      Graph1::edge_iterator ie1bn = g.erase( e1b );
      TESTER_ASSERT( (*ie1bn).getUid() == uid );
   }

   void testGraphVV()
   {
      testGraphImpl<MapperVector, MapperVector>();
   }

   void testGraphSS()
   {
      testGraphImpl<MapperSet, MapperSet>();
   }

   void testGraphSV()
   {
      testGraphImpl<MapperSet, MapperVector>();
   }

   void testGraphVS()
   {
      testGraphImpl<MapperVector, MapperSet>();
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGraph2);
TESTER_TEST( testVector );
TESTER_TEST( testSet );
TESTER_TEST( testGraphVV );
TESTER_TEST( testGraphSS );
TESTER_TEST( testGraphSV );
TESTER_TEST( testGraphVS );
TESTER_TEST_SUITE_END();
#endif