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

#ifndef NLL_ALGORITHM_BAYESIAN_NETWORK_SAMPLING_H_
# define NLL_ALGORITHM_BAYESIAN_NETWORK_SAMPLING_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Generates samples from the network
    */
   template <class Network>
   class BayesianNetworkSampling
   {
   public:
      typedef typename Network::Factor          Factor;
      typedef typename Factor::EvidenceValue    EvidenceValue;
      typedef PotentialTable::VectorI           VectorI;

      /**
       @brief given a bayesian network, generate iid samples
       */
      void compute( const Network& bayesianNetwork, ui32 nbSamples, std::vector<EvidenceValue>& outSamples, VectorI& outDomain ) const
      {
         ensure( 0, "NOT IMPLEMENTED FOR THE TYPE OF NETWORK" );
      }
   };

   /**
    @ingroup algorithm
    @brief Specialization for potential tables
    */
   template <>
   class BayesianNetworkSampling< BayesianNetwork<PotentialTable> >
   {
   public:
      typedef BayesianNetwork<PotentialTable>   Network;
      typedef Network::Factor                   Factor;
      typedef Factor::EvidenceValue             EvidenceValue;
      typedef PotentialTable::VectorI           VectorI;

      /**
       @brief given a bayesian network, generate iid samples
       */
      void compute( const Network& bayesianNetwork, ui32 nbSamples, std::vector<EvidenceValue>& outSamples, VectorI& outDomain ) const
      {
         ui32 maxDomain = 0;
         outSamples.clear();

         // first retrieve all the factors and build some indexes
         std::vector<const Factor*> factors;
         getFactors( bayesianNetwork, factors );
         std::set<ui32> mainDomain;
         std::set<ui32> fullDomain;
         for ( size_t n = 0; n < factors.size(); ++n )
         {
            const Factor& f = *factors[ n ];
            ensure( f.getDomain().size(), "there is no domain for this factor?" );
            maxDomain = std::max( maxDomain, f.getDomain()[ f.getDomain().size() - 1 ] );

            const ui32 currentDomain = f.getDomain()[ 0 ];
            mainDomain.insert( currentDomain );

            fullDomain.insert( f.getDomain().begin(), f.getDomain().end() );
         }
         ensure( fullDomain == mainDomain, "undeclared variables" );

         // index the factors by their main domain
         std::vector<const Factor*> factorsIndexedByDomain( maxDomain + 1 );    // here we store the table in an array according to domain[0] of each factor
         for ( size_t n = 0; n < factors.size(); ++n )
         {
            const Factor& f = *factors[ n ];
            const ui32 currentDomain = f.getDomain()[ 0 ];
            ensure( factorsIndexedByDomain[ currentDomain ] == 0, "there are several factors with the same main domain - hugh?" );
            factorsIndexedByDomain[ currentDomain ] = &f;
         }

         // map domain to the final domain array indexes
         std::vector<ui32> domainToFinalDomain( maxDomain + 1 );
         ui32 index = 0;
         outDomain = VectorI( static_cast<ui32>( mainDomain.size() ) );
         for ( std::set<ui32>::const_iterator it = mainDomain.begin(); it != mainDomain.end(); ++it, ++index )
         {
            const ui32 domainId = *it;
            domainToFinalDomain[ domainId ] = index;
            outDomain[ index ] = domainId;
         }

         // finally create the iid samples
         for ( ui32 sampleid = 0; sampleid < nbSamples; ++sampleid )
         {
            EvidenceValue sample( static_cast<ui32>( mainDomain.size() ) );

            // what we need to do is to always know the state of the parent variables. Due to the order property we simply
            // need to sample in reverse order of the domain
            for ( std::set<ui32>::const_reverse_iterator it = mainDomain.rbegin(); it != mainDomain.rend(); ++it )
            {
               // given the parent factors evidence, generate an evidence for the current domain
               // the evidence's domain must be the same as the factor!
               const ui32 mainDomainId = *it;
               const Factor& f = *factorsIndexedByDomain[ mainDomainId ];
               EvidenceValue evidence( f.getDomain().size() );
               for ( ui32 n = 0; n < evidence.size(); ++n )
               {
                  const ui32 domainId = f.getDomain()[ n ];
                  const ui32 domainIdInSample = domainToFinalDomain[ domainId ];
                  evidence[ n ] = sample[ domainIdInSample ];
               }

               f.sample( evidence );
               sample[ domainToFinalDomain[ *it ] ] = evidence[ 0 ];
            }

            outSamples.push_back( sample );
         }
      }
   };


}
}

#endif
