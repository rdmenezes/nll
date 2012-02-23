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

#ifndef NLL_ALGORITHM_BAYESIAN_INFERENCE_NAIVE_H_
# define NLL_ALGORITHM_BAYESIAN_INFERENCE_NAIVE_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief computes p(U-E-Q | E=e) with U all variables, Q query variables, E observed variables (evidence)
    @param BNetwork the bayesian network type

    This is the simplest implementation on the exact inference on BN, this should only be used for debug only as this method is extremly inefficient.
    It is creating the full enumeration without using at all the independence property of the factors
    */
   template <class BNetwork>
   class BayesianInferenceNaive
   {
   public:
      typedef PotentialGaussianMoment::Vector         Vector;
      typedef PotentialGaussianMoment::VectorI        VectorI;
      typedef typename BNetwork::Factor               Factor;
      typedef typename Factor::EvidenceValue          EvidenceValue;
      typedef BNetwork                                Bn;
      typedef typename Bn::Graph                      Graph;

   public:
      /**
       @param bn the bayesian network
       @param evidenceDomain the domain of the evidence
       @param evidenceValue the value of the evidence
       @param domainOfInterest the domain we want to get the likelihood
       */
      Factor run( const BayesianNetwork<Factor>& bn, 
                  const VectorI& evidenceDomain,
                  const EvidenceValue& evidenceValue,
                  const VectorI& domainOfInterest ) const
      {
         std::vector<const Factor*> factors;
         getFactors( bn, factors );

         if ( !factors.size() )
            return Factor();

         std::set<ui32> varEliminationOrder;
         for ( size_t n = 0; n < factors.size(); ++n )
         {
            std::for_each( factors[ n ]->getDomain().begin(),
                           factors[ n ]->getDomain().end(),
                           [&]( ui32 v )
                           {
                              varEliminationOrder.insert( v );
                           } );
         }

         // discard the domain of interest and evidence
         for ( ui32 n = 0; n < domainOfInterest.size(); ++n )
         {
            varEliminationOrder.erase( domainOfInterest[ n ] );
         }

         for ( ui32 n = 0; n < evidenceDomain.size(); ++n )
         {
            varEliminationOrder.erase( evidenceDomain[ n ] );
         }

         // enter the evidence in the factors
         VectorI evidenceDomainMarg( 1 );
         EvidenceValue evidenceValueMarg( 1 );

         std::vector<Factor> newFactors;
         newFactors.reserve( factors.size() );
         for ( size_t n = 0; n < factors.size(); ++n )
         {
            newFactors.push_back( *factors[ n ] );

            // check we have some evidence
            for ( ui32 evidenceVar = 0; evidenceVar < evidenceDomain.size(); ++evidenceVar )
            {
               bool isInPotential = std::binary_search( newFactors[ n ].getDomain().begin(),
                                                        newFactors[ n ].getDomain().end(),
                                                        evidenceDomain[ evidenceVar ] );
               if ( isInPotential )
               {
                  evidenceDomainMarg[ 0 ] = evidenceDomain[ evidenceVar ];
                  evidenceValueMarg[ 0 ] = evidenceValue[ evidenceVar ];
                  newFactors[ n ] = newFactors[ n ].conditioning( evidenceValueMarg, evidenceDomainMarg );
               }
            }
         }

         // finally create the big potential and marginalize out everything
         Factor f = newFactors[ 0 ];
         for ( size_t n = 1; n < newFactors.size(); ++n )
         {
            f = f * newFactors[ n ];
         }

         VectorI domainToMarginalize( 1 );
         for ( std::set<ui32>::const_iterator it = varEliminationOrder.begin(); it != varEliminationOrder.end(); ++it )
         {
            domainToMarginalize[ 0 ] = *it;
            f = f.marginalization( domainToMarginalize );
         }

         // we need to normalize the potential as with evidence it may not be anymore
         f.normalizeFull();
         return f;
      }
   };
}
}

#endif