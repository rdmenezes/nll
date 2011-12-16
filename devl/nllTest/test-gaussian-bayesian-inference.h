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
      BayesianNetwork() : _names( _network ), _Factors( _network )
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
      typedef algorithm::PotentialGaussianCanonical   Factor;
      typedef algorithm::BayesianNetwork BayesianNetwork;

      BayesianNetwork bnet;
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testBasicInf1 );
TESTER_TEST_SUITE_END();
#endif