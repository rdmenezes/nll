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

#ifndef NLL_ALGORITHM_BAYESIAN_NETWORK_ML_PARAM_ESTIMATION_H_
# define NLL_ALGORITHM_BAYESIAN_NETWORK_ML_PARAM_ESTIMATION_H_

namespace nll
{
namespace algorithm
{
   template <class BayesianNetworkT>
   class BayesianNetworkMaximumLikelihoodParameterEstimation
   {
   public:
      typedef BayesianNetworkT                     Network;
      typedef typename BayesianNetworkT::Factor    Factor;
      typedef typename Factor::EvidenceValue       EvidenceValue;
      typedef typename Factor::VectorI             VectorI;
      
      /**
       @brief Computes the ML estimates of the BN network given <fullyObservedData>, a fully observed dataset
       */
      void compute( const std::vector<EvidenceValue>& fullyObservedData, const VectorI& domain, Network& outMlEstimation )
      {
         ensure( 0, "This method is not implemented for this kind of network" );
      }
   };

   template <>
   class BayesianNetworkMaximumLikelihoodParameterEstimation< BayesianNetwork< PotentialTable > >
   {
   public:
      typedef BayesianNetwork< PotentialTable >   Network;
      typedef Network::Factor                     Factor;
      typedef Factor::EvidenceValue               EvidenceValue;
      typedef Factor::VectorI                     VectorI;
      
      /**
       @brief Computes the ML estimates of the BN network given <fullyObservedData>, a fully observed dataset
       */
      void compute( const VectorI& domain, const std::vector<EvidenceValue>& fullyObservedData, Network& outMlEstimation )
      {
         std::vector<Factor*> factors;
         getFactors( outMlEstimation, factors );

         // ML decomposability property: we can estimate each factor independently
         for ( size_t n = 0; n < factors.size(); ++n )
         {
            factors[ n ]->maximumLikelihoodEstimate( domain, fullyObservedData );
         }
      }
   };
}
}

#endif
