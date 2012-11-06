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
    @brief Traits to instanciate a Bayesian potential

    In the general case, instanciating a potential is not required. However for Linear Gaussian potential for example, each node must be transformed
    as a Gaussian canonical potential with all the operations associated with potentials
    */
   template <>
   struct TraitsInstanciateBayesianPotential<PotentialLinearGaussian>
   {
      typedef PotentialGaussianCanonical PotentialInstanciationType;

      static PotentialInstanciationType create( const PotentialLinearGaussian& potential )
      {
         // default behaviour: just copy the potential
         return potential.toGaussianCanonical();
      }
   };
}

namespace core
{
   template <class Function>
   struct FunctorCreator
   {
   };
}
}

/**
 @note BNT alpha or g values are not following the same conventions and so are not matching alpha_BNT = alpha / gaussian_normalization_constant
 */
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

   typedef algorithm::PotentialTable               Factor;
   typedef algorithm::BayesianNetwork<Factor>      BayesianNetwork;
   typedef algorithm::PotentialLinearGaussian      Factorg;
   typedef algorithm::BayesianNetwork<Factorg>     BayesianNetworkg;

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
      Factor query1 = inference.run( *bnet, core::make_buffer1D<size_t>( (int)WETGRASS ), core::make_buffer1D<size_t>( 1 ), core::make_buffer1D<size_t>( (int)SPRINKLER ) );
      TESTER_ASSERT( query1.getDomain().size() == 1 );
      TESTER_ASSERT( query1.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query1.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query1.getTable()[ 0 ], 0.5702, 1e-2 ) );
      TESTER_ASSERT( core::equal( query1.getTable()[ 1 ], 0.4298, 1e-2 ) );

      Factor query2 = inference.run( *bnet, core::make_buffer1D<size_t>( (int)WETGRASS ), core::make_buffer1D<size_t>( 0 ), core::make_buffer1D<size_t>( (int)SPRINKLER ) );
      TESTER_ASSERT( query2.getDomain().size() == 1 );
      TESTER_ASSERT( query2.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query2.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query2.getTable()[ 0 ], 0.9379, 1e-2 ) );
      TESTER_ASSERT( core::equal( query2.getTable()[ 1 ], 0.0621, 1e-2 ) );
      
      Factor query3 = inference.run( *bnet, core::make_buffer1D<size_t>( (int)WETGRASS, (int)CLOUDY ), core::make_buffer1D<size_t>( 1, 1 ), core::make_buffer1D<size_t>( (int)SPRINKLER ) );
      TESTER_ASSERT( query3.getDomain().size() == 1 );
      TESTER_ASSERT( query3.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query3.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query3.getTable()[ 0 ], 0.8696, 1e-2 ) );
      TESTER_ASSERT( core::equal( query3.getTable()[ 1 ], 0.1304, 1e-2 ) );

      Factor query4 = inference.run( *bnet, core::make_buffer1D<size_t>( (int)WETGRASS, (int)CLOUDY ), core::make_buffer1D<size_t>( 1, 0 ), core::make_buffer1D<size_t>( (int)SPRINKLER ) );
      TESTER_ASSERT( query4.getDomain().size() == 1 );
      TESTER_ASSERT( query4.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query4.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query4.getTable()[ 0 ], 0.1639, 1e-2 ) );
      TESTER_ASSERT( core::equal( query4.getTable()[ 1 ], 0.8361, 1e-2 ) );

      Factor query5 = inference.run( *bnet, core::Buffer1D<size_t>(), Buffer1D<size_t>(), core::make_buffer1D<size_t>( (int)SPRINKLER ) );
      TESTER_ASSERT( query5.getDomain().size() == 1 );
      TESTER_ASSERT( query5.getDomain()[ 0 ] == SPRINKLER );
      TESTER_ASSERT( query5.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query5.getTable()[ 0 ], 0.7, 1e-2 ) );
      TESTER_ASSERT( core::equal( query5.getTable()[ 1 ], 0.3, 1e-2 ) );

      Factor query6 = inference.run( *bnet, core::Buffer1D<size_t>(), Buffer1D<size_t>(), core::make_buffer1D<size_t>( (int)CLOUDY ) );
      TESTER_ASSERT( query6.getDomain().size() == 1 );
      TESTER_ASSERT( query6.getDomain()[ 0 ] == CLOUDY );
      TESTER_ASSERT( query6.getTable().size() == 2 );
      TESTER_ASSERT( core::equal( query6.getTable()[ 0 ], 0.5, 1e-2 ) );
      TESTER_ASSERT( core::equal( query6.getTable()[ 1 ], 0.5, 1e-2 ) );

      Factor query7 = inference.run( *bnet, core::make_buffer1D<size_t>( (int)WETGRASS ), core::make_buffer1D<size_t>( 1 ), core::make_buffer1D<size_t>( (int)CLOUDY ) );
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
         Factor query = inference.run( *bnet, core::make_buffer1D<size_t>( (int)WETGRASS ), core::make_buffer1D<size_t>( 1 ), core::make_buffer1D<size_t>( (int)SPRINKLER ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == SPRINKLER );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.6032, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.3968, 1e-2 ) );
      }

      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<size_t>( (int)CLOUDY ), core::make_buffer1D<size_t>( 1 ), core::make_buffer1D<size_t>( (int)WETGRASS ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.3358, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.6642, 1e-2 ) );
      }

      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<size_t>( (int)CLOUDY ), core::make_buffer1D<size_t>( 2 ), core::make_buffer1D<size_t>( (int)WETGRASS ) );
         TESTER_ASSERT( query.getDomain().size() == 1 );
         TESTER_ASSERT( query.getDomain()[ 0 ] == WETGRASS );
         TESTER_ASSERT( query.getTable().size() == 2 );
         TESTER_ASSERT( core::equal( query.getTable()[ 0 ], 0.2548, 1e-2 ) );
         TESTER_ASSERT( core::equal( query.getTable()[ 1 ], 0.7452, 1e-2 ) );
      }
      
      
      {
         InferenceAlgo inference;
         Factor query = inference.run( *bnet, core::make_buffer1D<size_t>( (int)CLOUDY ), core::make_buffer1D<size_t>( 2 ), core::make_buffer1D<size_t>( (int)WETGRASS, (int)SPRINKLER ) );
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
         Factor query = inference.run( *bnet, core::Buffer1D<size_t>(), core::Buffer1D<size_t>(), core::make_buffer1D<size_t>( (int)WETGRASS, (int)SPRINKLER ) );
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
         BayesianNetwork::Factor::EvidenceValue newSample( (size_t)file1.size() );
         for ( size_t n = 0; n < sample.size(); ++n )
         {
            newSample[ (size_t)n ] = static_cast<size_t>( sample[ (size_t)n ] - 1 );
         }
         samples.push_back( newSample );
      }

      // do some ML estimation
      std::auto_ptr<BayesianNetwork> bnet = buildSprinklerNet();
      algorithm::BayesianNetworkMaximumLikelihoodParameterEstimation<BayesianNetwork> mlEstimator;
      mlEstimator.compute( core::make_buffer1D<size_t>( (int)CLOUDY, (int)SPRINKLER, (int)RAIN, (int)WETGRASS ), samples, *bnet );

      std::vector<const BayesianNetwork::Factor*> factors;
      algorithm::getFactors( *bnet, factors );
      for ( size_t n = 0; n < factors.size(); ++n )
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
      for ( size_t n = 0; n < factorsFound.size(); ++n )
      {
         factorsFound[ n ]->print( std::cout );
      }

      std::auto_ptr<BayesianNetwork> bnetRef = buildSprinklerNet();
      std::vector<const BayesianNetwork::Factor*> factorsRef;
      algorithm::getFactors( *bnetRef, factorsRef );

      const double EPSILON = 1e-2;
      for ( size_t n = 0; n < factorsRef.size(); ++n )
      {
         for ( size_t nn = 0; nn < factorsRef[ n ]->getTable().size(); ++nn )
         {
            TESTER_ASSERT( core::equal( factorsRef[ n ]->getTable()[ nn ],
                                        factorsFound[ n ]->getTable()[ nn ],
                                        EPSILON ) );
         }
      }
   }


   static std::auto_ptr<BayesianNetworkg> buildGaussianBn1()
   {
      typedef algorithm::PotentialGaussianMoment FactorMoment;
      std::auto_ptr<BayesianNetworkg> bnet( new BayesianNetworkg() );

      //
      // cloudy
      //
      FactorMoment::VectorI domainC( 1 );
      domainC[ 0 ] = (int)CLOUDY;
      FactorMoment::Vector meanC( 1 );
      meanC[ 0 ] = 15;
      FactorMoment::Matrix covC( 1, 1 );
      covC( 0, 0 ) = 0.5;
      FactorMoment cF( meanC, covC, domainC );


      FactorMoment::VectorI domainR( 1 );
      domainR[ 0 ] = (int)RAIN;
      FactorMoment::Vector meanR( 1 );
      meanR[ 0 ] = 50;
      FactorMoment::Matrix covR( 1, 1 );
      covR( 0, 0 ) = 0.1;
      FactorMoment rF( meanR, covR, domainR );

      FactorMoment::VectorI domainS( 3 );
      domainS[ 0 ] = (int)SPRINKLER;
      domainS[ 1 ] = (int)CLOUDY;
      domainS[ 2 ] = (int)RAIN;
      FactorMoment::Vector meanS( 1 );
      meanR[ 0 ] = 30;
      FactorMoment::Matrix covS( 1, 1 );
      covR( 0, 0 ) = 3;
      FactorMoment rS( meanS, covS, domainS );

      /*
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
      */
      return bnet;
   }

   void testInferenceGaussianBn()
   {
      std::auto_ptr<BayesianNetworkg> bnet = buildGaussianBn1();

      std::cout << "ok" << std::endl;
   }

   template<typename T>
   struct EmptyDeleter
   {
       void operator() (T* )
       {
          // do nothing!
       }
   };

   void testLinearGaussian1()
   {
      //
      // Compared against BNT
      // C -
      //    -> S
      // R -
      typedef algorithm::PotentialLinearGaussian   Potential;
      enum
      {
         S, C, R
      };

      Potential::Vector meanR = core::make_buffer1D<double>( 50 );
      Potential::Matrix covR( 1, 1 ); covR[ 0 ] = 0.1;
      Potential::VectorI idR = core::make_buffer1D<size_t>( (int)R );
      Potential potR( meanR, covR, idR );

      Potential::Vector meanC = core::make_buffer1D<double>( 15 );
      Potential::Matrix covC( 1, 1 ); covC[ 0 ] = 0.5;
      Potential::VectorI idC = core::make_buffer1D<size_t>( (int)C );
      Potential potC( meanC, covC, idC );

      Potential::Vector meanS = core::make_buffer1D<double>( 30 );
      Potential::Matrix covS( 1, 1 ); covS[ 0 ] = 3;
      Potential::VectorI idS = core::make_buffer1D<size_t>( (int)S );
      Potential::Vector wS = core::make_buffer1D<double>( 1, 2 );
      
      std::vector<Potential::Dependency> dpsS;
      dpsS.push_back( Potential::Dependency( std::shared_ptr<Potential>( &potR, EmptyDeleter<Potential>() ), wS[ 0 ] ) );
      dpsS.push_back( Potential::Dependency( std::shared_ptr<Potential>( &potC, EmptyDeleter<Potential>() ), wS[ 1 ] ) );
      Potential potS( meanS, covS, idS, dpsS );

      algorithm::PotentialGaussianCanonical potr = potR.toGaussianCanonical();
      algorithm::PotentialGaussianCanonical pots = potS.toGaussianCanonical();
      algorithm::PotentialGaussianCanonical potc = potC.toGaussianCanonical();

      algorithm::PotentialGaussianMoment result = (potr * potc * pots).toGaussianMoment();

      result.print( std::cout );

      // validated against matlab
      TESTER_ASSERT( core::equal<double>( result.getMean()[ S ], 145 ) );
      TESTER_ASSERT( core::equal<double>( result.getMean()[ C ], 15 ) );
      TESTER_ASSERT( core::equal<double>( result.getMean()[ R ], 50 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( S, S ), 3.9, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( R, R ), 0.1, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( C, C ), 0.5, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( S, R ), 0.2, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( R, S ), 0.2, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( S, C ), 0.5, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( C, S ), 0.5, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( R, C ), 0, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( C, R ), 0, 1e-6 ) );

      // the result must be normalized
      const double alpha = result.getAlpha();
      result.normalizeGaussian();
      TESTER_ASSERT( core::equal<double>( alpha, result.getAlpha(), 1e-6 ) );
   }

   void testLinearGaussian2()
   {
      typedef algorithm::PotentialLinearGaussian               Potential;
      typedef algorithm::PotentialLinearGaussian::Dependency   Dependency;
      typedef algorithm::PotentialLinearGaussian::Dependencies   Dependencies;

      //
      // test a network x1 -> x2 -> x3
      //
      Potential::Vector meanX1 = core::make_buffer1D<double>( 1 );
      Potential::Matrix covX1( 1, 1 ); covX1[ 0 ] = 4;
      Potential::VectorI idX1 = core::make_buffer1D<size_t>( 3 );
      Potential potX1( meanX1, covX1, idX1 );

      Dependency dependencyX2( std::shared_ptr<Potential>( &potX1, EmptyDeleter<Potential>() ), 0.5 );
      Dependencies dependenciesX2;
      dependenciesX2.push_back( dependencyX2 );
      Potential::Vector meanX2 = core::make_buffer1D<double>( -3.5 );
      Potential::Matrix covX2( 1, 1 ); covX2[ 0 ] = 4;
      Potential::VectorI idX2 = core::make_buffer1D<size_t>( 2 );
      Potential potX2( meanX2, covX2, idX2, dependenciesX2 );

      Dependency dependencyX3( std::shared_ptr<Potential>( &potX2, EmptyDeleter<Potential>() ), -1 );
      Dependencies dependenciesX3;
      dependenciesX3.push_back( dependencyX3 );
      Potential::Vector meanX3 = core::make_buffer1D<double>( 1 );
      Potential::Matrix covX3( 1, 1 ); covX3[ 0 ] = 3;
      Potential::VectorI idX3 = core::make_buffer1D<size_t>( 1 );
      Potential potX3( meanX3, covX3, idX3, dependenciesX3 );

      algorithm::PotentialGaussianMoment pot = (potX1.toGaussianCanonical() * potX2.toGaussianCanonical() * potX3.toGaussianCanonical()).toGaussianMoment();
      pot.print( std::cout );


      // checked againtst: Probabilistic graphical model, daphne koller, p252

      TESTER_ASSERT( core::equal<double>( pot.getMean()[ 0 ], 4, 1e-5 ) );
      TESTER_ASSERT( core::equal<double>( pot.getMean()[ 1 ], -3, 1e-5 ) );
      TESTER_ASSERT( core::equal<double>( pot.getMean()[ 2 ], 1, 1e-5 ) );

      TESTER_ASSERT( core::equal<double>( pot.getCov()( 0, 0 ), 8, 1e-5 ) );
      TESTER_ASSERT( core::equal<double>( pot.getCov()( 1, 1 ), 5, 1e-5 ) );
      TESTER_ASSERT( core::equal<double>( pot.getCov()( 2, 2 ), 4, 1e-5 ) );

      TESTER_ASSERT( core::equal<double>( pot.getCov()( 2, 1 ), 2, 1e-5 ) );
      TESTER_ASSERT( core::equal<double>( pot.getCov()( 2, 0 ), -2, 1e-5 ) );
   }

   void testLinearGaussian3()
   {
      //
      // Compared against BNT, diamond network
      //   ->C -
      // W       -> S
      //   ->R -
      typedef algorithm::PotentialLinearGaussian   Potential;
      enum
      {
         S, C, R, W
      };

      Potential::Vector meanW = core::make_buffer1D<double>( 2 );
      Potential::Matrix covW( 1, 1 ); covW[ 0 ] = 1.2;
      Potential::VectorI idW = core::make_buffer1D<size_t>( (int)W );
      Potential potW( meanW, covW, idW );

      Potential::Vector meanR = core::make_buffer1D<double>( 50 );
      Potential::Matrix covR( 1, 1 ); covR[ 0 ] = 0.1;
      Potential::VectorI idR = core::make_buffer1D<size_t>( (int)R );
      std::vector<Potential::Dependency> dpsR;
      dpsR.push_back( Potential::Dependency( std::shared_ptr<Potential>( &potW, EmptyDeleter<Potential>() ), 0.1 ) );
      Potential potR( meanR, covR, idR, dpsR );

      Potential::Vector meanC = core::make_buffer1D<double>( 15 );
      Potential::Matrix covC( 1, 1 ); covC[ 0 ] = 0.5;
      Potential::VectorI idC = core::make_buffer1D<size_t>( (int)C );
      std::vector<Potential::Dependency> dpsC;
      dpsC.push_back( Potential::Dependency( std::shared_ptr<Potential>( &potW, EmptyDeleter<Potential>() ), 0.2 ) );
      Potential potC( meanC, covC, idC, dpsC );

      Potential::Vector meanS = core::make_buffer1D<double>( 30 );
      Potential::Matrix covS( 1, 1 ); covS[ 0 ] = 3;
      Potential::VectorI idS = core::make_buffer1D<size_t>( (int)S );
      Potential::Vector wS = core::make_buffer1D<double>( 1, 2 );
      
      std::vector<Potential::Dependency> dpsS;
      dpsS.push_back( Potential::Dependency( std::shared_ptr<Potential>( &potR, EmptyDeleter<Potential>() ), wS[ 0 ] ) );
      dpsS.push_back( Potential::Dependency( std::shared_ptr<Potential>( &potC, EmptyDeleter<Potential>() ), wS[ 1 ] ) );
      Potential potS( meanS, covS, idS, dpsS );

      algorithm::PotentialGaussianCanonical potr = potR.toGaussianCanonical();
      algorithm::PotentialGaussianCanonical pots = potS.toGaussianCanonical();
      algorithm::PotentialGaussianCanonical potc = potC.toGaussianCanonical();
      algorithm::PotentialGaussianCanonical potw = potW.toGaussianCanonical();

      algorithm::PotentialGaussianMoment result = (potw * potr * potc * pots).toGaussianMoment();
      result.print( std::cout );
      
      // validated against matlab BNT
      const double tol = 1e-3;
      TESTER_ASSERT( core::equal<double>( result.getMean()[ S ], 145.8, tol ) );
      TESTER_ASSERT( core::equal<double>( result.getMean()[ C ], 15.4, tol ) );
      TESTER_ASSERT( core::equal<double>( result.getMean()[ R ], 50.2, tol ) );
      TESTER_ASSERT( core::equal<double>( result.getMean()[ W ], 2, tol ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( S, S ), 4.092, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( S, C ), 0.596, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( S, R ), 0.248, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( S, W ), 0.48, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( C, C ), 0.548, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( C, R ), 0.024, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( C, W ), 0.24, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( R, R ), 0.112, 1e-6 ) );
      TESTER_ASSERT( core::equal<double>( result.getCov()( R, W ), 0.12, 1e-6 ) );

      TESTER_ASSERT( core::equal<double>( result.getCov()( W, W ), 1.2, 1e-6 ) );

      // the result must be normalized
      const double alpha = result.getAlpha();
      result.normalizeGaussian();
      TESTER_ASSERT( core::equal<double>( alpha, result.getAlpha(), 1e-6 ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
// test basic behaviour of linear gaussians
TESTER_TEST( testLinearGaussian3 );
TESTER_TEST( testLinearGaussian2 );
TESTER_TEST( testLinearGaussian1 );
/*
//TESTER_TEST( testInferenceGaussianBn );

TESTER_TEST( testPotentialTableMlParametersEstimation );
TESTER_TEST( testBasicInfPotentialTable );
TESTER_TEST( testBnPotentialSampling );
*/
TESTER_TEST_SUITE_END();
#endif