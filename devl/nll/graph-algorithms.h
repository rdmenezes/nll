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

#ifndef NLL_GRAPH_ALGORITHMS_H_
# define NLL_GRAPH_ALGORITHMS_H_

namespace nll
{
namespace core
{
   namespace impl
   {
      template <class Graph>
      class _ComputeGraphRoots : public GraphVisitorDfs<Graph>
      {
      public:
         typedef GraphVisitorDfs<Graph> Base;
         typedef typename Base::const_vertex_iterator                         const_vertex_iterator;
         typedef typename Base::const_edge_iterator                           const_edge_iterator;
         typedef typename Graph::template VertexMapper<unsigned>              VertexMapper;
         typedef typename Graph::template VertexMapper<const_vertex_iterator> VertexMapperIts;
         typedef typename Graph::VertexDescriptor                             VertexDescriptor;
         typedef typename Graph::EdgeDescriptor                               EdgeDescriptor;

         _ComputeGraphRoots( VertexMapper& emapper,
                             VertexMapperIts& itmapper ) :  _emapper( emapper ), _itmapper( itmapper )
         {}

         virtual void start( const Graph& )
         {
         }
         
         virtual void discoverVertex( const const_vertex_iterator& it, const Graph&)
         {
            _itmapper[ it ] = it;
         }

         virtual void discoverEdge( const const_edge_iterator& edge, const Graph& )
         {
            ++_emapper[ (*edge).getDestination() ];
         }

         virtual void finishVertex( const const_vertex_iterator& , const Graph& )
         {}

         virtual void finish( const Graph& )
         {
            _roots.clear();
         }

         std::vector<const_vertex_iterator> getRoots() const
         {
            std::vector<const_vertex_iterator> roots;
            for ( typename VertexMapper::const_iterator it = _emapper.begin(); it != _emapper.end(); ++it )
            {
               if ( *it == 0 )
               {
                  size_t uid = it - _emapper.begin();
                  roots.push_back( _itmapper[ uid ] );
               }
            }
            return roots;
         }

      private:
         VertexMapper                        _emapper;
         VertexMapperIts                     _itmapper;
         std::vector<const_vertex_iterator>  _roots;
      };
   }

   /**
    @ingroup core
    @brief Find the roots 
    */
   template <class Graph>
   std::vector< typename GraphVisitorDfs<Graph>::const_vertex_iterator >
   getGraphRoots( const Graph& g )
   {
      typedef impl::_ComputeGraphRoots<Graph>   RootFinder;

      typename RootFinder::VertexMapper vmapper( g );
      typename RootFinder::VertexMapperIts itsmapper( g, g.end() );

      RootFinder rootFinder( vmapper, itsmapper );
      rootFinder.visit( g );
      return rootFinder.getRoots();
   }

   namespace impl
   {
      /**
       @ingroup core
       @brief Vertex evaluator used in the dijkstra algorithm to set the visit priority
       */
      template <class Graph, class MapperValueType>
      class DijkstraVertexEvaluator
      {
      public:
         typedef typename Graph::template VertexMapper<MapperValueType>    VertexMapper;
         typedef typename Graph::const_vertex_iterator                     const_vertex_iterator;

      public:
         DijkstraVertexEvaluator( const VertexMapper& vertexMapper ) : _vertexMapper( vertexMapper )
         {}

         double operator()( const const_vertex_iterator& vit ) const
         {
            return _vertexMapper[ vit ];
         }

      private:
         const VertexMapper& _vertexMapper;
      };
   }

   /**
    @ingroup core
    @brief Dijkstra algorithm
    @see http://en.wikipedia.org/wiki/Dijkstra's_algorithm

    Computes the minimal path from a source vertex given a set of /positive/ distances
    */
   template <class Graph, class MapperValueType>
   class Dijkstra : public GraphVisitorBfsPriority<Graph, impl::DijkstraVertexEvaluator<Graph, MapperValueType> >
   {
   public:
      typedef GraphVisitorBfsPriority<Graph, impl::DijkstraVertexEvaluator<Graph, MapperValueType> > Base;
      typedef typename Base::const_vertex_iterator                         const_vertex_iterator;
      typedef typename Base::const_edge_iterator                           const_edge_iterator;
      typedef typename Graph::template VertexMapper<const_vertex_iterator> VertexMapperBacktrack;
      typedef typename Graph::template VertexMapper<MapperValueType>       VertexMapper;
      typedef typename Graph::template EdgeMapper<MapperValueType>         EdgeMapper;

      Dijkstra( EdgeMapper& distanceMap,
                const const_vertex_iterator& begin ) :  _distanceMap( distanceMap ), _begin( begin ), _current( begin ), _g( 0 )
      {
      }

      virtual void start( const Graph& g )
      {
         const_vertex_iterator end( g.end() );
         _vertexMapperBacktrack = std::auto_ptr<VertexMapperBacktrack>( new VertexMapperBacktrack( g, end ) );

         _g = &g;
      }

      virtual void newSourceVertex( const const_vertex_iterator& it, const Graph& )
      {
         _currentVertexVal = (*_vertexDistance)[ it ];
         _current = it;
      }

      virtual void discoverEdge( const const_edge_iterator& edge, const Graph& g )
      {
         const_vertex_iterator toNode = g.getIterator( (*edge).getDestination() );
         const MapperValueType arcDist = _distanceMap[ edge ];
         ensure( arcDist >= 0, "Dijkstra works only for arc >= 0" );
         MapperValueType& toNodeVal = (*_vertexDistance)[ toNode ];
         if ( ( arcDist + _currentVertexVal ) < toNodeVal )
         {
            toNodeVal = arcDist + _currentVertexVal;
            (*_vertexMapperBacktrack)[ toNode ] = _current;
         }
      }

      virtual void visit( const Graph& g )
      {
         _vertexDistance = std::auto_ptr<VertexMapper>( new VertexMapper( g, std::numeric_limits<MapperValueType>::max() ) );
         (*_vertexDistance)[ _begin ] = 0;

         impl::DijkstraVertexEvaluator<Graph, MapperValueType> eval( *_vertexDistance );
         Base::visit( g, eval );
      }

      std::vector<const_vertex_iterator> getPathTo( const const_vertex_iterator& end )
      {
         std::vector<const_vertex_iterator> path;
         if ( (*_vertexMapperBacktrack)[ end ] == _g->end() )
            return std::vector<const_vertex_iterator>();
         const_vertex_iterator cur = end;
         path.push_back( cur );
         while ( cur != _begin )
         {
            cur = (*_vertexMapperBacktrack)[ cur ];
            path.push_back( cur );
         }

         return std::vector<const_vertex_iterator>( path.rbegin(), path.rend() );
      }

      std::vector<const_vertex_iterator> getPathTo( const typename Graph::VertexDescriptor& desc )
      {
         const_vertex_iterator it = _g->getIterator( desc );
         return getPathTo( it );
      }

      MapperValueType getDistanceTo( const const_vertex_iterator& it ) const
      {

         return (*_vertexDistance)[ it ];
      }

      MapperValueType getDistanceTo( const typename Graph::VertexDescriptor& desc ) const
      {
         return (*_vertexDistance)[ desc ];
      }

   private:
      EdgeMapper                             _distanceMap;
      std::auto_ptr<VertexMapper>            _vertexDistance;
      const_vertex_iterator                  _begin;

      MapperValueType                        _currentVertexVal;
      const_vertex_iterator                  _current;
      std::auto_ptr<VertexMapperBacktrack>   _vertexMapperBacktrack;
      const Graph*                           _g;
   };
}
}

#endif
