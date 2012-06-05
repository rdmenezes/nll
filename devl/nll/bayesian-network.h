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

#ifndef NLL_ALGORITHM_BAYESIAN_NETWORK_H_
# define NLL_ALGORITHM_BAYESIAN_NETWORK_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Model a bayesian network with by a single type of factor <FactorT>

    U = all the model's variables

    Inference: 3 main questions:
    - likelyhood queries: computes p(E=e) = Sum_(y in U-E) p(Y=y, E=e)
    - conditional probability queries: on a set of variables Y, computes p(Y=y | E=e) = p(Y=y, E=e) / p(E=e) = sum_(w in U-E-Y) p(W=w, Y=y, E=e)/sum_(z in U-E)p(Z=z, E=e)
    - maximum a posteriory assignment: computes MAP(Y|E=e) = argmax_y p(Y=y | E=e)  <=> argmax_y p(Y=y, E=e)

    Learning:
    - parameter optimizations
    - structure learning
    */
   template <class FactorT>
   class BayesianNetwork
   {
   public:
      typedef FactorT                                    Factor;
      typedef core::GraphAdgencyList<>                   Graph;
      typedef typename Graph::VertexMapper<std::string>  NodeNameMapper;
      typedef typename Graph::VertexMapper<Factor>       NodeFactorMapper;

   public:
      typedef typename Graph::VertexDescriptor           NodeDescritor;

   public:
      BayesianNetwork() : _names( _network ), _factors( _network )
      {}

      NodeDescritor addNode( const std::string& name, const Factor& factor )
      {
         NodeDescritor desc = _network.addVertex();
         _names[ desc ] = name;
         _factors[ desc ] = factor;
         return desc;
      }

      void addLink( const NodeDescritor& source, const NodeDescritor& destination )
      {
         _network.addEdge( source, destination );
      }

      const Graph& getNetwork() const
      {
         return _network;
      }

      const NodeFactorMapper& getFactors() const
      {
         return _factors;
      }

      NodeFactorMapper& getFactors()
      {
         return _factors;
      }

      const NodeNameMapper& getNames() const
      {
         return _names;
      }

   private:
      Graph                   _network;
      NodeNameMapper          _names;
      NodeFactorMapper        _factors;
   };
}
}

#endif