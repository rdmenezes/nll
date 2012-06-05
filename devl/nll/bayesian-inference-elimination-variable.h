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

#ifndef NLL_ALGORITHM_BAYESIAN_INFERENCE_VARIABLE_ELIMINATION_H_
# define NLL_ALGORITHM_BAYESIAN_INFERENCE_VARIABLE_ELIMINATION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief computes p(U-E-Q | E=e) with U all variables, Q query variables, E observed variables (evidence)

    Computes exact inference given a graph, query variables and evidence
    */
   template <class BNetwork>
   class BayesianInferenceVariableElimination
   {
   public:
      typedef PotentialGaussianMoment::Vector         Vector;
      typedef PotentialGaussianMoment::VectorI        VectorI;
      typedef typename BNetwork::Factor               Factor;
      typedef typename Factor::EvidenceValue          EvidenceValue;
      typedef BNetwork                                Bn;
      typedef typename Bn::Graph                      Graph;

      typedef TraitsInstanciateBayesianPotential<Factor>             FactorCreator;
      typedef typename FactorCreator::PotentialInstanciationType     FactorCreatorType;

   public:
      FactorCreatorType run( const BayesianNetwork<Factor>& bn, 
                             const VectorI& evidenceDomain,
                             const EvidenceValue& evidenceValue,
                             const VectorI& domainOfInterest ) const
      {
         std::vector<const Factor*> _factors;
         getFactors( bn, _factors );

         std::vector<FactorCreatorType> factors;
         factors.reserve( _factors.size() );
         for ( size_t n = 0; n < _factors.size(); ++n )
         {
            factors.push_back( FactorCreator::create( *_factors[ n ] ) );
         }

         if ( !factors.size() )
            return FactorCreatorType();

         std::set<ui32> varEliminationOrder;
         for ( size_t n = 0; n < factors.size(); ++n )
         {
            std::for_each( factors[ n ].getDomain().begin(),
                           factors[ n ].getDomain().end(),
                           [&]( ui32 v )
                           {
                              varEliminationOrder.insert( v );
                           } );
         }

         std::vector<ui32> eliminationOrder;
         eliminationOrder.reserve( varEliminationOrder.size() );

         for ( std::set<ui32>::const_iterator it = varEliminationOrder.begin(); it != varEliminationOrder.end(); ++it )
         {
            eliminationOrder.push_back( *it );
         }

         return run( bn, evidenceDomain, evidenceValue, domainOfInterest, eliminationOrder );
      }

      /**
       @param bn the bayesian network
       @param evidenceDomain the domain of the evidence
       @param evidenceValue the value of the evidence
       @param domainOfInterest the domain we want to get the likelihood
       @param variableEliminationOrder the variable elimination order
       */
      FactorCreatorType run( const BayesianNetwork<Factor>& bn, 
                             const VectorI& evidenceDomain,
                             const EvidenceValue& evidenceValue,
                             const VectorI& domainOfInterest,
                             const std::vector<ui32>& variableEliminationOrder ) const
      {
         std::vector<const Factor*> _factors;
         getFactors( bn, _factors );

         std::vector<FactorCreatorType> factors;
         factors.reserve( _factors.size() );
         for ( size_t n = 0; n < _factors.size(); ++n )
         {
            factors.push_back( FactorCreator::create( *_factors[ n ] ) );
         }

         if ( !factors.size() )
            return FactorCreatorType();

         std::vector<ui32> varEliminationOrder = variableEliminationOrder;

         // discard the domain of interest and evidence
         for ( ui32 n = 0; n < domainOfInterest.size(); ++n )
         {
            std::vector<ui32>::iterator it = std::find( varEliminationOrder.begin(), varEliminationOrder.end(), domainOfInterest[ n ] );
            if ( it != varEliminationOrder.end() )
            {
               varEliminationOrder.erase( it );
            }
            assert( std::find( varEliminationOrder.begin(), varEliminationOrder.end(), domainOfInterest[ n ] ) == varEliminationOrder.end() );
         }

         for ( ui32 n = 0; n < evidenceDomain.size(); ++n )
         {
            std::vector<ui32>::iterator it = std::find( varEliminationOrder.begin(), varEliminationOrder.end(), evidenceDomain[ n ] );
            if ( it != varEliminationOrder.end() )
            {
               varEliminationOrder.erase( it );
            }
            assert( std::find( varEliminationOrder.begin(), varEliminationOrder.end(), evidenceDomain[ n ] ) == varEliminationOrder.end() );
         }

         // enter the evidence in the factors
         VectorI evidenceDomainMarg( 1 );
         EvidenceValue evidenceValueMarg( 1 );

         std::vector<FactorCreatorType> newFactors;
         newFactors.reserve( factors.size() );
         for ( size_t n = 0; n < factors.size(); ++n )
         {
            newFactors.push_back( factors[ n ] );

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

         // finally run the variable elimination algo
         for ( std::vector<ui32>::const_iterator it = varEliminationOrder.begin(); it != varEliminationOrder.end(); ++it )
         {
            std::vector<ui32> factorsInScope;
            std::vector<ui32> factorsNotInScope;
            getFactorsInScope( newFactors, *it, factorsInScope, factorsNotInScope );

            if ( factorsInScope.size() )
            {
               // just save the unused factors
               std::vector<FactorCreatorType> tmpFactors;
               for ( size_t n = 0; n < factorsNotInScope.size(); ++n )
               {
                  tmpFactors.push_back( newFactors[ factorsNotInScope[ n ] ] );
               }

               // create a big factor with all factors involved with this domain and marginalize it
               FactorCreatorType f = newFactors[ factorsInScope[ 0 ] ];
               for ( size_t n = 1; n < factorsInScope.size(); ++n )
               {
                  f = f * newFactors[ factorsInScope[ n ] ];
               }
               VectorI domainToMarginalize( 1 );
               domainToMarginalize[ 0 ] = *it;
               tmpFactors.push_back( f.marginalization( domainToMarginalize ) );

               // finally export the saved factors for a new pass
               newFactors = tmpFactors;
            }
         }

         // ... and combines all the remaining factors and normalize the final factor
         ensure( newFactors.size(), "unexpected error: no remaining factors!" );
         FactorCreatorType f = newFactors[ 0 ];
         for ( size_t factor = 1; factor < newFactors.size(); ++factor )
         {
            f = f * newFactors[ factor ];
         }

         f.normalizeFull();
         return f;
      }


   private:
      static bool isDomainInScope( const FactorCreatorType& f, ui32 domainVariable )
      {
         return std::binary_search( f.getDomain().begin(),
                                    f.getDomain().end(),
                                    domainVariable );
      }

      static void getFactorsInScope( const std::vector<FactorCreatorType>& factors,
                                    ui32 domainVariable,
                                    std::vector<ui32>& indexFactorsInScope_out,
                                    std::vector<ui32>& indexFactorsNotInScope_out )
      {
         indexFactorsInScope_out.clear();

         for ( size_t factor = 0; factor < factors.size(); ++factor )
         {
            if ( isDomainInScope( factors[ factor ], domainVariable ) )
            {
               indexFactorsInScope_out.push_back( static_cast<ui32>( factor ) );
            } else {
               indexFactorsNotInScope_out.push_back( static_cast<ui32>( factor ) );
            }
         }
      }
   };
}
}

#endif