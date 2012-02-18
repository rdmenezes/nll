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

      // TODO ADD TEST WITH FULL EVIDENCE
   }

   void testBasicInfPotentialTable()
   {
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceVariableElimination<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
      testBasicInfPotentialTableImpl<algorithm::BayesianInferenceNaive<algorithm::BayesianNetwork<algorithm::PotentialTable>>>();
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianBayesianInference);
TESTER_TEST( testBasicInfPotentialTable );
TESTER_TEST_SUITE_END();
#endif