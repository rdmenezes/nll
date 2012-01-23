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

#ifndef NLL_GRAPH_ADJENCY_LIST_H_
# define NLL_GRAPH_ADJENCY_LIST_H_

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
         return const_cast<DataStorage&>( *d._it );	// Bad: const_cast workaround as the set is always returning a const to preserve order, but we guaranty the ID will not be modified
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
		  friend Mapper;

      public:
         const_iterator( const_iteratorImpl it ) : _it( it )
         {}

         const_iterator( iterator it ) : _it( it._it )
         {}

		 const_iterator( const const_iterator& it ) : _it( it._it )
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
    for some mapper type, some indexing will be done (e.g., <VertexType>) but won't be valid anymore after iterator
    invalidation and must be recalculated.

    The graph defines only the structure, the data are stored externally. Each time the graph is updated,
    changes to the mappers are made to reflect this. It is always growing... Descriptor are never
    invalidated to access data via VertexMapper/EdgeMapper (we are using the compact UID to retrieve
    the data).

    Order of the edges and vertices are not specified and depends on the container implementation.
    */
   template <class VertexStorageTypeT = MapperVector, class EdgeStorageTypeT = MapperVector>
   class GraphAdgencyList
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
         friend GraphAdgencyList;

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
         friend GraphAdgencyList;

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
         friend GraphAdgencyList;

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
         friend GraphAdgencyList;

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
         typedef typename Storage::iterator        iterator;
         typedef typename Storage::const_iterator  const_iterator;

      public:
         DataMapper( const GraphAdgencyList& g, size_t size, const T val = T() ) : _g( &g  ), _storage( size, val ), _default( val )
         {
         }

         iterator begin()
         {
            return _storage.begin();
         }

         const_iterator begin() const
         {
            return _storage.begin();
         }

         iterator end()
         {
            return _storage.end();
         }

         const_iterator end() const
         {
            return _storage.end();
         }

         virtual ~DataMapper()
         {
            if ( _g )
            {
               _g->unregisterObserver( this );
            }
         }

         size_t size() const
         {
            return _storage.size();
         }

         const T& operator[]( size_t uid ) const
         {
            return _storage[ uid ];
         }

         T& operator[]( size_t uid )
         {
            return _storage[ uid ];
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
         const GraphAdgencyList*    _g;
         Storage                    _storage;
         T                          _default;
      };

      /**
       @brief Specialization for the vertex data, complexity o(1) op for accessing o(N) in memory
       */
      template <class T>
      class VertexMapper : public DataMapper<T, std::vector<T> >
      {
         typedef DataMapper<T, std::vector<T> > Base;

      public:
         using Base::operator[];

         VertexMapper( const GraphAdgencyList& g, const T val = T() ) : DataMapper( g, g.size(), val )
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

		 /*
         T& operator[]( const vertex_iterator& it )
         {

            return _storage[ (*it).getUid() ];
         }*/

		 
         T& operator[]( const const_vertex_iterator& it )
         {

            return _storage[ (*it).getUid() ];
         }

		 /*
         const T& operator[]( const vertex_iterator& it ) const
         {

            return _storage[ (*it).getUid() ];
         }*/

		 
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
         using Base::operator[];

         EdgeMapper( const GraphAdgencyList& g, const T val = T() ) : DataMapper( g, g.getNbEdges(), val )
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

		 /*
         T& operator[]( const edge_iterator& it )
         {

            return _storage[ (*it).getUid() ];
         }*/

		 
         T& operator[]( const const_edge_iterator& it )
         {

            return _storage[ (*it).getUid() ];
         }
		 /*
         const T& operator[]( const edge_iterator& it ) const
         {

            return _storage[ (*it).getUid() ];
         }*/

		 
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

      GraphAdgencyList() : _uidVertexGenerator( 0 ), _uidEdgeGenerator( 0 )
      {
      }

      ~GraphAdgencyList()
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
         const Edge& e = *it;
         Uid uid;
         Vertex* v = 0;
         if ( safeMode )
         {
            vertex_iterator vi = _vertexs.find( e.src ); 
            assert( vi != _vertexs.end() );  // not in the list?
            uid = (*vi).getUid();

            v = const_cast<Vertex*>( &(*vi) ); // work around for std::set implementation
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
               Vertex& v = const_cast<Vertex&>( *iv );
               for ( edge_iterator ie = v.begin(); ie != v.end(); ++ie )
               {
                  Uid uid = (*ie).getUid();
                  _uidVertexRecyling.push( uid );
               }
            } else {
               // else individually check all the edges
               Vertex& v = const_cast<Vertex&>( *iv );
               for ( edge_iterator ie = v.begin(); ie != v.end(); )
               {
                  const VertexDescriptor& desc = (*ie).dst;
                  vertex_iterator idst = safeMode ? _vertexs.find( desc ) : _vertexs.getIterator( desc );

                  if ( it == idst )
                  {
                     // recycle the UID
                     Uid uid = (*ie).getUid();
                     _uidVertexRecyling.push( uid );

                     // remove it from the list of edges
                     ie = v._edges.erase( ie );
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
         vertex_iterator ita = _vertexs.getIterator( src );
		   Vertex& itav = const_cast<Vertex&>( *ita );
         return EdgeDescriptor( src, itav._edges.insert( Edge( uid, src, dst ) ) );
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

      size_t getNbEdges() const
      {
         size_t nb = 0;
         for ( const_vertex_iterator it = begin(); it != end(); ++it )
         {
            nb += (*it).size();
         }

         return nb;
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

         Vertex& v = const_cast<Vertex&>( *it );   // std::set work around
         return v._edges.getIterator( desc._desc );
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
         GraphAdgencyList& g = const_cast<GraphAdgencyList&>( *this );
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
}
}

#endif