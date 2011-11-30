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
         typedef typename Graph::VertexMapper<unsigned>              VertexMapper;
         typedef typename Graph::VertexMapper<const_vertex_iterator> VertexMapperIts;
         typedef typename Graph::VertexDescriptor                    VertexDescriptor;
         typedef typename Graph::EdgeDescriptor                      EdgeDescriptor;

         _ComputeGraphRoots( VertexMapper& emapper,
                             VertexMapperIts& itmapper ) :  _emapper( emapper ), _itmapper( itmapper )
         {}

         virtual void start( const Graph& )
         {
         }
         
         virtual void discoverVertex( const const_vertex_iterator& it, const Graph& g)
         {
            _itmapper[ it ] = it;
         }

         virtual void discoverEdge( const const_edge_iterator& edge, const Graph& g )
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
            for ( VertexMapper::const_iterator it = _emapper.begin(); it != _emapper.end(); ++it )
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

      /**
       @brief Find the roots 
       */
      template <class Graph>
      std::vector< typename GraphVisitorDfs<Graph>::const_vertex_iterator >
      getGraphRootsDirected( const Graph& g )
      {
         typedef _ComputeGraphRoots<Graph>   RootFinder;

         RootFinder::VertexMapper vmapper( g );
         RootFinder::VertexMapperIts itsmapper( g, g.end() );

         RootFinder rootFinder( vmapper, itsmapper );
         rootFinder.visit( g );
         return rootFinder.getRoots();
      }
   }

   /**
    @brief Find the roots 
    */
   template <class Graph>
   std::vector< typename GraphVisitorDfs<Graph>::const_vertex_iterator >
   getGraphRoots( const Graph& g )
   {
      if ( TraitGraphIsDirected<Graph>::Value )
      {
         return impl::getGraphRootsDirected( g );
      } else {
         ensure( 0, "TO IMPLEMENT" );
      }
   }
}
}

#endif
