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
}
}


class TestGaussianBayesianInference
{
public:
   enum DomainVariable
   {
      // the order is important for efficiency reason, however for practicality this is not the best. So use PotentialTable::reorderTable
      WETGRASS = 50,
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

   static std::auto_ptr<BayesianNetwork> buildSprinklerNet2()
   {
      std::auto_ptr<BayesianNetwork> bnet( new BayesianNetwork() );

      //
      // cloudy
      //
      Factor::VectorI domainCloudyTable( 1 );
      domainCloudyTable[ 0 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityCloudy( 1 );
      cardinalityCloudy[ 0 ] = 3;
      Factor cloudy( core::make_buffer1D<double>( 0.2, 0.5, 0.3 ), domainCloudyTable, cardinalityCloudy );

      //
      // sprinkler
      //
      Factor::VectorI domainSprinklerTable( 2 );
      domainSprinklerTable[ 0 ] = (int)SPRINKLER;
      domainSprinklerTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalitySprinkler( 2 );
      cardinalitySprinkler[ 0 ] = 2;
      cardinalitySprinkler[ 1 ] = 3;
      // note: the table is encoded by domain table order starting by FF then FT, TF, TT and so on for more domain...
      Factor sprinkler( core::make_buffer1D<double>( 0.5, 0.5, 0.7, 0.3, 0.9, 0.1 ), domainSprinklerTable, cardinalitySprinkler );

      //
      // Rain
      //
      Factor::VectorI domainRainTable( 2 );
      domainRainTable[ 0 ] = (int)RAIN;
      domainRainTable[ 1 ] = (int)CLOUDY;
      Factor::VectorI  cardinalityRain( 2 );
      cardinalityRain[ 0 ] = 2;
      cardinalityRain[ 1 ] = 3;
      Factor rain( core::make_buffer1D<double>( 0.8, 0.2, 0.4, 0.6, 0.2, 0.8 ), domainRainTable, cardinalityRain );

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

   template <class InferenceAlgo>
   void testBasicInfPotentialTableImpl2()
   {
      // see example from BNT http://bnt.googlecode.com/svn/trunk/docs/usage.html
      

      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet2();

     
      //
      // inference test. Checked against Matlab BNT
      //
      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<ui32>( (int)WETGRASS ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)SPRINKLER ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == SPRINKLER );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.6032, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.3968, 1e-2 ) );
      }

      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<ui32>( (int)CLOUDY ), core::make_buffer1D<ui32>( 1 ), core::make_buffer1D<ui32>( (int)WETGRASS ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.3358, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.6642, 1e-2 ) );
      }

      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<ui32>( (int)CLOUDY ), core::make_buffer1D<ui32>( 2 ), core::make_buffer1D<ui32>( (int)WETGRASS ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.2548, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.7452, 1e-2 ) );
      }
      
      
      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<ui32>( (int)CLOUDY ), core::make_buffer1D<ui32>( 2 ), core::make_buffer1D<ui32>( (int)WETGRASS, (int)SPRINKLER ) );
         TESTER_ASSERT( query.getDomain().size() == 2 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getDomain()[ 1 ] == SPRINKLER );
         TESTER_ASSERT( query.getTable().size() == 4 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.2520, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.6480, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 2 ], 0.0028, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 3 ], 0.0972, 1e-2 ) );
      }

      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::Buffer1D<ui32>(), core::Buffer1D<ui32>(), core::make_buffer1D<ui32>( (int)WETGRASS, (int)SPRINKLER ) );
         TESTER_ASSERT( query.getDomain().size() == 2 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getDomain()[ 1 ] == SPRINKLER );
         TESTER_ASSERT( query.getTable().size() == 4 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.3186, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.4014, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 2 ], 0.0159, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 3 ], 0.2641, 1e-2 ) );
      }
   }

   void testBasicInfPotentialTable()
   {
      
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceVariableElimination<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceNaive<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
      
      testBasicInfPotentialTableImpl2<algorithm::BayesianInferenceVariableElimination<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
      testBasicInfPotentialTableImpl2<algorithm::BayesianInferenceNaive<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
   }

   void testPotentialTableMlParametersEstimation()
   {
      typedef algorithm::BayesianNetwork<algorithm::PotentialTable> BayesianNetwork;

      // check against BNT results
      std::vector< std::vector< double > > file1 = core::readVectorFromMatlabAsColumn( NLL_TEST_PATH "data/bnt/bnt_samples_sprinkler.txt");

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
      mlEstimator.compute( core::make_buffer1D<ui32>( (int)CLOUDY, (int)SPRINKLER, (int)RAIN, (int)WETGRASS ), samples, *bnet );

      std::vector<const BayesianNetwork::Factor*> factors;
      algorithm::getFactors( *bnet, factors );
      for ( ui32 n = 0; n < factors.size(); ++n )
      {
         factors[ n ]->print( std::cout );
      }

      // compare against BNT
      TESTER_ASSERT( factors.size() == 4 );
      TESTER_ASSERT( factors[ 0 ]->getDomain().size() == 1 );
      TESTER_ASSERT( factors[ 0 ]->getDomain()[ 0 ] == (int)CLOUDY );
      TESTER_ASSERT( factors[ 0 ]->getTable().size() == 2 );
      TESTER_ASSERT( core::equal( factors[ 0 ]->getTable()[ 0 ], 0.46, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 0 ]->getTable()[ 1 ], 0.53, 0.01 ) );

      TESTER_ASSERT( factors[ 1 ]->getDomain().size() == 2 );
      TESTER_ASSERT( factors[ 1 ]->getDomain()[ 0 ] == (int)SPRINKLER );
      TESTER_ASSERT( factors[ 1 ]->getDomain()[ 1 ] == (int)CLOUDY );
      TESTER_ASSERT( factors[ 1 ]->getTable().size() == 4 );
      TESTER_ASSERT( core::equal( factors[ 1 ]->getTable()[ 0 ], 0.5, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 1 ]->getTable()[ 1 ], 0.5, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 1 ]->getTable()[ 2 ], 0.9375, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 1 ]->getTable()[ 3 ], 0.0625, 0.01 ) );

      TESTER_ASSERT( factors[ 2 ]->getDomain().size() == 2 );
      TESTER_ASSERT( factors[ 2 ]->getDomain()[ 0 ] == (int)RAIN );
      TESTER_ASSERT( factors[ 2 ]->getDomain()[ 1 ] == (int)CLOUDY );
      TESTER_ASSERT( factors[ 2 ]->getTable().size() == 4 );
      TESTER_ASSERT( core::equal( factors[ 2 ]->getTable()[ 0 ], 0.71, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 2 ]->getTable()[ 1 ], 0.28, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 2 ]->getTable()[ 2 ], 0.18, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 2 ]->getTable()[ 3 ], 0.81, 0.01 ) );

      TESTER_ASSERT( factors[ 3 ]->getDomain().size() == 3 );
      TESTER_ASSERT( factors[ 3 ]->getDomain()[ 0 ] == (int)WETGRASS );
      TESTER_ASSERT( factors[ 3 ]->getDomain()[ 1 ] == (int)RAIN );
      TESTER_ASSERT( factors[ 3 ]->getDomain()[ 2 ] == (int)SPRINKLER );
      TESTER_ASSERT( factors[ 3 ]->getTable().size() == 8 );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 0 ], 1.0, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 1 ], 0.0, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 2 ], 0.076, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 3 ], 0.923, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 4 ], 0.25, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 5 ], 0.75, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 6 ], 0.0, 0.01 ) );
      TESTER_ASSERT( core::equal( factors[ 3 ]->getTable()[ 7 ], 1.0, 0.01 ) );
   }

   void testBnPotentialSampling()
   {
      srand( 100 );
      typedef algorithm::BayesianNetwork<algorithm::PotentialTable> BayesianNetwork;
      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet();

      typedef algorithm::BayesianNetworkSampling<BayesianNetwork> Sampler;
      Sampler bnSampler;
      std::vector<Sampler::EvidenceValue> samples;
      Sampler::VectorI domainOutput;
      bnSampler.compute( *bnet, 10000, samples, domainOutput );

      // now reestimate the BN and compare to true values
      algorithm::BayesianNetworkMaximumLikelihoodParameterEstimation<BayesianNetwork> mlEstimator;
      mlEstimator.compute( domainOutput, samples, *bnet );

      std::vector<const BayesianNetwork::Factor*> factorsFound;
      algorithm::getFactors( *bnet, factorsFound );
      for ( ui32 n = 0; n < factorsFound.size(); ++n )
      {
         factorsFound[ n ]->print( std::cout );
      }

      std::auto_ptr<BayesianNetwork> bnetRef = buildSprinklerNet();
      std::vector<const BayesianNetwork::Factor*> factorsRef;
      algorithm::getFactors( *bnetRef, factorsRef );

      const double EPSILON = 1e-2;
      for ( ui32 n = 0; n < factorsRef.size(); ++n )
      {
         for ( ui32 nn = 0; nn < factorsRef[ n ]->getTable().size(); ++nn )
         {
            TESTER_ASSERT( core::equal( factorsRef[ n ]->getTable()[ nn ],
                                        factorsFound[ n ]->getTable()[ nn ],
                                        EPSILON ) );
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testPotentialTableMlParametersEstimation );
TESTER_TEST( testBasicInfPotentialTable );
TESTER_TEST( testBnPotentialSampling );
TESTER_TEST_SUITE_END();
#endif