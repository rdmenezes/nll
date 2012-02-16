#include <nll/nll.h>
#include <stack>
#include <tester/register.h>
#include "config.h"


using namespace nll;
using namespace nll::core;


namespace nll
{
namespace algorithm
{
   /**
    @brief computes p(U-E, E=e) with E observed variables (evidence) and U-E the rest of the variables modeled by the BN

    Given U = all the model's variables,
    it computes likelyhood queries: p(U-E, E=e) = (Sum_(y in U-E) p(Y=y, E=e))
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

   private:
      template <class Functor>
      class BnVisitor : public core::GraphVisitorBfs<Graph>
      {
      public:
         typedef typename Graph::const_vertex_iterator   const_vertex_iterator;

      public:
         BnVisitor( const Bn& bn, Functor& fun ) : _bn( bn ), _func( fun )
         {}

         void run()
         {
            visit( _bn.getNetwork() );
         }

      private:
         virtual void discoverVertex( const const_vertex_iterator& it, const Graph& g )
         {
            const Factor& f = _bn.getFactors()[ it ];
            _func( f );
         }

      private:
         const Bn&   _bn;
         Functor&    _func;
      };

      class GetFactorsFunctor
      {
      public:
         void clear()
         {
            _factors.clear();
         }

         void operator()( const Factor& f )
         {
            _factors.push_back( &f );
         }

         const std::vector<const Factor*>& getFactors() const
         {
            return _factors;
         }

      private:
         std::vector<const Factor*> _factors;
      };

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
         GetFactorsFunctor functor;
         BnVisitor<GetFactorsFunctor> visitorGetFactors( bn, functor );
         visitorGetFactors.run();

         if ( functor.getFactors().size() == 0 )
            return Factor();

         std::set<ui32> varEliminationOrder;
         for ( size_t n = 0; n < functor.getFactors().size(); ++n )
         {
            std::for_each( functor.getFactors()[ n ]->getDomain().begin(),
                           functor.getFactors()[ n ]->getDomain().end(),
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
         newFactors.reserve( functor.getFactors().size() );
         for ( size_t n = 0; n < functor.getFactors().size(); ++n )
         {
            newFactors.push_back( *functor.getFactors()[ n ] );

            // check we have some evidence
            for ( ui32 evidenceVar = 0; evidenceVar < evidenceDomain.size(); ++evidenceVar )
            {
               bool isInPotential = std::binary_search( newFactors[ n ].getDomain().begin(),
                                                        newFactors[ n ].getDomain().end(),
                                                        evidenceVar );
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

         return f;


         /*
         // now enter the evidence
         std::vector<Factor> newFactors;
         newFactors.reserve( functor.getFactors().size() );
         for ( size_t n = 0; n < functor.getFactors().size(); ++n )
         {
            VectorI intersectionDomain;
            EvidenceValue  intersectionValue;
            std::set<ui32> potentialDomain( functor.getFactors()[ n ]->getDomain().begin(),
                                            functor.getFactors()[ n ]->getDomain().end() );
            computeIntersection( evidenceDomain, evidenceValue, potentialDomain, intersectionDomain, intersectionValue );
            if ( intersectionDomain.size() == 0 )
            {
               // there is no evidence, so just copy the potential
               newFactors.push_back( *functor.getFactors()[ n ] );
            } else {
               // enter the evidence
               newFactors.push_back( functor.getFactors()[ n ]->conditioning( intersectionValue, intersectionDomain ) );
            }

            std::cout << "potential=" << n << std::endl;
            newFactors.rbegin()->print( std::cout );
         }

         // finally, run the variable elimination
         VectorI domainToMarginalize( 1 );
         for ( std::set<ui32>::const_iterator it = varEliminationOrder.begin(); it != varEliminationOrder.end(); ++it )
         //for ( std::set<ui32>::const_reverse_iterator it = varEliminationOrder.rbegin(); it != varEliminationOrder.rend(); ++it )
         {
            std::vector<ui32> factorsInScope;
            std::vector<ui32> factorsNotInScope;
            getFactorsInScope( newFactors, *it, factorsInScope, factorsNotInScope );

            std::vector<Factor> factors;
            if ( factorsInScope.size() )
            {
               std::cout << "domain=" << *it << std::endl;
               domainToMarginalize[ 0 ] = *it;
               Factor tmpFactor;
               for ( std::vector<ui32>::const_iterator itInScope = factorsInScope.begin(); itInScope != factorsInScope.end(); ++itInScope )
               {
                  std::cout << "factorInScope=" << *itInScope << std::endl;
                  tmpFactor = tmpFactor * newFactors[ *itInScope ].marginalization( domainToMarginalize );
               }
               tmpFactor.print( std::cout );
               factors.push_back( tmpFactor );
            }

            for ( std::vector<ui32>::const_iterator itNotInScope = factorsNotInScope.begin(); itNotInScope != factorsNotInScope.end(); ++itNotInScope )
            {
               factors.push_back( newFactors[ *itNotInScope ] );
            }

            newFactors = factors;
         }

         ensure( newFactors.size(), "unexpected error: no remaining factors!" );
         Factor f = newFactors[ 0 ];
         for ( size_t factor = 1; factor < newFactors.size(); ++factor )
         {
            f = f * newFactors[ factor ];
         }

         return f;*/
      }


   private:
      static bool isDomainInScope( const Factor& f, ui32 domainVariable )
      {
         return std::binary_search( f.getDomain().begin(),
                                    f.getDomain().end(),
                                    domainVariable );
      }

      static void getFactorsInScope( const std::vector<Factor>& factors,
                                    ui32 domainVariable,
                                    std::vector<ui32>& indexFactorsInScope_out,
                                    std::vector<ui32>& indexFactorsNotInScope_out )
      {
         indexFactorsInScope_out.clear();

         for ( size_t factor = 0; factor < factors.size(); ++factor )
         {
            const bool isInFactor = std::binary_search( factors[ factor ].getDomain().begin(),
                                                        factors[ factor ].getDomain().end(),
                                                        domainVariable );
            if ( isDomainInScope( factors[ factor ], domainVariable ) )
            {
               indexFactorsInScope_out.push_back( factor );
            } else {
               indexFactorsNotInScope_out.push_back( factor );
            }
         }
      }

      static void computeIntersection( const VectorI& evidenceDomain,
                                       const EvidenceValue&  evidenceValue,
                                       const std::set<ui32>& potentialDomain, 
                                       VectorI& intersectionDomain,
                                       EvidenceValue&  intersectionValue )
      {
         std::vector<ui32> indexIntersection;
         for ( ui32 n = 0; n < evidenceDomain.size(); ++n )
         {
            if ( potentialDomain.find( evidenceDomain[ n ] ) != potentialDomain.end() )
            {
               indexIntersection.push_back( n );
            }
         }

         intersectionDomain = VectorI( static_cast<ui32>( indexIntersection.size() ) );
         intersectionValue = EvidenceValue( static_cast<ui32>( indexIntersection.size() ) );
         for ( size_t n = 0; n < indexIntersection.size(); ++n )
         {
            intersectionDomain[ n ] = evidenceDomain[ indexIntersection[ n ] ];
            intersectionValue[ n ] = evidenceValue[ indexIntersection[ n ] ];
         }
      }
   };
}
}


class TestGaussianBayesianInference
{
public:
   template <class InferenceAlgo>
   void testBasicInfPotentialTableImpl()
   {
      // see example from BNT http://bnt.googlecode.com/svn/trunk/docs/usage.html
      enum DomainVariable
      {
         // the order is important for efficiency reason, however for practicality this is not the best. So use PotentialTable::reorderTable
         WETGRASS,
         RAIN,
         SPRINKLER,
         CLOUDY
      };

      typedef algorithm::PotentialTable            Factor;
      typedef algorithm::BayesianNetwork<Factor>   BayesianNetwork;

      BayesianNetwork bnet;

      //
      // cloudy
      //
      Factor::VectorI domainCloudyTable( 1 );
      domainCloudyTable[ 0 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityCloudy( 1 );
      cardinalityCloudy[ 0 ] = 2;
      Factor cloudy( core::make_buffer1D<double>( 0.5, 0.5 ), domainCloudyTable, cardinalityCloudy );

      //
      // sprinkler
      //
      Factor::VectorI domainSprinklerTable( 2 );
      domainSprinklerTable[ 0 ] = (int)SPRINKLER;
      domainSprinklerTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalitySprinkler( 2 );
      cardinalitySprinkler[ 0 ] = 2;
      cardinalitySprinkler[ 1 ] = 2;
      // note: the table is encoded by domain table order starting by FF then FT, TF, TT and so on for more domain...
      Factor sprinkler( core::make_buffer1D<double>( 0.5, 0.5, 0.9, 0.1 ), domainSprinklerTable, cardinalitySprinkler );

      //
      // Rain
      //
      Factor::VectorI domainRainTable( 2 );
      domainRainTable[ 0 ] = (int)RAIN;
      domainRainTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityRain( 2 );
      cardinalityRain[ 0 ] = 2;
      cardinalityRain[ 1 ] = 2;
      Factor rain( core::make_buffer1D<double>( 0.8, 0.2, 0.2, 0.8 ), domainRainTable, cardinalityRain );

      //
      // Wet
      //
      Factor::VectorI domainWetTable( 3 );
      domainWetTable[ 0 ] = (int)WETGRASS;
      domainWetTable[ 1 ] = (int)RAIN;
      domainWetTable[ 2 ] = (int)SPRINKLER;
      Factor::VectorI  cardinalityWet( 3 );
      cardinalityWet[ 0 ] = 2;
      cardinalityWet[ 1 ] = 2;
      cardinalityWet[ 2 ] = 2;
      Factor wet( core::make_buffer1D<double>( 1, 0, 0.1, 0.9, 0.1, 0.9, 0.01, 0.99 ), domainWetTable, cardinalityWet );

      //
      // create network
      //
      BayesianNetwork::NodeDescritor cloudyNode    = bnet.addNode( "CLOUDY",     cloudy );
      BayesianNetwork::NodeDescritor wetNode       = bnet.addNode( "WET",        wet );
      BayesianNetwork::NodeDescritor sprinklerNode = bnet.addNode( "SPRINKLER",  sprinkler );
      BayesianNetwork::NodeDescritor rainNode      = bnet.addNode( "RAIN",       rain );

      bnet.addLink( cloudyNode, sprinklerNode );
      bnet.addLink( cloudyNode, rainNode );
      bnet.addLink( sprinklerNode, wetNode );
      bnet.addLink( rainNode, wetNode );

      //
      // inference test. Checked against Matlab BNT
      //

      InferenceAlgo inference;
      Factor query1 = inference.run( bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query1.getDomain().size() == 1 );
      TESTER_ASSERT( query1.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query1.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query1.getTable()[ 0 ], 0.5702, 1e-2 ) );
      TESTER_ASSERT( core::equal( query1.getTable()[ 1 ], 0.4298, 1e-2 ) );

      Factor query2 = inference.run( bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 0 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query2.getDomain().size() == 1 );
      TESTER_ASSERT( query2.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query2.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query2.getTable()[ 0 ], 0.9379, 1e-2 ) );
      TESTER_ASSERT( core::equal( query2.getTable()[ 1 ], 0.0621, 1e-2 ) );
      
      Factor query3 = inference.run( bnet, core::make_buffer1D<ui32>( (int)WETGRASS, (int)CLOUDY ), core::make_buffer1D<ui32>( 1, 1 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query3.getDomain().size() == 1 );
      TESTER_ASSERT( query3.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query3.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query3.getTable()[ 0 ], 0.8696, 1e-2 ) );
      TESTER_ASSERT( core::equal( query3.getTable()[ 1 ], 0.1304, 1e-2 ) );

      Factor query4 = inference.run( bnet, core::make_buffer1D<ui32>( (int)WETGRASS, (int)CLOUDY ), core::make_buffer1D<ui32>( 1, 0 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query4.getDomain().size() == 1 );
      TESTER_ASSERT( query4.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query4.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query4.getTable()[ 0 ], 0.1639, 1e-2 ) );
      TESTER_ASSERT( core::equal( query4.getTable()[ 1 ], 0.8361, 1e-2 ) );

      Factor query5 = inference.run( bnet, core::Buffer1D<ui32>(), Buffer1D<ui32>(), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query5.getDomain().size() == 1 );
      TESTER_ASSERT( query5.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query5.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query5.getTable()[ 0 ], 0.7, 1e-2 ) );
      TESTER_ASSERT( core::equal( query5.getTable()[ 1 ], 0.3, 1e-2 ) );

      Factor query6 = inference.run( bnet, core::Buffer1D<ui32>(), Buffer1D<ui32>(), core::make_buffer1D<ui32>( (int)CLOUDY ) );
      TESTER_ASSERT( query6.getDomain().size() == 1 );
      TESTER_ASSERT( query6.getDomain()[ 0 ] == CLOUDY );
      TESTER_ASSERT( query6.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query6.getTable()[ 0 ], 0.5, 1e-2 ) );
      TESTER_ASSERT( core::equal( query6.getTable()[ 1 ], 0.5, 1e-2 ) );

      Factor query7 = inference.run( bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)CLOUDY ) );
      TESTER_ASSERT( query7.getDomain().size() == 1 );
      TESTER_ASSERT( query7.getDomain()[ 0 ] == CLOUDY );
      TESTER_ASSERT( query7.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query7.getTable()[ 0 ], 0.4242, 1e-2 ) );
      TESTER_ASSERT( core::equal( query7.getTable()[ 1 ], 0.5758, 1e-2 ) );
   }

   void testBasicInfPotentialTable()
   {
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceNaive<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testBasicInfPotentialTable );
TESTER_TEST_SUITE_END();
#endif