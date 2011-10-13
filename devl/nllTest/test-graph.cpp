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

   template <class NodeDetailT = Empty, class ArcDetailT = Empty, class DirectedT = Directed>
   struct GraphImplAdjencyList
   {
      typedef DirectedT                   Direction;
      typedef NodeDetailT                 NodeDetail;
      typedef ArcDetailT                  ArcDetail;

      struct Arc
      {
         friend GraphImplAdjencyList;

      private:
         Arc( ui32 duid, ui32 toUid ) : detailUid( duid ), toNodeUid( toUid )
         {}

         ui32           detailUid;
         ui32           toNodeUid;
      };
      typedef std::vector<Arc>     Arcs;

      struct Node
      {
         friend GraphImplAdjencyList;

      private:
         Node( ui32 duid ) : detailUid( duid )
         {}

         ui32  detailUid;
         Arcs  arcs;
      };
      typedef std::vector<Node>     Nodes;

      struct NodeDescriptor
      {
         NodeDescriptor( ui32 suid, ui32 duid ) : detailUid( duid ), storageUid( suid )
         {}

         ui32           detailUid;
         ui32           storageUid;
      };

      struct ArcDescriptor
      {
         ArcDescriptor( ui32 sid, ui32 did, const NodeDescriptor& s, const NodeDescriptor& d ) : storageUid( sid ), detailUid( did ), src( s ), dst( d )
         {
         }
         ui32           detailUid;
         ui32           storageUid;
         NodeDescriptor src;
         NodeDescriptor dst;
      };

      typedef std::vector<NodeDescriptor>    NodeDescriptors;
      typedef std::vector<ArcDescriptor>     ArcDescriptors;
      typedef std::vector<NodeDetail>        NodeDetails;
      typedef std::vector<ArcDetail>         ArcDetails;

      class ConstNodeIterator
      {
      public:
         ConstNodeIterator( GraphImplAdjencyList& graph, ui32 uid ) : _graph( graph ), _it( graph._nodes.begin() + uid )
         {}

         ConstNodeIterator& operator++()
         {
            ++_it;
            return *this;
         }

         ConstNodeIterator& operator++(int)
         {
            ConstNodeIterator it( *this );
            ++_it;
            return it;
         }

         bool operator!=( const ConstNodeIterator& it ) const
         {
            return _it != it._it;
         }

         const NodeDetail& operator*() const
         {
            return _graph._nodeDetails[ _it->detailUid ];
         }

      private:
         ConstNodeIterator operator=( ConstNodeIterator& ); // disabled copy

      protected:
         GraphImplAdjencyList&      _graph;
         typename Nodes::iterator   _it;
      };

      class NodeIterator : public ConstNodeIterator
      {
      public:
         NodeIterator( GraphImplAdjencyList& graph, ui32 uid ) : ConstNodeIterator( graph, uid )
         {}

         bool operator!=( const NodeIterator& it ) const
         {
            return _it != it._it;
         }

         NodeDetail& operator*()
         {
            return _graph._nodeDetails[ _it->detailUid ];
         }
      };

      class ConstArcIterator
      {
      public:
         ConstArcIterator( GraphImplAdjencyList& graph, ui32 nodeUid, ui32 arcUid ) : _graph( graph ), _it( graph._nodes[ nodeUid ].arcs.begin() + arcUid )
         {}

         ConstArcIterator& operator++()
         {
            ++_it;
            return *this;
         }

         ConstArcIterator& operator++(int)
         {
            ConstArcIterator it( *this );
            ++_it;
            return it;
         }

         bool operator!=( const ConstArcIterator& it ) const
         {
            return _it != it._it;
         }

         const ArcDetail& operator*() const
         {
            return _graph._arcDetails[ _it->detailUid ];
         }

      private:
         ConstArcIterator operator=( ConstArcIterator& ); // disabled copy

      protected:
         GraphImplAdjencyList&      _graph;
         typename Arcs::iterator    _it;
      };

      class ArcIterator : public ConstArcIterator
      {
      public:
         ArcIterator( GraphImplAdjencyList& graph, ui32 nodeUid, ui32 arcUid ) : ConstArcIterator( graph, nodeUid, arcUid )
         {}

         ArcDetail& operator*()
         {
            return _graph._arcDetails[ _it->detailUid ];
         }
      };

      class Visitor
      {
      public:
         virtual void start(){}
         virtual void end(){}
      };

      

   public:
      GraphImplAdjencyList()
      {}

      
      NodeDescriptor addNode( const NodeDetail& detail )
      {
         return addNodeImpl( Direction(), detail );
      }
      
      ArcDescriptor addArc( const NodeDescriptor& n1, const NodeDescriptor& n2, const ArcDetail& detail )
      {
         return addArcImpl( Direction(), n1, n2, detail );
      }

      ArcDescriptor getArc( const NodeDescriptor& n1, const NodeDescriptor& n2, bool& found )
      {
         return getArcImpl( Direction(), n1, n2, found );
      }

      NodeIterator beginNodes()
      {
         return NodeIterator( *this, 0 );
      }

      NodeIterator endNodes()
      {
         return NodeIterator( *this, _nodes.size() );
      }

      ConstNodeIterator beginNodes() const
      {
         return ConstNodeIterator( *this, 0 );
      }

      ConstNodeIterator endNodes() const
      {
         return ConstNodeIterator( *this, _nodes.size() );
      }

      ArcIterator beginOutEdges( const NodeDescriptor& desc )
      {
         return ArcIterator( *this, desc.storageUid, 0 );
      }

      ConstArcIterator beginOutEdges( const NodeDescriptor& desc ) const
      {
         return ConstArcIterator( *this, desc.storageUid, 0 );
      }

      ArcIterator endOutEdges( const NodeDescriptor& desc )
      {
         return ArcIterator( *this, desc.storageUid, _nodes[ desc.storageUid ].arcs.size() );
      }

      ConstArcIterator endOutEdges( const NodeDescriptor& desc ) const
      {
         return ConstArcIterator( *this, desc.storageUid, _nodes[ desc.storageUid ].arcs.size() );
      }

      const ArcDetail& getArcDetail( const ArcDescriptor& desc ) const
      {
         return _arcDetails[ desc.detailUid ];
      }

      ArcDetail& getArcDetail( const ArcDescriptor& desc )
      {
         return _arcDetails[ desc.detailUid ];
      }

      const ArcDetail& getArcDetail( const Arc& desc ) const
      {
         return _arcDetails[ desc.detailUid ];
      }

      ArcDetail& getArcDetail( const Arc& desc )
      {
         return _arcDetails[ desc.detailUid ];
      }

      NodeDetail& getNodeDetail( const NodeDescriptor& desc )
      {
         return _nodeDetails[ desc.detailUid ];
      }

      const NodeDetail& getNodeDetail( const NodeDescriptor& desc ) const
      {
         return _nodeDetails[ desc.detailUid ];
      }

      NodeDetail& getNodeDetail( const Node& desc )
      {
         return _nodeDetails[ desc.detailUid ];
      }

      const NodeDetail& getNodeDetail( const Node& desc ) const
      {
         return _nodeDetails[ desc.detailUid ];
      }

   private:
      NodeDescriptor createNode( ui32 detailUid )
      {
         NodeDescriptor desc( _nodes.size(), detailUid );
         _nodes.push_back( Node( detailUid ) );
         return desc;
      }

      NodeDetail& createNodeDetail( const NodeDetail& detail, ui32& uid )
      {
         if ( _recycleNodeDetails.size() )
         {
            const ui32 uidRecycled = *_recycleNodeDetails.rbegin();
            NodeDetail& d = _nodeDetails[ uidRecycled ];
            _recycleNodeDetails.pop_back();

            uid = uidRecycled;
            d = detail;
            return d;
         } else {
            uid = _nodeDetails.size();
            _nodeDetails.push_back( detail );
            return *_nodeDetails.rbegin();
         }
      }

      ArcDetail& createArcDetail( const ArcDetail& detail, ui32& uid )
      {
         if ( _recycleArcDetails.size() )
         {
            const ui32 uidRecycled = *_recycleArcDetails.rbegin();
            ArcDetail& d = _arcDetails[ uidRecycled ];
            _recycleArcDetails.pop_back();

            uid = uidRecycled;
            d = detail;
            return d;
         } else {
            uid = _arcDetails.size();
            _arcDetails.push_back( detail );
            return *_arcDetails.rbegin();
         }
      }

      
      template <class IsDirected>
      NodeDescriptor addNodeImpl( IsDirected fswitch, const NodeDetail& detail );

      
      template <class IsDirected>
      ArcDescriptor addArcImpl( IsDirected fswitch, const NodeDescriptor& n1, const NodeDescriptor& n2, const ArcDetail& detail );


      template <class IsDirected>
      ArcDescriptor getArcImpl( IsDirected fswitch, const NodeDescriptor& n1, const NodeDescriptor& n2, bool& found );

      template <>
      NodeDescriptor addNodeImpl( Directed, const NodeDetail& detail )
      {
         ui32 duid;
         createNodeDetail( detail, duid );
         return createNode( duid );
      }  

      template <>
      ArcDescriptor addArcImpl( Directed, const NodeDescriptor& n1, const NodeDescriptor& n2, const ArcDetail& detail )
      {
         ui32 uid;
         createArcDetail( detail, uid );
         Node& node = _nodes[ n1.storageUid ];
         ArcDescriptor newArc( node.arcs.size(), uid, n1, n2 );

         #ifdef NLL_GRAPH_SECURE_CHECKS
         for ( Arcs::iterator it = node.arcs.begin(); it != node.arcs.end(); ++it )
         {
            ensure( it->toNodeUid != n2.storageUid, "the arc has been added twice" );
         }
         #endif
         node.arcs.push_back( Arc( uid, n2.storageUid ) );
         return newArc;
      }

      template <>
      ArcDescriptor getArcImpl( Directed, const NodeDescriptor& n1, const NodeDescriptor& n2, bool& found )
      {
         Node& node = _nodes[ n1.storageUid ];
         Arcs& arcs = node.arcs;
         Arcs::iterator it = std::find_if( arcs.begin(), arcs.end(), PredicateAreDirectedNodeUidEqual( n2.storageUid ) );
         if ( it != arcs.end() )
         {
            found = true;
            return ArcDescriptor( it - arcs.begin(), it->detailUid, n1, n2 );
         } else {
            found = false;
            return ArcDescriptor( (ui32)-1, (ui32)-1, n1, n2 );
         }
      }

      struct PredicateAreDirectedNodeUidEqual
      {
         PredicateAreDirectedNodeUidEqual( ui32 toNodeUid ) : _toNodeUid( toNodeUid )
         {}

         bool operator()( const Arc& n ) const
         {
            return _toNodeUid == n.toNodeUid;
         }

      private:
         ui32   _toNodeUid;
      };


   private:
      Nodes             _nodes;

      NodeDetails       _nodeDetails;
      ArcDetails        _arcDetails;
      std::vector<ui32> _recycleNodeDetails;
      std::vector<ui32> _recycleArcDetails;
   };

   template <class Impl = GraphImpl<> >
   class Graph
   {
   public:
      typedef typename Impl::NodeDetail     NodeDetail;
      typedef typename Impl::ArcDetail      ArcDetail;
      typedef typename Impl::NodeDescriptor NodeDescriptor;
      typedef typename Impl::ArcDescriptor  ArcDescriptor;
      typedef typename Impl::Node           Node;
      typedef typename Impl::Arc            Arc;
      typedef Impl                          GraphImpl;

      typedef typename Impl::NodeIterator      NodeIterator;
      typedef typename Impl::ConstNodeIterator ConstNodeIterator;
      typedef typename Impl::ArcIterator       ArcIterator;
      typedef typename Impl::ConstArcIterator  ConstArcIterator;

   public:
      Graph( const Impl impl = Impl() ) : _impl( impl )
      {}

      NodeDescriptor addNode( const NodeDetail details = NodeDetail() )
      {
         return _impl.addNode( details );
      }

      ArcDescriptor addArc( const NodeDescriptor& n1, const NodeDescriptor& n2, const ArcDetail d = ArcDetail() )
      {
         return _impl.addArc( n1, n2, d );
      }

      ArcDescriptor getArc( const NodeDescriptor& n1, const NodeDescriptor& n2, bool& found )
      {
         return _impl.getArc( n1, n2, found );
      }

      NodeIterator beginNodes()
      {
         return _impl.beginNodes();
      }

      NodeIterator endNodes()
      {
         return _impl.endNodes();
      }

      ConstNodeIterator beginNodes() const
      {
         return _impl.beginNodes();
      }

      ConstNodeIterator endNodes() const
      {
         return _impl.endNodes();
      }

      ArcIterator beginOutEdges( const NodeDescriptor& desc )
      {
         return _impl.beginOutEdges( desc );
      }

      ArcIterator beginOutEdges( const NodeDescriptor& desc ) const
      {
         return _impl.beginOutEdges( desc );
      }

      ArcIterator endOutEdges( const NodeDescriptor& desc )
      {
         return _impl.endOutEdges( desc );
      }

      ArcIterator endOutEdges( const NodeDescriptor& desc ) const
      {
         return _impl.endOutEdges( desc );
      }

      const ArcDetail& operator[]( const ArcDescriptor& desc ) const
      {
         return _impl.getArcDetail( desc );
      }

      ArcDetail& operator[]( const ArcDescriptor& desc )
      {
         return _impl.getArcDetail( desc );
      }

      const ArcDetail& operator[]( const Arc& desc ) const
      {
         return _impl.getArcDetail( desc );
      }

      ArcDetail& operator[]( const Arc& desc )
      {
         return _impl.getArcDetail( desc );
      }

      const NodeDetail& operator[]( const NodeDescriptor& desc ) const
      {
         return _impl.getNodeDetail( desc );
      }

      NodeDetail& operator[]( const NodeDescriptor& desc )
      {
         return _impl.getNodeDetail( desc );
      }

      const NodeDetail& operator[]( const Node& desc ) const
      {
         return _impl.getNodeDetail( desc );
      }

      NodeDetail& operator[]( const Node& desc )
      {
         return _impl.getNodeDetail( desc );
      }

   private:
      GraphImpl   _impl;
   };
}
}

class TestGraph
{
public:
   void test1()
   {
      typedef core::Graph< GraphImplAdjencyList<int, std::string, Directed> > Graph;
      Graph g;

      Graph::NodeDescriptor n1 = g.addNode( 10 );
      Graph::NodeDescriptor n2 = g.addNode( 11 );
      Graph::NodeDescriptor n3 = g.addNode( 12 );

      Graph::ArcDescriptor a2 = g.addArc( n1, n2, "test" );
      Graph::ArcDescriptor a3 = g.addArc( n1, n3, "test2" );

      bool found;
      Graph::ArcDescriptor a2a = g.getArc( n1, n2, found );


      for ( Graph::NodeIterator it = g.beginNodes(); it != g.endNodes(); ++it )
      {
         std::cout << "V=" << *it << std::endl;
      }

      for ( Graph::ConstNodeIterator it = g.beginNodes(); it != g.endNodes(); ++it )
      {
         std::cout << "V=" << *it << std::endl;
      }

      for ( Graph::ArcIterator it = g.beginOutEdges( n1 ); it != g.endOutEdges( n1 ); ++it )
      {
         std::cout << "A=" << *it << std::endl;
      }

      for ( Graph::ConstArcIterator it = g.beginOutEdges( n1 ); it != g.endOutEdges( n1 ); ++it )
      {
         std::cout << "A=" << *it << std::endl;
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGraph);
TESTER_TEST( test1 );
TESTER_TEST_SUITE_END();
#endif