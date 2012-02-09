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

      const NodeNameMapper& getNames() const
      {
         return _names;
      }

   private:
      Graph                   _network;
      NodeNameMapper          _names;
      NodeFactorMapper        _factors;
   };

   /**
    @brief computes p(U-E, E=e) with E observed variables (evidence) and U-E the rest of the variables modeled by the BN

    Given U = all the model's variables,
    it computes likelyhood queries: p(U-E, E=e) = (Sum_(y in U-E) p(Y=y, E=e))
    */
   template <class Factor>
   class BayesianInferenceVariableElimination
   {
   public:
      typedef PotentialGaussianMoment::Vector         Vector;
      typedef PotentialGaussianMoment::VectorI        VectorI;
      typedef typename Factor::EvidenceValue          EvidenceValue;
      typedef BayesianNetwork<Factor>                 Bn;
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
       */
      Factor run( const BayesianNetwork<Factor>& bn, 
                  const VectorI& evidenceDomain,
                  const EvidenceValue& evidenceValue ) const
      {
         GetFactorsFunctor functor;
         BnVisitor<GetFactorsFunctor> visitorGetFactors( bn, functor );
         visitorGetFactors.run();

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

         for ( ui32 n = 0; n < evidenceDomain.size(); ++n )
         {
            // remove the evidence variable from the variable to eliminate
            varEliminationOrder.erase( evidenceDomain[ n ] );
         }

         // now enter the evidence
         std::vector<Factor> newFactors;
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
               functor.getFactors()[ n ]->conditioning( intersectionValue, intersectionDomain );
            }
         }

         // finally, run the variable elimination
         for ( std::set<ui32>::const_iterator it = varEliminationOrder.begin(); it != varEliminationOrder.end(); ++it )
         {
         }


         std::cout << "F=" << functor.getFactors().size() << std::endl;
         return Factor();
      }


   private:
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
   void testBasicInf1()
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
      // inference test
      //
      algorithm::BayesianInferenceVariableElimination<Factor> inference;
      inference.run( bnet, core::make_buffer1D<ui32>( (int)RAIN ), core::make_buffer1D<ui32>( 1 ) );
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testBasicInf1 );
TESTER_TEST_SUITE_END();
#endif