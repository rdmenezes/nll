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
      Descriptor getDescriptor( const iterator& it ) const

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
         Descriptor( iterator it ) : _it( it ), _uid( (*it).getUid() )
         {}

         Uid getUid() const
         {
            return _uid;
         }

      private:
         Uid      _uid;
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

      Descriptor getDescriptor( const iterator& it ) const
      {
         return Descriptor( it );
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

         bool operator==( const iterator& it ) const
         {
            return !operator!=( it );
         }

      private:
         const iteratorImpl& getIterator() const
         {
            return _it;
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

         bool operator==( const const_iterator& it ) const
         {
            return !operator!=( it );
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
         const const_iteratorImpl& getIterator() const
         {
            return _it;
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
         for ( iteratorImpl itUp = newIt; itUp != _wrappers.end(); ++itUp )
         {
            // update the quick access index
            --itUp->uidPerVector;
         }
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

      Descriptor getDescriptor( const iterator& it ) const
      {
         const Uid uid = it.getIterator()->uid;
         const Uid uidPerVector = it.getIterator()->uidPerVector;
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

    <CAUTION>
    When an iterator is invalidated, the corresponding descriptor is also invalidated, except
    for the methods annotated with <Safe> which will "find" the corresponding element. Indeed,
    for some mapper type, some indexing will be done but won't be valid anymore after iterator
    invalidation and must be recalculated.

    The graph defines only the structure, the data are stored externally. Each time the graph is updated,
    changes to the mappers are made to reflect this. It is always growing... Descriptor are never
    invalidated to access data via VertexMapper/EdgeMapper (we are using the compact UID to retrieve
    the data).

    Order of the edges and vertices are not specified and depends on the container implementation.
    */
   template <class VertexStorageTypeT = MapperVector, class EdgeStorageTypeT = MapperVector>
   class Graph
   {
   public:
      typedef VertexStorageTypeT             VertexStorageType;
      typedef EdgeStorageTypeT               EdgeStorageType;
      class Edge;
      class Vertex;
      typedef Mapper< Vertex, VertexStorageType >           Vertexs;
      typedef typename Vertexs::iterator                    vertex_iterator;
      typedef typename Vertexs::const_iterator              const_vertex_iterator;
      typedef typename Vertexs::Uid                         Uid;
      typedef typename Vertexs::Descriptor                  VertexDescriptor;

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
         //virtual void erase( Uid uid ) = 0;

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

         const VertexDescriptor& getSource() const
         {
            return src;
         }

         const VertexDescriptor& getDestination() const
         {
            return dst;
         }

      private:
         typename Vertexs::Descriptor src;
         typename Vertexs::Descriptor dst;

      private:
         Uid  _uid;
      };
      typedef Mapper< Edge, EdgeStorageType >   Edges;
      typedef typename Edges::iterator          edge_iterator;
      typedef typename Edges::const_iterator    const_edge_iterator;

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

         size_t size() const
         {
            return _edges.size();
         }

      private:
         Uid   _uid;
         Edges _edges;
      };

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
      void registerEdgeObserver( DataMapperInterface* i ) const
      {
         _edgesObserver.push_back( i );
      }

      void registerVertexObserver( DataMapperInterface* i ) const
      {
         _vertexsObserver.push_back( i );
      }

      void unregisterObserver( DataMapperInterface* i ) const
      {
         // it is in one of the set...
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
         DataMapper( const Graph& g, const T val = T() ) : _g( &g  ), _storage( g.size(), val ), _default( val )
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
         const Graph*   _g;
         Storage        _storage;
         T              _default;
      };

      /**
       @brief Specialization for the vertex data, complexity o(1) op for accessing o(N) in memory
       */
      template <class T>
      class VertexMapper : public DataMapper<T, std::vector<T> >
      {
         typedef DataMapper<T, std::vector<T> > Base;

      public:
         VertexMapper( const Graph& g, const T val = T() ) : DataMapper( g, val )
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

         T& operator[]( const vertex_iterator& it )
         {

            return _storage[ (*it).getUid() ];
         }

         T& operator[]( const const_vertex_iterator& it )
         {

            return _storage[ (*it).getUid() ];
         }

         const T& operator[]( const vertex_iterator& it ) const
         {

            return _storage[ (*it).getUid() ];
         }

         const T& operator[]( const const_vertex_iterator& it ) const
         {

            return _storage[ (*it).getUid() ];
         }

         /*
         virtual void erase( Uid uid ) 
         {
            Storage::iterator it = _storage.begin() + uid;
            _storage.erase( it );
         }*/

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
         EdgeMapper( const Graph& g, const T val = T() ) : DataMapper( g, val )
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

         T& operator[]( const edge_iterator& it )
         {

            return _storage[ (*it).getUid() ];
         }

         T& operator[]( const const_edge_iterator& it )
         {

            return _storage[ (*it).getUid() ];
         }

         const T& operator[]( const edge_iterator& it ) const
         {

            return _storage[ (*it).getUid() ];
         }

         const T& operator[]( const const_edge_iterator& it ) const
         {

            return _storage[ (*it).getUid() ];
         }

         /*
         virtual void erase( Uid uid ) 
         {
            Storage::iterator it = _storage.begin() + uid;
            _storage.erase( it );
         }*/

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

      /**
       @param safeMode if true, it means some of the vertex iterators were invalidated and a deep search must be done
              to retrieve iterators from descriptors. E.g. if only edges are removed/added safeMode = false is good
       */
      edge_iterator erase( const edge_iterator& it, bool safeMode = true )
      {
         // get the source vertex
         Edge& e = *it;
         Uid uid;
         Vertex* v = 0;
         if ( safeMode )
         {
            vertex_iterator vi = _vertexs.find( e.src ); 
            assert( vi != _vertexs.end() );  // not in the list?
            uid = (*vi).getUid();

            v = &(*vi);
         } else {
            v = &_vertexs.getData( e.src ); // quick retrieval. safe mode assumed 
            uid = e.getUid();
         }
         _uidEdgeRecyling.push( uid );   // here we recycle the UID to be reused for next <add>
         return v->_edges.erase( it );
      }

      /**
       @param safeMode if true, it means some of the vertex iterators were invalidated and a deep search must be done
              to retrieve iterators from descriptors. E.g. if only edges are removed/added safeMode = false is good
       */
      vertex_iterator erase( const vertex_iterator& it, bool safeMode = true )
      {
         // run through all the edges, remove the edges that are connected to this vertex
         for ( vertex_iterator iv = begin(); iv != end(); ++iv )
         {
            if ( it == iv )
            {
               // recycle all the edges in this vertex
               for ( edge_iterator ie = (*iv).begin(); ie != (*iv).end(); ++ie )
               {
                  Uid uid = (*ie).getUid();
                  _uidVertexRecyling.push( uid );
               }
            } else {
               // else individually check all the edges
               for ( edge_iterator ie = (*iv).begin(); ie != (*iv).end(); )
               {
                  const VertexDescriptor& desc = (*ie).dst;
                  vertex_iterator idst = safeMode ? _vertexs.find( desc ) : _vertexs.getIterator( desc );

                  if ( it == idst )
                  {
                     // recycle the UID
                     Uid uid = (*ie).getUid();
                     _uidVertexRecyling.push( uid );

                     // remove it from the list of edges
                     ie = (*iv)._edges.erase( ie );
                  } else {
                     ++ie;
                  }
               } 
            }
         }

         return _vertexs.erase( it );
      }

      edge_iterator erase( const EdgeDescriptor& desc )
      {
         // from the descriptor we need to know the source so that we can lookup the edge iterator
         edge_iterator it = getIterator( desc );
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
      // These converters assume no iterators were invalidated
      //

      edge_iterator getIterator( const EdgeDescriptor& desc )
      {
         // first get the source vertex
         vertex_iterator it = _vertexs.getIterator( desc._src );
         assert( it != _vertexs.end() );

         return (*it)._edges.getIterator( desc._desc );
      }

      const_edge_iterator getIterator( const EdgeDescriptor& desc ) const
      {
         // first get the source vertex
         const_vertex_iterator it = _vertexs.getIterator( desc._src );
         assert( it != _vertexs.end() );

         return (*it)._edges.getIterator( desc._desc );
      }

      vertex_iterator getIterator( const VertexDescriptor& desc )
      {
         return _vertexs.getIterator( desc );
      }

      const_vertex_iterator getIterator( const VertexDescriptor& desc ) const
      {
         return _vertexs.getIterator( desc );
      }

      //
      // safe methods: we know a descriptor has been invalidated, we need to "find" safely its UID
      // generally much more exepensive than using a quick index for some of the mappers
      //
      vertex_iterator getIteratorSafe( const VertexDescriptor& desc )
      {
         return _vertexs.find( desc );
      }

      const_vertex_iterator getIteratorSafe( const VertexDescriptor& desc ) const
      {
         return _vertexs.find( desc );
      }

      const_edge_iterator getIteratorSafe( const EdgeDescriptor& desc ) const
      {
         // first get the source vertex
         const_vertex_iterator it = getIteratorSafe( desc._src );
         assert( it != _vertexs.end() );

         return (*it)._edges.find( desc._desc );
      }

      edge_iterator getIteratorSafe( const EdgeDescriptor& desc )
      {
         // first get the source vertex
         vertex_iterator it = getIteratorSafe( desc._src );
         assert( it != _vertexs.end() );

         return (*it)._edges.find( desc._desc );
      }

      //
      // other safe conversions (assuming the iterator is valid)
      //

      VertexDescriptor getDescriptor( const vertex_iterator& it ) const
      {
         return _vertexs.getDescriptor( it );
      }

      EdgeDescriptor getDescriptor( const edge_iterator& it ) const
      {
         Graph& g = const_cast<Graph&>( *this );
         vertex_iterator itf = g._vertexs.getIterator( (*it).src );
         assert( itf != g.end() );  // doesn't contain it?
         const Edges& edges = (*itf)._edges; 

         return EdgeDescriptor( getDescriptor( itf ), edges.getDescriptor( it ) );
      }

   private:
      Vertexs                 _vertexs;
      Uid                     _uidVertexGenerator;
      std::stack<Uid>         _uidVertexRecyling;
      Uid                     _uidEdgeGenerator;
      std::stack<Uid>         _uidEdgeRecyling;

      // these are mutables as they are not really part of the graph structure...
      mutable DataMapperInterfaces    _vertexsObserver;
      mutable DataMapperInterfaces    _edgesObserver;
   };

   template <class GraphT>
   class GraphVisitorBfs
   {
   public:
      typedef GraphT                   Graph;
      typedef typename Graph::Vertex   Vertex;
      typedef typename Graph::Edge     Edge;

      typedef typename Graph::vertex_iterator   vertex_iterator;
      typedef typename Graph::edge_iterator     edge_iterator;
      typedef typename Graph::const_vertex_iterator   const_vertex_iterator;
      typedef typename Graph::const_edge_iterator     const_edge_iterator;

      // run before the algorithm is started
      virtual void start( const Graph& ){}

      // run after the algorithm has finished
      virtual void finish( const Graph& ){}

      // called when the vertex has been discovered for the first time
      virtual void discoverVertex( const const_vertex_iterator& , const Graph& ){}

      // called when all the edges have been discovered
      virtual void finishVertex( const const_vertex_iterator& , const Graph& ){}

      // called each time an edge is discovered
      virtual void discoverEdge( const const_edge_iterator& , const Graph& ){}

      virtual void visit( const Graph& g )
      {
         typename GraphT::VertexMapper<char> vertexDiscovered( g );
         std::vector<const_vertex_iterator> its;
         start( g );
         if ( g.size() )
         {
            for ( const_vertex_iterator vertex = g.begin(); vertex != g.end(); ++vertex )
            {

               if ( vertexDiscovered[ vertex ] )
                  continue;   // we already checked this vertex

               
               // queue the first vertex
               its.push_back( vertex );
               const_vertex_iterator& itsrc = *its.rbegin();
               discoverVertex( itsrc, g );
               vertexDiscovered[ itsrc ] = 1;

               // finally continue until all vertexes have been visited
               while ( its.size() )
               {
                  const_vertex_iterator it = *its.rbegin();
                  its.pop_back();
                  
                  for ( const_edge_iterator ite = (*it).begin(); ite != (*it).end(); ++ite )
                  {
                     const_vertex_iterator toVertexIt = g.getIterator( (*ite).getSource() );
                     const bool hasBeenDiscovered = vertexDiscovered[ toVertexIt ] == 1;
                     if ( !hasBeenDiscovered )
                     {
                        
                        discoverEdge( ite, g );
                        its.push_back( toVertexIt );
                        discoverVertex( toVertexIt, g );
                        vertexDiscovered[ toVertexIt ] = 1;
                     }
                  }
                  finishVertex( it, g );  // we have visited all out edges...
               }
            }
         }
         finish( g );
      }
   };
}
}


class TestGraph2
{
   template <class Graph>
   class ConstVisitorBfsPrint : public GraphVisitorBfs<Graph>
   {
   public:
      typedef typename Graph::EdgeMapper<std::string>    EdgeMapper;
      typedef typename Graph::VertexMapper<int>          VertexMapper;
      typedef typename Graph::VertexDescriptor           VertexDescriptor;
      typedef typename Graph::EdgeDescriptor             EdgeDescriptor;


      ConstVisitorBfsPrint( const VertexMapper& v, const EdgeMapper& e ) : _v( v ), _e( e )
      {}

      virtual void start( Graph& )
      {
         std::cout << "---DFS started---" << std::endl;
      }
      
      virtual void discoverVertex( const const_vertex_iterator& vertex, const Graph& )
      {
         std::cout << "Vertex=" << _v[ vertex ] << std::endl;
         discoveryList.push_back( "V" + core::val2str( _v[ vertex ] ) );
      }

      virtual void discoverEdge( const const_edge_iterator& edge, const Graph& )
      {
         std::cout << "Edge=" << _e[ edge ] << std::endl;
         discoveryList.push_back( "E" + _e[ edge ] );
      }

      virtual void finishVertex( const const_vertex_iterator& vertex, const Graph& )
      {
         std::cout << "END Vertex=" << _v[ vertex ] << std::endl;
         discoveryList.push_back( "EV" + core::val2str( _v[ vertex ] ) );
      }

      std::vector<std::string> discoveryList;

   private:
      const VertexMapper&     _v;
      const EdgeMapper&       _e;
   };

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
      {
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
         Graph1::edge_iterator ie2b = g.getIterator( e2b );

         // test the const method
         const Graph1& gc = g;
         g.getIterator( e1b );
         gc.getIterator( e1b );
         g.getIterator( v3 );
         gc.getIterator( v3 );

         Graph1::edge_iterator ie1bn = g.erase( e1b );
      }

      {
         Graph1 g3;
         //const Graph1& g3c = g3;

         Graph1::VertexDescriptor v1 = g3.addVertex();
         Graph1::VertexDescriptor v2 = g3.addVertex();
         Graph1::VertexDescriptor v3 = g3.addVertex();
         Graph1::VertexDescriptor v4 = g3.addVertex();

         Graph1::VertexMapper<int> mv( g3, -2 );
         Graph1::EdgeMapper<int>   me( g3, -1 );

         Graph1::EdgeDescriptor e1 = g3.addEdge( v1, v2 );
         Graph1::EdgeDescriptor e2 = g3.addEdge( v1, v3 );
         Graph1::EdgeDescriptor e3 = g3.addEdge( v1, v4 );
         Graph1::EdgeDescriptor e4 = g3.addEdge( v2, v4 );
         Graph1::EdgeDescriptor e5 = g3.addEdge( v3, v4 );

         mv[ v1 ] = 1;
         mv[ v2 ] = 2;
         mv[ v3 ] = 3;
         mv[ v4 ] = 4;

         me[ e1 ] = 1;
         me[ e2 ] = 2;
         me[ e3 ] = 3;
         me[ e4 ] = 4;
         me[ e5 ] = 5;

         Graph1::vertex_iterator iv1 = g3.begin();
         Graph1::vertex_iterator iv1c = iv1;
         Graph1::edge_iterator ie1 = (*iv1).begin();
         Graph1::edge_iterator ie1c = ie1;
         TESTER_ASSERT( me[ g3.getDescriptor( ie1 ) ] == 1 );
         ++ie1;
         TESTER_ASSERT( me[ g3.getDescriptor( ie1 ) ] == 2 );
         ++ie1;
         TESTER_ASSERT( me[ g3.getDescriptor( ie1 ) ] == 3 );


         TESTER_ASSERT( mv[ g3.getDescriptor( iv1 ) ] == 1 );

         ++iv1;
         TESTER_ASSERT( mv[ g3.getDescriptor( iv1 ) ] == 2 );

         ++iv1;
         TESTER_ASSERT( mv[ g3.getDescriptor( iv1 ) ] == 3 );

         ++iv1;
         TESTER_ASSERT( mv[ g3.getDescriptor( iv1 ) ] == 4 );

         ie1 = g3.erase( ie1c );
         TESTER_ASSERT( me[ g3.getDescriptor( ie1 ) ] == 2 );
         ++ie1;
         TESTER_ASSERT( me[ g3.getDescriptor( ie1 ) ] == 3 );

         ie1 = (*iv1c).begin();
         TESTER_ASSERT( me[ g3.getDescriptor( ie1 ) ] == 2 );
         ++ie1;
         TESTER_ASSERT( me[ g3.getDescriptor( ie1 ) ] == 3 );
      }

      {
         Graph1 g3;

         Graph1::VertexDescriptor v1 = g3.addVertex();
         Graph1::VertexDescriptor v2 = g3.addVertex();
         Graph1::VertexDescriptor v3 = g3.addVertex();
         Graph1::VertexDescriptor v4 = g3.addVertex();

         Graph1::VertexMapper<int> mv( g3, -2 );
         Graph1::EdgeMapper<int>   me( g3, -1 );

         Graph1::EdgeDescriptor e1 = g3.addEdge( v1, v2 );
         Graph1::EdgeDescriptor e2 = g3.addEdge( v1, v3 );
         Graph1::EdgeDescriptor e3 = g3.addEdge( v1, v4 );
         Graph1::EdgeDescriptor e4 = g3.addEdge( v2, v4 );
         Graph1::EdgeDescriptor e5 = g3.addEdge( v3, v4 );

         mv[ v1 ] = 1;
         mv[ v2 ] = 2;
         mv[ v3 ] = 3;
         mv[ v4 ] = 4;

         me[ e1 ] = 1;
         me[ e2 ] = 2;
         me[ e3 ] = 3;
         me[ e4 ] = 4;
         me[ e5 ] = 5;

         Graph1::vertex_iterator iv1 = g3.getIteratorSafe( v1 );
         iv1 = g3.erase( iv1 );
         Graph1::VertexDescriptor iv1d = g3.getDescriptor( iv1 );
         TESTER_ASSERT( mv[ iv1d ] == 2 );
         TESTER_ASSERT( g3.size() == 3 );

         // erase v4, check edges of v2 v3
         iv1 = g3.getIteratorSafe( v4 );
         iv1 = g3.erase( iv1 );

         Graph1::vertex_iterator iv2 = g3.getIteratorSafe( v2 );
         Graph1::vertex_iterator iv3 = g3.getIteratorSafe( v3 );
         TESTER_ASSERT( (*iv2).size() == 0 );
         TESTER_ASSERT( (*iv3).size() == 0 );

         // check full deallocation
         while ( g3.size() )
         {
            g3.erase( g3.begin() );
         }
      }
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

   void testBfs()
   {
      typedef Graph<MapperVector, MapperVector>   Graph1;

      nll::core::Timer time;
      Graph1 g;

      Graph1::VertexMapper<int>         mv( g );
      Graph1::EdgeMapper<std::string>   me( g );

      Graph1::VertexDescriptor n1 = g.addVertex();
      Graph1::VertexDescriptor n2 = g.addVertex();
      Graph1::VertexDescriptor n3 = g.addVertex();
      Graph1::VertexDescriptor n4 = g.addVertex();
      Graph1::VertexDescriptor n5 = g.addVertex();
      Graph1::VertexDescriptor n6 = g.addVertex();

      mv[ n1 ] = 1;
      mv[ n2 ] = 2;
      mv[ n3 ] = 3;
      mv[ n4 ] = 4;
      mv[ n5 ] = 5;
      mv[ n6 ] = 6;

      Graph1::EdgeDescriptor e1 = g.addEdge( n1, n2 );
      Graph1::EdgeDescriptor e2 = g.addEdge( n2, n3 );
      Graph1::EdgeDescriptor e3 = g.addEdge( n2, n4 );
      Graph1::EdgeDescriptor e4 = g.addEdge( n4, n5 );
      Graph1::EdgeDescriptor e5 = g.addEdge( n4, n6 );
      Graph1::EdgeDescriptor e6 = g.addEdge( n4, n1 );
      Graph1::EdgeDescriptor e7 = g.addEdge( n1, n4 );

      me[ e1 ] = "a";
      me[ e2 ] = "b";
      me[ e3 ] = "c";
      me[ e4 ] = "d";
      me[ e5 ] = "e";
      me[ e6 ] = "f";
      me[ e7 ] = "g";

      ConstVisitorBfsPrint<Graph1> visitor( mv, me );
      visitor.visit( g );

      TESTER_ASSERT( visitor.discoveryList.size() == 17 );
      TESTER_ASSERT( visitor.discoveryList[ 0 ] == "V1" );
      TESTER_ASSERT( visitor.discoveryList[ 1 ] == "Ea" );
      TESTER_ASSERT( visitor.discoveryList[ 2 ] == "V2" );
      TESTER_ASSERT( visitor.discoveryList[ 3 ] == "Eg" );
      TESTER_ASSERT( visitor.discoveryList[ 4 ] == "V4" );
      TESTER_ASSERT( visitor.discoveryList[ 5 ] == "EV1" );
      TESTER_ASSERT( visitor.discoveryList[ 6 ] == "Ed" );
      TESTER_ASSERT( visitor.discoveryList[ 7 ] == "V5" );
      TESTER_ASSERT( visitor.discoveryList[ 8 ] == "Ee" );
      TESTER_ASSERT( visitor.discoveryList[ 9 ] == "V6" );
      TESTER_ASSERT( visitor.discoveryList[ 10 ] == "EV4" );
      TESTER_ASSERT( visitor.discoveryList[ 11 ] == "EV6" );
      TESTER_ASSERT( visitor.discoveryList[ 12 ] == "EV5" );
      TESTER_ASSERT( visitor.discoveryList[ 13 ] == "Eb" );
      TESTER_ASSERT( visitor.discoveryList[ 14 ] == "V3" );
      TESTER_ASSERT( visitor.discoveryList[ 15 ] == "EV2" );
      TESTER_ASSERT( visitor.discoveryList[ 16 ] == "EV3" );
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
TESTER_TEST( testBfs );
TESTER_TEST_SUITE_END();
#endif