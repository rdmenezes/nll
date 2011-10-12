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
         Arc( ui32 duid, ui32 toUid ) : detailUid( duid ), toNodeUid( toUid )
         {}

         ui32           detailUid;
         ui32           toNodeUid;
      };
      typedef std::vector<Arc>     Arcs;

      struct Node
      {
         Node( ui32 duid ) : detailUid( duid ), isValid( true )
         {}

         ui32  detailUid;
         bool  isValid;
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

      Arc getArc( const NodeDescriptor& n1, const NodeDescriptor& n2, bool& found )
      {
         return getArcImpl( Direction(), n1, n2, found );
      }

   private:
      NodeDescriptor createNode( ui32 detailUid )
      {
         if ( _recycleNodeDescriptors.size() )
         {
            const ui32 uidRecycled = *_recycleNodeDescriptors.rbegin();
            Node& node = _nodes[ uidRecycled ];
            _recycleNodeDescriptors.pop_back();

            node.isValid = true;
            node.arcs.clear();
            return NodeDescriptor( uidRecycled, detailUid );
         } else {
            NodeDescriptor desc( _nodes.size(), detailUid );

            _nodes.push_back( Node( detailUid ) );
            return desc;
         }
      }

      NodeDetail& createNodeDetail( const NodeDetail& detail, ui32& uid )
      {
         if ( _recycleNodeDetails.size() )
         {
            const ui32 uidRecycled = *_recycleNodeDetails.rbegin();
            NodeDetail& d = _nodeDetails[ uidRecycled ];
            _recycleNodeDescriptors.pop_back();

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
      Arc getArcImpl( IsDirected fswitch, const NodeDescriptor& n1, const NodeDescriptor& n2, bool& found );

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
      Arc getArcImpl( Directed, const NodeDescriptor& n1, const NodeDescriptor& n2, bool& found )
      {
         Node& node = _nodes[ n1.storageUid ];
         Arcs& arcs = node.arcs;
         Arcs::iterator it = std::find_if( arcs.begin(), arcs.end(), PredicateAreDirectedNodeUidEqual( n2.storageUid ) );
         if ( it != arcs.end() )
         {
            found = true;
            return *it;
         } else {
            found = false;
            return Arc( (ui32)-1, (ui32)-1 );
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
      std::vector<ui32> _recycleNodeDescriptors;
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
      typedef typename Impl::Nodes::iterator NodeIterator;

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

      Arc getArc( const NodeDescriptor& n1, const NodeDescriptor& n2, bool& found )
      {
         return _impl.getArc( n1, n2, found );
      }

      NodeIterator beginNodes();






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

      Graph::ArcDescriptor a2 = g.addArc( n1, n2, "test" );

      bool found;
      Graph::Arc a2a = g.getArc( n1, n2, found );


   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGraph);
TESTER_TEST( test1 );
TESTER_TEST_SUITE_END();
#endif