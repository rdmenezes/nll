/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
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

#ifndef NLL_GRAPH_VISITOR_H_
# define NLL_GRAPH_VISITOR_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Breadth First Search visitor
    */
   template <class GraphT>
   class GraphVisitorBfs
   {
   public:
      typedef GraphT                   Graph;
      typedef typename Graph::Vertex   Vertex;
      typedef typename Graph::Edge     Edge;

      typedef typename Graph::const_vertex_iterator   const_vertex_iterator;
      typedef typename Graph::const_edge_iterator     const_edge_iterator;

      GraphVisitorBfs() : _mustAbort( false )
      {}

      // signal the algorithm to stop before the next vertex visited
      bool abort()
      {
         _mustAbort = true;
      }

      // run before the algorithm is started
      virtual void start( const Graph& ){}

      // run after the algorithm has finished
      virtual void finish( const Graph& ){}

      // called when the vertex has been discovered for the first time
      virtual void discoverVertex( const const_vertex_iterator& , const Graph& ){}

      // called when there is a new "source" vertex (e.g., when all edges have been discovered, this method will be called on the next vertex to handle)
      virtual void newSourceVertex( const const_vertex_iterator& , const Graph& ){}

      // called when all the edges have been discovered
      virtual void finishVertex( const const_vertex_iterator& , const Graph& ){}

      // called each time an edge is discovered
      virtual void discoverEdge( const const_edge_iterator& , const Graph& ){}

      virtual void visit( const Graph& g )
      {
         typename GraphT::VertexMapper<char> vertexDiscovered( g, 0 );
         std::vector<const_vertex_iterator> its;
         start( g );
         if ( g.size() )
         {
            for ( const_vertex_iterator vertex = g.begin(); vertex != g.end(); ++vertex )
            {
               if ( _mustAbort )
                  return;
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
                  newSourceVertex( it, g );
                  its.pop_back();
                  
                  for ( const_edge_iterator ite = (*it).begin(); ite != (*it).end(); ++ite )
                  {
                     const_vertex_iterator toVertexIt = g.getIterator( (*ite).getDestination() );
                     const char v = vertexDiscovered[ toVertexIt ];
                     const bool hasBeenDiscovered = v == 1;
                     discoverEdge( ite, g );
                     if ( !hasBeenDiscovered )
                     {
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

   protected:
      bool _mustAbort;
   };

   /**
    @ingroup core
    @brief Breadth First Search visitor, using a priotity queue to sort the breadth vertex
    */
   template <class GraphT, class VertexEvaluator>
   class GraphVisitorBfsPriority
   {
   public:
      typedef GraphT                   Graph;
      typedef typename Graph::Vertex   Vertex;
      typedef typename Graph::Edge     Edge;
      typedef typename Graph::const_vertex_iterator   const_vertex_iterator;
      typedef typename Graph::const_edge_iterator     const_edge_iterator;

      struct Pair
      {
         double                  first;
         const_vertex_iterator   second;

         Pair( double v, const const_vertex_iterator& it ) : first( v ), second( it )
         {}

         bool operator<( const Pair& p ) const
         {
            return first > p.first;
         }
      };

      GraphVisitorBfsPriority() : _mustAbort( false )
      {}

      // signal the algorithm to stop before the next vertex visited
      bool abort()
      {
         _mustAbort = true;
      }

      // run before the algorithm is started
      virtual void start( const Graph& ){}

      // run after the algorithm has finished
      virtual void finish( const Graph& ){}

      // called when the vertex has been discovered for the first time
      virtual void discoverVertex( const const_vertex_iterator& , const Graph& ){}

      // called when there is a new "source" vertex (e.g., when all edges have been discovered, this method will be called on the next vertex to handle)
      virtual void newSourceVertex( const const_vertex_iterator& , const Graph& ){}

      // called when all the edges have been discovered
      virtual void finishVertex( const const_vertex_iterator& , const Graph& ){}

      // called each time an edge is discovered
      virtual void discoverEdge( const const_edge_iterator& , const Graph& ){}

      virtual void visit( const Graph& g, const VertexEvaluator& eval )
      {
         typename GraphT::VertexMapper<char> vertexDiscovered( g, 0 );
         std::priority_queue<Pair> its;
         start( g );
         if ( g.size() )
         {
            for ( const_vertex_iterator vertex = g.begin(); vertex != g.end(); ++vertex )
            {
               if ( _mustAbort )
                  return;
               if ( vertexDiscovered[ vertex ] )
                  continue;   // we already checked this vertex

               
               // queue the first vertex
               its.push( Pair( eval( vertex ), vertex ) );
               const_vertex_iterator& itsrc = its.top().second;
               discoverVertex( itsrc, g );
               vertexDiscovered[ itsrc ] = 1;

               // finally continue until all vertexes have been visited
               while ( its.size() )
               {
                  const_vertex_iterator it = its.top().second;
                  newSourceVertex( it, g );
                  its.pop();
                  
                  for ( const_edge_iterator ite = (*it).begin(); ite != (*it).end(); ++ite )
                  {
                     const_vertex_iterator toVertexIt = g.getIterator( (*ite).getDestination() );
                     const char v = vertexDiscovered[ toVertexIt ];
                     const bool hasBeenDiscovered = v == 1;
                     discoverEdge( ite, g );
                     if ( !hasBeenDiscovered )
                     {
                        its.push( Pair( eval( toVertexIt ), toVertexIt ) );
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

   protected:
      bool              _mustAbort;
   };

   /**
    @ingroup core
    @brief Depth First Search visitor
    */
   template <class GraphT>
   class GraphVisitorDfs
   {
   public:
      typedef GraphT                   Graph;
      typedef typename Graph::Vertex   Vertex;
      typedef typename Graph::Edge     Edge;

      typedef typename Graph::const_vertex_iterator   const_vertex_iterator;
      typedef typename Graph::const_edge_iterator     const_edge_iterator;

      GraphVisitorDfs() : _mustAbort( false )
      {}

      // signal the algorithm to stop before the next vertex is visited
      bool abort()
      {
         _mustAbort = true;
      }

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
         typename GraphT::VertexMapper<char> vertexDiscovered( g, 0 );
         std::list<const_vertex_iterator> its;
         std::list< std::pair<const_edge_iterator, const_edge_iterator> > eits;

         start( g );
         if ( vertexDiscovered.size() )
         {
            for ( const_vertex_iterator vertex = g.begin(); vertex != g.end(); ++vertex )
            {
               if ( _mustAbort )
                  return;
               if ( vertexDiscovered[ vertex ] == 1 )
                  continue;   // we already checked this vertex

               eits.push_back( std::make_pair( (*vertex).begin(), (*vertex).end() ) );
               its.push_back( vertex );
               discoverVertex( vertex, g );
               vertexDiscovered[ vertex ] = 1;

               // finally continue until all vertexes have been visited
               while ( eits.size() )
               {
                  const_edge_iterator& begin = eits.rbegin()->first;
                  const_edge_iterator& end = eits.rbegin()->second;
                  if ( begin == end )
                  {
                     finishVertex( *its.rbegin(), g );
                     eits.pop_back();
                     its.pop_back();
                  } else {
                     // explore another vertex
                     const_vertex_iterator itTo = g.getIterator( (*begin).getDestination() );
                     discoverEdge( begin, g );
                     if ( vertexDiscovered[ itTo ] == 0 )
                     {
                        discoverVertex( itTo, g );
                        vertexDiscovered[ itTo ] = 1;
                        if ( (*itTo).size() )
                        {
                           eits.push_back( std::make_pair( (*itTo).begin(), (*itTo).end() ) );
                           its.push_back( itTo );
                        } else {
                           finishVertex( itTo, g );
                        }
                     }
                     ++begin;
                  }
               }
            }
         }
         finish( g );
      }

   protected:
      bool  _mustAbort;
   };
}
}

#endif
