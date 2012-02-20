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
   template <class BayesianNetworkT>
   class BayesianNetworkMaximumLikelihoodParameterEstimation
   {
   public:
      typedef BayesianNetworkT                     Network;
      typedef typename BayesianNetworkT::Factor    Factor;
      typedef typename Factor::EvidenceValue       EvidenceValue;
      
      /**
       @brief Computes the ML estimates of the BN network given <fullyObservedData>, a fully observed dataset
       */
      void compute( const std::vector<EvidenceValue>& fullyObservedData )
      {
         ensure( 0, "This method is not implemented for this kind of network", Network& outMlEstimation );
      }
   };

   template <>
   class BayesianNetworkMaximumLikelihoodParameterEstimation< BayesianNetwork< PotentialTable > >
   {
   public:
      typedef BayesianNetwork< PotentialTable >   Network;
      typedef Network::Factor                     Factor;
      typedef Factor::EvidenceValue               EvidenceValue;
      
      /**
       @brief Computes the ML estimates of the BN network given <fullyObservedData>, a fully observed dataset
       */
      void compute( const std::vector<EvidenceValue>& fullyObservedData, Network& outMlEstimation )
      {
         std::vector<Factor*> factors;
         getFactors( outMlEstimation, factors );
      }
   };
}
}


class TestGaussianBayesianInference
{
public:
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

   static std::auto_ptr<BayesianNetwork> buildSprinklerNet()
   {
      std::auto_ptr<BayesianNetwork> bnet( new BayesianNetwork() );

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
      BayesianNetwork::NodeDescritor cloudyNode    = bnet->addNode( "CLOUDY",     cloudy );
      BayesianNetwork::NodeDescritor wetNode       = bnet->addNode( "WET",        wet );
      BayesianNetwork::NodeDescritor sprinklerNode = bnet->addNode( "SPRINKLER",  sprinkler );
      BayesianNetwork::NodeDescritor rainNode      = bnet->addNode( "RAIN",       rain );

      bnet->addLink( cloudyNode, sprinklerNode );
      bnet->addLink( cloudyNode, rainNode );
      bnet->addLink( sprinklerNode, wetNode );
      bnet->addLink( rainNode, wetNode );

      return bnet;
   }

   template <class InferenceAlgo>
   void testBasicInfPotentialTableImpl()
   {
      // see example from BNT http://bnt.googlecode.com/svn/trunk/docs/usage.html
      

      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet();


      //
      // inference test. Checked against Matlab BNT
      //

      InferenceAlgo inference;
      Factor query1 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query1.getDomain().size() == 1 );
      TESTER_ASSERT( query1.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query1.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query1.getTable()[ 0 ], 0.5702, 1e-2 ) );
      TESTER_ASSERT( core::equal( query1.getTable()[ 1 ], 0.4298, 1e-2 ) );

      Factor query2 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 0 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query2.getDomain().size() == 1 );
      TESTER_ASSERT( query2.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query2.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query2.getTable()[ 0 ], 0.9379, 1e-2 ) );
      TESTER_ASSERT( core::equal( query2.getTable()[ 1 ], 0.0621, 1e-2 ) );
      
      Factor query3 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS, (int)CLOUDY ), core::make_buffer1D<ui32>( 1, 1 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query3.getDomain().size() == 1 );
      TESTER_ASSERT( query3.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query3.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query3.getTable()[ 0 ], 0.8696, 1e-2 ) );
      TESTER_ASSERT( core::equal( query3.getTable()[ 1 ], 0.1304, 1e-2 ) );

      Factor query4 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS, (int)CLOUDY ), core::make_buffer1D<ui32>( 1, 0 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query4.getDomain().size() == 1 );
      TESTER_ASSERT( query4.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query4.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query4.getTable()[ 0 ], 0.1639, 1e-2 ) );
      TESTER_ASSERT( core::equal( query4.getTable()[ 1 ], 0.8361, 1e-2 ) );

      Factor query5 = inference.run( *bnet, core::Buffer1D<ui32>(), Buffer1D<ui32>(), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
      TESTER_ASSERT( query5.getDomain().size() == 1 );
      TESTER_ASSERT( query5.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query5.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query5.getTable()[ 0 ], 0.7, 1e-2 ) );
      TESTER_ASSERT( core::equal( query5.getTable()[ 1 ], 0.3, 1e-2 ) );

      Factor query6 = inference.run( *bnet, core::Buffer1D<ui32>(), Buffer1D<ui32>(), core::make_buffer1D<ui32>( (int)CLOUDY ) );
      TESTER_ASSERT( query6.getDomain().size() == 1 );
      TESTER_ASSERT( query6.getDomain()[ 0 ] == CLOUDY );
      TESTER_ASSERT( query6.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query6.getTable()[ 0 ], 0.5, 1e-2 ) );
      TESTER_ASSERT( core::equal( query6.getTable()[ 1 ], 0.5, 1e-2 ) );

      Factor query7 = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)CLOUDY ) );
      TESTER_ASSERT( query7.getDomain().size() == 1 );
      TESTER_ASSERT( query7.getDomain()[ 0 ] == CLOUDY );
      TESTER_ASSERT( query7.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query7.getTable()[ 0 ], 0.4242, 1e-2 ) );
      TESTER_ASSERT( core::equal( query7.getTable()[ 1 ], 0.5758, 1e-2 ) );
   }

   void testBasicInfPotentialTable()
   {
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceVariableElimination<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceNaive<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
   }

   void testPotentialTableMlParametersEstimation()
   {
      typedef algorithm::BayesianNetwork<algorithm::PotentialTable> BayesianNetwork;

      // check against BNT results
      std::vector< std::vector< double > > file1 = core::readVectorFromMatlabAsColumn("c:/tmp/f.txt");

      // reorganize the data
      std::vector< BayesianNetwork::Factor::EvidenceValue > samples;
      for ( size_t id = 0; id < file1.size(); ++id )
      {
         std::vector< double >& sample = file1[ id ];
         BayesianNetwork::Factor::EvidenceValue newSample( file1.size() );
         for ( size_t n = 0; n < sample.size(); ++n )
         {
            newSample[ n ] = static_cast<ui32>( sample[ n ] - 1 );
         }
         samples.push_back( newSample );
      }

      // do some ML estimation
      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet();
      algorithm::BayesianNetworkMaximumLikelihoodParameterEstimation<BayesianNetwork> mlEstimator;
      mlEstimator.compute( samples, *bnet );
      std::cout << "a" << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testPotentialTableMlParametersEstimation );
TESTER_TEST( testBasicInfPotentialTable );
TESTER_TEST_SUITE_END();
#endif