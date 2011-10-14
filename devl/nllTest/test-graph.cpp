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

   /**
    @brief Graph with an adjacency list representation

    TODO: remove VertexDescriptor and replace it by an iterator instead
          replace ConstEdgeIterator by proper iterator on Edge and Vertex and not on the descriptors
          policy to give a unique ID to Edge/Vertex

    */
   template <class VertexDetailT = Empty, class EdgeDetailT = Empty, class DirectedT = Directed>
   struct GraphImplAdjencyList
   {
      typedef DirectedT               Direction;
      typedef VertexDetailT           VertexDetail;
      typedef EdgeDetailT             EdgeDetail;

      struct VertexDescriptor
      {
         friend GraphImplAdjencyList;

      private:
         VertexDescriptor( ui32 suid, ui32 duid ) : detailUid( duid ), storageUid( suid )
         {}

         ui32           detailUid;
         ui32           storageUid;
      };

      struct EdgeDescriptor
      {
         friend GraphImplAdjencyList;

      private:
         EdgeDescriptor( ui32 sid, ui32 did, const VertexDescriptor& s, const VertexDescriptor& d ) : storageUid( sid ), detailUid( did ), src( s ), dst( d )
         {
         }
         ui32           detailUid;
         ui32           storageUid;
         VertexDescriptor src;
         VertexDescriptor dst;
      };

      class ConstVertexIterator;
      class VertexIterator;

      struct Edge
      {
         friend GraphImplAdjencyList;

      private:
         Edge( GraphImplAdjencyList& g, ui32 duid, ui32 toUid, ui32 fromUid ) : graph( &g ), detailUid( duid ), toVertexUid( toUid ), fromVertexUid( fromUid )
         {}

      public:
         ConstVertexIterator getToVertexIterator() const;
         ConstVertexIterator getFromVertexIterator() const;
         VertexIterator getToVertexIterator();
         VertexIterator getFromVertexIterator();

      private:
         GraphImplAdjencyList* graph;
         ui32           detailUid;
         ui32           toVertexUid;
         ui32           fromVertexUid;
      };
      typedef std::vector<Edge>     Edges;

      struct Vertex
      {
         friend GraphImplAdjencyList;

      private:
         Vertex( GraphImplAdjencyList& g, ui32 duid ) : graph( &g ), detailUid( duid )
         {}

      public:
         typedef typename Edges::iterator          EdgeIterator;
         typedef typename Edges::const_iterator    ConstEdgeIterator;

         EdgeIterator begin()
         {
            return edges.begin();
         }

         EdgeIterator end()
         {
            return edges.end();
         }

         ConstEdgeIterator begin() const
         {
            return edges.begin();
         }

         ConstEdgeIterator end() const
         {
            return edges.end();
         }

         size_t getNbEdges() const
         {
            return edges.size();
         }

      private:
         GraphImplAdjencyList* graph;
         ui32     detailUid;
         Edges    edges;
      };
      typedef std::vector<Vertex>     Vertexs;

      typedef std::vector<VertexDescriptor>   VertexDescriptors;
      typedef std::vector<EdgeDescriptor>     EdgeDescriptors;
      typedef std::vector<VertexDetail>       VertexDetails;
      typedef std::vector<EdgeDetail>         EdgeDetails;

      class ConstVertexIterator
      {
      public:
         ConstVertexIterator( const GraphImplAdjencyList& graph, ui32 uid ) : _graph( &const_cast<GraphImplAdjencyList&>( graph ) ), _it( _graph->_vertexs.begin() + uid )
         {}

         ConstVertexIterator& operator++()
         {
            ++_it;
            return *this;
         }

         ConstVertexIterator operator++(int)
         {
            ConstVertexIterator it( *this );
            ++_it;
            return it;
         }

         ConstVertexIterator operator+( size_t d ) const
         {
            ConstVertexIterator it( * this );
            it._it += d;
            return it;
         }

         size_t operator-( const ConstVertexIterator& it ) const
         {
            return _it - it._it;
         }

         bool operator!=( const ConstVertexIterator& it ) const
         {
            return _it != it._it;
         }

         const VertexDetail& operator*() const
         {
            return _graph->_vertexDetails[ _it->detailUid ];
         }

         const Vertex& getVertex() const
         {
            return *_it;
         }

      protected:
         GraphImplAdjencyList*         _graph;
         typename Vertexs::iterator    _it;
      };

      class VertexIterator : public ConstVertexIterator
      {
      public:
         VertexIterator( GraphImplAdjencyList& graph, ui32 uid ) : ConstVertexIterator( graph, uid )
         {}

         bool operator!=( const VertexIterator& it ) const
         {
            return _it != it._it;
         }

         VertexDetail& operator*()
         {
            return _graph->_vertexDetails[ _it->detailUid ];
         }

         Vertex& getVertex()
         {
            return *_it;
         }

         size_t operator-( const VertexIterator& it ) const
         {
            return _it - it._it;
         }

         VertexIterator operator+( size_t d ) const
         {
            VertexIterator it( * this );
            it._it += d;
            return it;
         }
      };

      class ConstEdgeIterator
      {
      public:
         ConstEdgeIterator( const GraphImplAdjencyList& graph, ui32 VertexUid, ui32 EdgeUid ) : _graph( &const_cast<GraphImplAdjencyList&>( graph ) ), _it( _graph->_vertexs[ VertexUid ].edges.begin() + EdgeUid )
         {}

         ConstEdgeIterator& operator++()
         {
            ++_it;
            return *this;
         }

         ConstEdgeIterator operator++(int)
         {
            ConstEdgeIterator it( *this );
            ++_it;
            return it;
         }

         bool operator!=( const ConstEdgeIterator& it ) const
         {
            return _it != it._it;
         }

         const EdgeDetail& operator*() const
         {
            return _graph->_edgeDetails[ _it->detailUid ];
         }

         const Edge& getEdge() const
         {
            return *_it;
         }

      protected:
         GraphImplAdjencyList*         _graph;
         typename Edges::iterator      _it;
      };

      class EdgeIterator : public ConstEdgeIterator
      {
      public:
         EdgeIterator( GraphImplAdjencyList& graph, ui32 VertexUid, ui32 EdgeUid ) : ConstEdgeIterator( graph, VertexUid, EdgeUid )
         {}

         EdgeDetail& operator*()
         {
            return _graph->_edgeDetails[ _it->detailUid ];
         }

         Edge& getEdge()
         {
            return *_it;
         }
      };

   public:
      GraphImplAdjencyList()
      {}

      
      VertexDescriptor addVertex( const VertexDetail& detail )
      {
         return addVertexImpl( Direction(), detail );
      }
      
      EdgeDescriptor addEdge( const VertexDescriptor& n1, const VertexDescriptor& n2, const EdgeDetail& detail )
      {
         return addEdgeImpl( Direction(), n1, n2, detail );
      }

      EdgeDescriptor getEdge( const VertexDescriptor& n1, const VertexDescriptor& n2, bool& found )
      {
         return getEdgeImpl( Direction(), n1, n2, found );
      }

      VertexIterator beginVertexs()
      {
         return VertexIterator( *this, 0 );
      }

      VertexIterator endVertexs()
      {
         return VertexIterator( *this, _vertexs.size() );
      }

      ConstVertexIterator beginVertexs() const
      {
         return ConstVertexIterator( *this, 0 );
      }

      ConstVertexIterator endVertexs() const
      {
         return ConstVertexIterator( *this, _vertexs.size() );
      }

      EdgeIterator beginOutEdges( const VertexDescriptor& desc )
      {
         return EdgeIterator( *this, desc.storageUid, 0 );
      }

      ConstEdgeIterator beginOutEdges( const VertexDescriptor& desc ) const
      {
         return ConstEdgeIterator( *this, desc.storageUid, 0 );
      }

      EdgeIterator endOutEdges( const VertexDescriptor& desc )
      {
         return EdgeIterator( *this, desc.storageUid, _vertexs[ desc.storageUid ].edges.size() );
      }

      ConstEdgeIterator endOutEdges( const VertexDescriptor& desc ) const
      {
         return ConstEdgeIterator( *this, desc.storageUid, _vertexs[ desc.storageUid ].edges.size() );
      }

      EdgeDetail& getEdgeDetail( const EdgeDescriptor& desc ) const
      {
         return _edgeDetails[ desc.detailUid ];
      }

      EdgeDetail& getEdgeDetail( const EdgeDescriptor& desc )
      {
         return _edgeDetails[ desc.detailUid ];
      }

      EdgeDetail& getEdgeDetail( const Edge& desc ) const
      {
         return _edgeDetails[ desc.detailUid ];
      }

      EdgeDetail& getEdgeDetail( const Edge& desc )
      {
         return _edgeDetails[ desc.detailUid ];
      }

      VertexDetail& getVertexDetail( const VertexDescriptor& desc )
      {
         return _vertexDetails[ desc.detailUid ];
      }

      
      VertexDetail& getVertexDetail( const VertexDescriptor& desc ) const
      {
         return _vertexDetails[ desc.detailUid ];
      }

      VertexDetail& getVertexDetail( const Vertex& desc )
      {
         return _vertexDetails[ desc.detailUid ];
      }

      
      VertexDetail& getVertexDetail( const Vertex& desc ) const
      {
         return _vertexDetails[ desc.detailUid ];
      }

   private:
      VertexDescriptor createVertex( ui32 detailUid )
      {
         VertexDescriptor desc( _vertexs.size(), detailUid );
         _vertexs.push_back( Vertex( *this, detailUid ) );
         return desc;
      }

      VertexDetail& createVertexDetail( const VertexDetail& detail, ui32& uid )
      {
         if ( _recycleVertexDetails.size() )
         {
            const ui32 uidRecycled = *_recycleVertexDetails.rbegin();
            VertexDetail& d = _vertexDetails[ uidRecycled ];
            _recycleVertexDetails.pop_back();

            uid = uidRecycled;
            d = detail;
            return d;
         } else {
            uid = _vertexDetails.size();
            _vertexDetails.push_back( detail );
            return *_vertexDetails.rbegin();
         }
      }

      EdgeDetail& createEdgeDetail( const EdgeDetail& detail, ui32& uid )
      {
         if ( _recycleEdgeDetails.size() )
         {
            const ui32 uidRecycled = *_recycleEdgeDetails.rbegin();
            EdgeDetail& d = _edgeDetails[ uidRecycled ];
            _recycleEdgeDetails.pop_back();

            uid = uidRecycled;
            d = detail;
            return d;
         } else {
            uid = _edgeDetails.size();
            _edgeDetails.push_back( detail );
            return *_edgeDetails.rbegin();
         }
      }

      
      template <class IsDirected>
      VertexDescriptor addVertexImpl( IsDirected fswitch, const VertexDetail& detail );

      
      template <class IsDirected>
      EdgeDescriptor addEdgeImpl( IsDirected fswitch, const VertexDescriptor& n1, const VertexDescriptor& n2, const EdgeDetail& detail );


      template <class IsDirected>
      EdgeDescriptor getEdgeImpl( IsDirected fswitch, const VertexDescriptor& n1, const VertexDescriptor& n2, bool& found );

      template <>
      VertexDescriptor addVertexImpl( Directed, const VertexDetail& detail )
      {
         ui32 duid;
         createVertexDetail( detail, duid );
         return createVertex( duid );
      }  

      template <>
      EdgeDescriptor addEdgeImpl( Directed, const VertexDescriptor& n1, const VertexDescriptor& n2, const EdgeDetail& detail )
      {
         ui32 uid;
         createEdgeDetail( detail, uid );
         Vertex& Vertex = _vertexs[ n1.storageUid ];
         EdgeDescriptor newEdge( Vertex.edges.size(), uid, n1, n2 );

         #ifdef NLL_GRAPH_SECURE_CHECKS
         for ( Edges::iterator it = Vertex.edges.begin(); it != Vertex.edges.end(); ++it )
         {
            ensure( it->toVertexUid != n2.storageUid, "the Edge has been added twice" );
         }
         #endif
         Vertex.edges.push_back( Edge( *this, uid, n2.storageUid, n1.storageUid ) );
         return newEdge;
      }

      template <>
      EdgeDescriptor getEdgeImpl( Directed, const VertexDescriptor& n1, const VertexDescriptor& n2, bool& found )
      {
         Vertex& Vertex = _vertexs[ n1.storageUid ];
         Edges& edges = Vertex.edges;
         Edges::iterator it = std::find_if( edges.begin(), edges.end(), PredicateAreDirectedVertexUidEqual( n2.storageUid ) );
         if ( it != edges.end() )
         {
            found = true;
            return EdgeDescriptor( it - edges.begin(), it->detailUid, n1, n2 );
         } else {
            found = false;
            return EdgeDescriptor( (ui32)-1, (ui32)-1, n1, n2 );
         }
      }

      struct PredicateAreDirectedVertexUidEqual
      {
         PredicateAreDirectedVertexUidEqual( ui32 toVertexUid ) : _toVertexUid( toVertexUid )
         {}

         bool operator()( const Edge& n ) const
         {
            return _toVertexUid == n.toVertexUid;
         }

      private:
         ui32   _toVertexUid;
      };


   private:
      Vertexs              _vertexs;
      mutable VertexDetails        _vertexDetails;
      mutable EdgeDetails          _edgeDetails;
      std::vector<ui32>    _recycleVertexDetails;
      std::vector<ui32>    _recycleEdgeDetails;
   };
   
   template <class VertexDetailT, class EdgeDetailT, class DirectedT>
   typename GraphImplAdjencyList<VertexDetailT, EdgeDetailT, DirectedT>::ConstVertexIterator GraphImplAdjencyList<VertexDetailT, EdgeDetailT, DirectedT>::Edge::getToVertexIterator() const
   {
      return graph->beginVertexs() + toVertexUid;
   }

   template <class VertexDetailT, class EdgeDetailT, class DirectedT>
   typename GraphImplAdjencyList<VertexDetailT, EdgeDetailT, DirectedT>::ConstVertexIterator GraphImplAdjencyList<VertexDetailT, EdgeDetailT, DirectedT>::Edge::getFromVertexIterator() const
   {
      return graph->beginVertexs() + fromVertexUid;
   }

   template <class VertexDetailT, class EdgeDetailT, class DirectedT>
   typename GraphImplAdjencyList<VertexDetailT, EdgeDetailT, DirectedT>::VertexIterator GraphImplAdjencyList<VertexDetailT, EdgeDetailT, DirectedT>::Edge::getToVertexIterator()
   {
      return graph->beginVertexs() + toVertexUid;
   }

   template <class VertexDetailT, class EdgeDetailT, class DirectedT>
   typename GraphImplAdjencyList<VertexDetailT, EdgeDetailT, DirectedT>::VertexIterator GraphImplAdjencyList<VertexDetailT, EdgeDetailT, DirectedT>::Edge::getFromVertexIterator()
   {
      return graph->beginVertexs() + fromVertexUid;
   }

   template <class Impl = GraphImpl<> >
   class Graph
   {
   public:
      typedef typename Impl::VertexDetail       VertexDetail;
      typedef typename Impl::EdgeDetail         EdgeDetail;
      typedef typename Impl::VertexDescriptor   VertexDescriptor;
      typedef typename Impl::EdgeDescriptor     EdgeDescriptor;
      typedef typename Impl::Vertex             Vertex;
      typedef typename Impl::Edge               Edge;
      typedef Impl                              GraphImpl;
      typedef typename Impl::Direction          Direction;

      typedef typename Impl::VertexIterator        VertexIterator;
      typedef typename Impl::ConstVertexIterator   ConstVertexIterator;
      typedef typename Impl::EdgeIterator          EdgeIterator;
      typedef typename Impl::ConstEdgeIterator     ConstEdgeIterator;

   public:
      class VisitorBfs
      {
      public:
         typedef typename Impl::Vertex             Vertex;
         typedef typename Impl::Edge               Edge;
         typedef Graph<Impl>                       Graph;

         // run before the algorithm is started
         virtual void start( const Graph& ){}

         // run after the algorithm has finished
         virtual void finish( const Graph& ){}

         // called when the vertex has been discovered for the first time
         virtual void discoverVertex( const Vertex& , const Graph& ){}

         // called when all the edges have been discovered
         virtual void finishVertex( const Vertex& , const Graph& ){}

         // called each time an edge is discovered
         virtual void discoverEdge( const Edge& , const Graph& ){}

         virtual void visit( const Graph& g )
         {
            //std::vector<char> visited( g.getNbVertexs() );
            std::vector<char> vertexDiscovered( g.getNbVertexs() );
            std::list<ConstVertexIterator> its;

            start( g );
            if ( vertexDiscovered.size() )
            {
               for ( ConstVertexIterator vertex = g.beginVertexs(); vertex != g.endVertexs(); ++vertex )
               {
                  size_t vertexId = vertex - g.beginVertexs();
                  if ( vertexDiscovered[ vertexId ] == 1 )
                     continue;   // we already checked this vertex

                  // queue the first vertex
                  its.push_back( vertex );
                  discoverVertex( its.rbegin()->getVertex(), g );
                  vertexDiscovered[ vertexId ] = 1;

                  // finally continue until all vertexes have been visited
                  while ( its.size() )
                  {
                     ConstVertexIterator it = *its.rbegin();
                     its.pop_back();
                     for ( Vertex::ConstEdgeIterator ite = it.getVertex().begin(); ite != it.getVertex().end(); ++ite )
                     {
                        ConstVertexIterator toVertexIt = ite->getToVertexIterator();
                        const size_t uid = toVertexIt - g.beginVertexs();
                        const bool hasBeenDiscovered = vertexDiscovered[ uid ] == 1;
                        if ( !hasBeenDiscovered )
                        {
                           discoverEdge( *ite, g );
                           its.push_back( toVertexIt );
                           discoverVertex( toVertexIt.getVertex(), g );
                           vertexDiscovered[ uid ] = 1;
                        }
                     }
                     finishVertex( it.getVertex(), g );  // we have visited all out edges...
                  }
               }
            }
            finish( g );
         }
      };

      class VisitorDfs
      {
      public:
         typedef typename Impl::Vertex             Vertex;
         typedef typename Impl::Edge               Edge;
         typedef Graph<Impl>                       Graph;

         // run before the algorithm is started
         virtual void start( const Graph& ){}

         // run after the algorithm has finished
         virtual void finish( const Graph& ){}

         // called when the vertex has been discovered for the first time
         virtual void discoverVertex( const Vertex& , const Graph& ){}

         // called when all the edges have been discovered
         virtual void finishVertex( const Vertex& , const Graph& ){}

         // called each time an edge is discovered
         virtual void discoverEdge( const Edge& , const Graph& ){}

         virtual void visit( const Graph& g )
         {
            //std::vector<char> visited( g.getNbVertexs() );
            std::vector<char> vertexDiscovered( g.getNbVertexs() );
            std::list<ConstVertexIterator> its;
            std::list< std::pair<Vertex::ConstEdgeIterator, Vertex::ConstEdgeIterator> > eits;

            start( g );
            if ( vertexDiscovered.size() )
            {
               for ( ConstVertexIterator vertex = g.beginVertexs(); vertex != g.endVertexs(); ++vertex )
               {
                  size_t vertexId = vertex - g.beginVertexs();
                  if ( vertexDiscovered[ vertexId ] == 1 )
                     continue;   // we already checked this vertex

                  eits.push_back( std::make_pair( vertex.getVertex().begin(), vertex.getVertex().end() ) );
                  its.push_back( vertex );
                  discoverVertex( vertex.getVertex(), g );
                  vertexDiscovered[ vertexId ] = 1;

                  // finally continue until all vertexes have been visited
                  while ( eits.size() )
                  {
                     Vertex::ConstEdgeIterator& begin = eits.rbegin()->first;
                     Vertex::ConstEdgeIterator& end = eits.rbegin()->second;
                     if ( begin == end )
                     {
                        finishVertex( (*its.rbegin()).getVertex(), g );
                        eits.pop_back();
                        its.pop_back();
                     } else {
                        // explore another vertex
                        ConstVertexIterator itTo = begin->getToVertexIterator();
                        const size_t vertexIdTo = itTo - g.beginVertexs();
                        discoverEdge( *begin, g );
                        if ( !vertexDiscovered[ vertexIdTo ] )
                        {
                           discoverVertex( itTo.getVertex(), g );
                           vertexDiscovered[ vertexIdTo ] = 1;
                           if ( itTo.getVertex().getNbEdges() )
                           {
                              eits.push_back( std::make_pair( itTo.getVertex().begin(), itTo.getVertex().end() ) );
                              its.push_back( itTo );
                           } else {
                              finishVertex( itTo.getVertex(), g );
                           }
                        }
                        ++begin;
                     }
                  }
               }
            }
            finish( g );
         }
      };

   public:
      Graph( const Impl impl = Impl() ) : _impl( impl )
      {}

      size_t getNbVertexs() const
      {
         return endVertexs() - beginVertexs();
      }

      VertexDescriptor addVertex( const VertexDetail details = VertexDetail() )
      {
         return _impl.addVertex( details );
      }

      EdgeDescriptor addEdge( const VertexDescriptor& n1, const VertexDescriptor& n2, const EdgeDetail d = EdgeDetail() )
      {
         return _impl.addEdge( n1, n2, d );
      }

      EdgeDescriptor getEdge( const VertexDescriptor& n1, const VertexDescriptor& n2, bool& found )
      {
         return _impl.getEdge( n1, n2, found );
      }

      VertexIterator beginVertexs()
      {
         return _impl.beginVertexs();
      }

      VertexIterator endVertexs()
      {
         return _impl.endVertexs();
      }

      ConstVertexIterator beginVertexs() const
      {
         return _impl.beginVertexs();
      }

      ConstVertexIterator endVertexs() const
      {
         return _impl.endVertexs();
      }

      EdgeIterator beginOutEdges( const VertexDescriptor& desc )
      {
         return _impl.beginOutEdges( desc );
      }

      EdgeIterator beginOutEdges( const VertexDescriptor& desc ) const
      {
         return _impl.beginOutEdges( desc );
      }

      EdgeIterator endOutEdges( const VertexDescriptor& desc )
      {
         return _impl.endOutEdges( desc );
      }

      EdgeIterator endOutEdges( const VertexDescriptor& desc ) const
      {
         return _impl.endOutEdges( desc );
      }

      EdgeDetail& operator[]( const EdgeDescriptor& desc ) const
      {
         return _impl.getEdgeDetail( desc );
      }

      EdgeDetail& operator[]( const EdgeDescriptor& desc )
      {
         return _impl.getEdgeDetail( desc );
      }

      EdgeDetail& operator[]( const Edge& desc ) const
      {
         return _impl.getEdgeDetail( desc );
      }

      EdgeDetail& operator[]( const Edge& desc )
      {
         return _impl.getEdgeDetail( desc );
      }

      VertexDetail& operator[]( const VertexDescriptor& desc ) const
      {
         return _impl.getVertexDetail( desc );
      }

      VertexDetail& operator[]( const VertexDescriptor& desc )
      {
         return _impl.getVertexDetail( desc );
      }
      
      VertexDetail& operator[]( const Vertex& desc ) const
      {
         return _impl.getVertexDetail( desc );
      }

      VertexDetail& operator[]( const Vertex& desc )
      {
         return _impl.getVertexDetail( desc );
      }

   private:
      GraphImpl   _impl;
   };

   namespace impl
   {
      template <class GraphImpl>
      class _VisitorFindRoot : public GraphImpl::VisitorBfs
      {
         typedef typename GraphImpl::VisitorBfs  Base;

      public:
         typedef typename Base::Graph                 Graph;
         typedef typename Base::Edge                  Edge;
         typedef typename Base::Vertex                Vertex;

         typedef typename GraphImpl::ConstVertexIterator     ConstVertexIterator;
         typedef typename GraphImpl::VertexIterator          VertexIterator;
         typedef typename GraphImpl::VertexDescriptor        VertexDescriptor;

         virtual void start( const Graph& g )
         {
            enum {VAL = Equal<GraphImpl::Direction, Directed>::value };
            STATIC_ASSERT( VAL ); // the graph must be directed!
            _counts = std::vector<ui32>( g.getNbVertexs() );
         }

         virtual void discoverEdge( const typename Base::Edge& edge, const Graph& g )
         {
            size_t uid = edge.getToVertexIterator() - g.beginVertexs();
            ++_counts[ uid ];
         }

         virtual void finish( const Graph& g )
         {
            _roots.clear();

            typedef std::vector< std::pair< ui32, ui32 > >  Pairs;
            Pairs pairs;
            for ( ConstVertexIterator it = g.beginVertexs(); it != g.endVertexs(); ++it )
            {
               ui32 index = (ui32)(it - g.beginVertexs());
               pairs.push_back( std::make_pair( _counts[ index ], index ) );
            }

            std::sort( pairs.begin(), pairs.end() );
            for ( Pairs::const_iterator it = pairs.begin(); it != pairs.end(); ++it )
            {
               if ( it->first != 0 )
                  break;
               _roots.push_back( g.beginVertexs() + it->second );
            }

            _counts.clear();
         }

         const std::vector<ConstVertexIterator>& getRoots() const
         {
            return _roots;
         }

      private:
         std::vector<ui32>                _counts;
         std::vector<ConstVertexIterator> _roots;
      };
   }

   template <class GraphImpl>
   std::vector<typename Graph<GraphImpl>::ConstVertexIterator> findRoots( const Graph<GraphImpl>& graph )
   {
      impl::_VisitorFindRoot< Graph<GraphImpl> > visitor;
      visitor.visit( graph );
      return visitor.getRoots();
   }
}
}

class TestGraph
{
public:
   typedef core::Graph< GraphImplAdjencyList<int, std::string, Directed> > Graph1;

   class ConstVisitorBfsPrint : public Graph1::VisitorBfs
   {
   public:
      virtual void start( const Graph& )
      {
         std::cout << "---DFS started---" << std::endl;
      }
      virtual void discoverVertex( const Vertex& vertex, const Graph& g )
      {
         std::cout << "Vertex=" << g[ vertex ] << std::endl;
         discoveryList.push_back( "V" + core::val2str( g[ vertex ] ) );
      }

      virtual void discoverEdge( const Edge& edge, const Graph& g )
      {
         std::cout << "Edge=" << g[ edge ] << std::endl;
         discoveryList.push_back( "E" + g[ edge ] );
      }

      virtual void finishVertex( const Vertex& vertex, const Graph& g )
      {
         std::cout << "END Vertex=" << g[ vertex ] << std::endl;
         discoveryList.push_back( "EV" + core::val2str( g[ vertex ] ) );
      }

      std::vector<std::string> discoveryList;
   };

   class ConstVisitorDfsPrint : public Graph1::VisitorDfs
   {
   public:
      virtual void start( const Graph& )
      {
         std::cout << "---DFS started---" << std::endl;
      }
      virtual void discoverVertex( const Vertex& vertex, const Graph& g )
      {
         std::cout << "Vertex=" << g[ vertex ] << std::endl;
         discoveryList.push_back( "V" + core::val2str( g[ vertex ] ) );
      }

      virtual void discoverEdge( const Edge& edge, const Graph& g )
      {
         std::cout << "Edge=" << g[ edge ] << std::endl;
         discoveryList.push_back( "E" + g[ edge ] );
      }

      virtual void finishVertex( const Vertex& vertex, const Graph& g )
      {
         std::cout << "END Vertex=" << g[ vertex ] << std::endl;
         discoveryList.push_back( "EV" + core::val2str( g[ vertex ] ) );
      }

      std::vector<std::string> discoveryList;
   };

   class VisitorBfsInc : public Graph1::VisitorBfs
   {
   public:
      virtual void discoverVertex( const Vertex& vertex, const Graph& g )
      {
         ++g[ vertex ];
      }

      virtual void discoverEdge( const Edge&, const Graph& )
      {
      }

      std::vector<ui32> discoveryList;
   };

   void test1()
   {
      nll::core::Timer time;
      Graph1 g;

      Graph1::VertexDescriptor n1 = g.addVertex( 10 );
      Graph1::VertexDescriptor n2 = g.addVertex( 11 );
      Graph1::VertexDescriptor n3 = g.addVertex( 12 );
      g.addVertex( 13 );
      g.addVertex( 14 );

      Graph1::EdgeDescriptor a2 = g.addEdge( n1, n2, "test" );
      Graph1::EdgeDescriptor a3 = g.addEdge( n1, n3, "test2" );

      //TESTER_ASSERT( *a2.getToVertexIterator() == n2 );

      bool found;
      Graph1::EdgeDescriptor a2a = g.getEdge( n1, n2, found );


      for ( Graph1::VertexIterator it = g.beginVertexs(); it != g.endVertexs(); ++it )
      {
         std::cout << "V=" << *it << std::endl;
      }

      for ( Graph1::ConstVertexIterator it = g.beginVertexs(); it != g.endVertexs(); ++it )
      {
         std::cout << "V=" << *it << std::endl;
      }

      for ( Graph1::EdgeIterator it = g.beginOutEdges( n1 ); it != g.endOutEdges( n1 ); ++it )
      {
         std::cout << "A=" << *it << std::endl;
      }

      for ( Graph1::ConstEdgeIterator it = g.beginOutEdges( n1 ); it != g.endOutEdges( n1 ); ++it )
      {
         std::cout << "A=" << *it << std::endl;
      }

      std::vector<Graph1::ConstVertexIterator> roots = findRoots( g );
      TESTER_ASSERT( roots.size() == 3 );
      TESTER_ASSERT( *roots[ 0 ] == 10 );
      TESTER_ASSERT( *roots[ 1 ] == 13 );
      TESTER_ASSERT( *roots[ 2 ] == 14 );

      VisitorBfsInc visitor;
      visitor.visit( g );
      std::vector<Graph1::ConstVertexIterator> roots2 = findRoots( g );
      TESTER_ASSERT( roots2.size() == 3 );
      TESTER_ASSERT( *roots2[ 0 ] == 11 );
      TESTER_ASSERT( *roots2[ 1 ] == 14 );
      TESTER_ASSERT( *roots2[ 2 ] == 15 );

      ConstVisitorDfsPrint visitor2;
      visitor2.visit( g );
      std::cout << "time=" << time.getCurrentTime() << std::endl;
   }

   void testDfs()
   {
      nll::core::Timer time;
      Graph1 g;

      Graph1::VertexDescriptor n1 = g.addVertex( 1 );
      Graph1::VertexDescriptor n2 = g.addVertex( 2 );
      Graph1::VertexDescriptor n3 = g.addVertex( 3 );
      Graph1::VertexDescriptor n4 = g.addVertex( 4 );
      Graph1::VertexDescriptor n5 = g.addVertex( 5 );
      Graph1::VertexDescriptor n6 = g.addVertex( 6 );

      g.addEdge( n1, n2, "a" );
      g.addEdge( n2, n3, "b" );
      g.addEdge( n2, n4, "c" );
      g.addEdge( n4, n5, "d" );
      g.addEdge( n4, n6, "e" );
      g.addEdge( n4, n1, "f" );
      g.addEdge( n1, n4, "g" );

      ConstVisitorDfsPrint visitor;
      visitor.visit( g );

      TESTER_ASSERT( visitor.discoveryList.size() == 19 );
      TESTER_ASSERT( visitor.discoveryList[ 0 ] == "V1" );
      TESTER_ASSERT( visitor.discoveryList[ 1 ] == "Ea" );
      TESTER_ASSERT( visitor.discoveryList[ 2 ] == "V2" );
      TESTER_ASSERT( visitor.discoveryList[ 3 ] == "Eb" );
      TESTER_ASSERT( visitor.discoveryList[ 4 ] == "V3" );
      TESTER_ASSERT( visitor.discoveryList[ 5 ] == "EV3" );
      TESTER_ASSERT( visitor.discoveryList[ 6 ] == "Ec" );
      TESTER_ASSERT( visitor.discoveryList[ 7 ] == "V4" );
      TESTER_ASSERT( visitor.discoveryList[ 8 ] == "Ed" );
      TESTER_ASSERT( visitor.discoveryList[ 9 ] == "V5" );
      TESTER_ASSERT( visitor.discoveryList[ 10 ] == "EV5" );
      TESTER_ASSERT( visitor.discoveryList[ 11 ] == "Ee" );
      TESTER_ASSERT( visitor.discoveryList[ 12 ] == "V6" );
      TESTER_ASSERT( visitor.discoveryList[ 13 ] == "EV6" );
      TESTER_ASSERT( visitor.discoveryList[ 14 ] == "Ef" );
      TESTER_ASSERT( visitor.discoveryList[ 15 ] == "EV4" );
      TESTER_ASSERT( visitor.discoveryList[ 16 ] == "EV2" );
      TESTER_ASSERT( visitor.discoveryList[ 17 ] == "Eg" );
      TESTER_ASSERT( visitor.discoveryList[ 18 ] == "EV1" );
   }

   void testBfs()
   {
      nll::core::Timer time;
      Graph1 g;

      Graph1::VertexDescriptor n1 = g.addVertex( 1 );
      Graph1::VertexDescriptor n2 = g.addVertex( 2 );
      Graph1::VertexDescriptor n3 = g.addVertex( 3 );
      Graph1::VertexDescriptor n4 = g.addVertex( 4 );
      Graph1::VertexDescriptor n5 = g.addVertex( 5 );
      Graph1::VertexDescriptor n6 = g.addVertex( 6 );

      g.addEdge( n1, n2, "a" );
      g.addEdge( n2, n3, "b" );
      g.addEdge( n2, n4, "c" );
      g.addEdge( n4, n5, "d" );
      g.addEdge( n4, n6, "e" );
      g.addEdge( n4, n1, "f" );
      g.addEdge( n1, n4, "g" );

      ConstVisitorBfsPrint visitor;
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
TESTER_TEST_SUITE(TestGraph);
TESTER_TEST( test1 );
TESTER_TEST( testDfs );
TESTER_TEST( testBfs );
TESTER_TEST_SUITE_END();
#endif