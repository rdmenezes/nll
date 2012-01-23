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
         CLOUDY,
         SPRINKLER,
         RAIN,
         WETGRASS
      };

      typedef algorithm::PotentialTable            Factor;
      typedef algorithm::BayesianNetwork<Factor>   BayesianNetwork;

      BayesianNetwork bnet;

      Factor::VectorI domainCloudyTable( 1 );
      domainCloudyTable[ 0 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityCloudy( 1 );
      cardinalityCloudy[ 0 ] = 2;
      Factor cloudy( core::make_buffer1D<double>( 0.5, 0.5 ), domainCloudyTable, cardinalityCloudy );
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testBasicInf1 );
TESTER_TEST_SUITE_END();
#endif