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

   private:
      typedef core::GraphAdgencyList<> Graph;
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
         _Factors[ desc ] = factor;
         return desc;
      }

      void addLink( const NodeDescritor& source, const NodeDescritor& destination )
      {
         _network.addEdge( source, destination );
      }

   private:
      Graph                   _network;
      NodeNameMapper          _names;
      NodeFactorMapper        _factors;
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
      Factor Rain( core::make_buffer1D<double>( 0.8, 0.2, 0.2, 0.8 ), domainRainTable, cardinalityRain );

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
      Factor Wet( core::make_buffer1D<double>( 1, 0, 0.1, 0.9, 0.1, 0.9, 0.01, 0.99 ), domainWetTable, cardinalityWet );
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testBasicInf1 );
TESTER_TEST_SUITE_END();
#endif