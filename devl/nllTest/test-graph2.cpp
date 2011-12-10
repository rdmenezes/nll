#include <nll/nll.h>
#include <stack>
#include <tester/register.h>
#include "config.h"


using namespace nll;
using namespace nll::core;


namespace nll
{
namespace core
{
   
}
}


class TestGraph2
{
   template <class Graph>
   class ConstVisitorDfsPrint : public GraphVisitorDfs<Graph>
   {
   public:
      typedef typename Graph::EdgeMapper<std::string>    EdgeMapper;
      typedef typename Graph::VertexMapper<int>          VertexMapper;
      typedef typename Graph::VertexDescriptor           VertexDescriptor;
      typedef typename Graph::EdgeDescriptor             EdgeDescriptor;


      ConstVisitorDfsPrint( const VertexMapper& v, const EdgeMapper& e ) : _v( v ), _e( e )
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
      typedef GraphAdgencyList<VertexType, EdgeType>   Graph1;
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
		 /*
		 Graph1::vertex_iterator vvv = g.begin();
		 Graph1::Vertex& vertex = *vvv;
		 Graph1::edge_iterator vite = vertex.begin();*/
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
         Graph1::const_edge_iterator ie1 = (*iv1).begin();
         Graph1::const_edge_iterator ie1c = ie1;
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

   template <class VertexType, class EdgeType>
   void testBfsImpl()
   {
      typedef GraphAdgencyList<VertexType, EdgeType>   Graph1;

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

   void testBfsVV()
   {
      testBfsImpl<MapperVector, MapperVector>();
   }

   void testBfsSS()
   {
      testBfsImpl<MapperSet, MapperSet>();
   }

   void computeRoots()
   {
      typedef GraphAdgencyList<MapperVector, MapperVector>   Graph1;

      nll::core::Timer time;
      Graph1 g;

      Graph1::VertexDescriptor n1 = g.addVertex();
      Graph1::VertexDescriptor n2 = g.addVertex();
      Graph1::VertexDescriptor n3 = g.addVertex();
      Graph1::VertexDescriptor n4 = g.addVertex();
      Graph1::VertexDescriptor n5 = g.addVertex();
      Graph1::VertexDescriptor n6 = g.addVertex();

      Graph1::EdgeDescriptor e1 = g.addEdge( n1, n2 );
      Graph1::EdgeDescriptor e2 = g.addEdge( n2, n3 );
      Graph1::EdgeDescriptor e3 = g.addEdge( n2, n4 );
      Graph1::EdgeDescriptor e4 = g.addEdge( n4, n5 );
      Graph1::EdgeDescriptor e5 = g.addEdge( n6, n4 );
      Graph1::EdgeDescriptor e7 = g.addEdge( n1, n4 );

      std::vector<Graph1::const_vertex_iterator> roots = getGraphRoots( g );
      TESTER_ASSERT( roots.size() == 2 );
      TESTER_ASSERT( Graph1::const_vertex_iterator( g.getIterator( n1 ) ) == roots[ 0 ] );
      TESTER_ASSERT( Graph1::const_vertex_iterator( g.getIterator( n6 ) ) == roots[ 1 ] );
   }

   void testDijkstra()
   {
      typedef GraphAdgencyList<MapperVector, MapperVector>   Graph1;

      nll::core::Timer time;
      Graph1 g;

      Graph1::VertexDescriptor n1 = g.addVertex();
      Graph1::VertexDescriptor n2 = g.addVertex();
      Graph1::VertexDescriptor n3 = g.addVertex();
      Graph1::VertexDescriptor n4 = g.addVertex();
      Graph1::VertexDescriptor n5 = g.addVertex();
      Graph1::VertexDescriptor n6 = g.addVertex();

      Graph1::EdgeDescriptor e1 = g.addEdge( n1, n2 );
      Graph1::EdgeDescriptor e2 = g.addEdge( n1, n3 );
      Graph1::EdgeDescriptor e3 = g.addEdge( n1, n6 );
      Graph1::EdgeDescriptor e4 = g.addEdge( n2, n3 );
      Graph1::EdgeDescriptor e5 = g.addEdge( n2, n4 );
      Graph1::EdgeDescriptor e6 = g.addEdge( n3, n4 );
      Graph1::EdgeDescriptor e7 = g.addEdge( n4, n5 );
      Graph1::EdgeDescriptor e8 = g.addEdge( n6, n5 );
      Graph1::EdgeDescriptor e9 = g.addEdge( n3, n6 );

      Graph1::EdgeMapper<double> emapper( g );
      emapper[ e1 ] = 7;
      emapper[ e2 ] = 9;
      emapper[ e3 ] = 14;
      emapper[ e4 ] = 10;
      emapper[ e5 ] = 15;
      emapper[ e6 ] = 11;
      emapper[ e7 ] = 6;
      emapper[ e8 ] = 9;
      emapper[ e9 ] = 2;

      Dijkstra<Graph1, double> dijkstra( emapper, g.getIterator(n1) );
      dijkstra.visit( g );
      dijkstra.getPathTo( g.getIterator(n6) );
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGraph2);
/*
TESTER_TEST( testVector );
TESTER_TEST( testSet );
TESTER_TEST( testGraphVV );
TESTER_TEST( testGraphSS );
TESTER_TEST( testGraphSV );
TESTER_TEST( testGraphVS );
TESTER_TEST( testBfsVV );
TESTER_TEST( testBfsSS );
TESTER_TEST( computeRoots );*/
TESTER_TEST( testDijkstra );
TESTER_TEST_SUITE_END();
#endif