#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>

#include "database-builder.h"

using namespace nll;

namespace
{
   double resultDiff[][ 2 ]=
   {
      {0.0919813,	0.116378	},
      {-0.0705267,	-0.106903	},
      {-0.0503165,	-0.0481841},	
      {-0.0151292,	0.052864	},
      {0.13377,	-0.0380011	},
      {0.175426,	-0.037694	},
      {-0.0501631,	-0.021391},	
      {-0.070078,	-0.140306	},
      {-0.0462889,	0.0121822	},
      {-0.0471031,	0.0652041	},
      {-0.0162099,	-0.127057	},
      {-0.0399076,	-0.161202	},
      {-0.0185961,	-0.00789646	},
      {0.0114212,	0.175377	},
      {-0.390967,	0.0674795	},
      {-0.0162348,	0.0856919},	
      {0.0831761,	0.0181782	},
      {-0.0607817,	-0.105075	},
      {0.273842,	-0.17991	},
      {-0.157061,	0.0281302	},
      {0.104268,	0.0766855	},
      {-0.0194355,	0.068165},	
      {0.0353714,	0.162934	},
      {-0.0450936,	-0.0251704	},
      {-0.0178707,	-0.0582978},	
      {-0.0371207,	0.159049	},
      {-0.00420392,	0.195743	},
      {-0.00988973,	0.21855	},
      {-0.0416936,	-0.160811},	
      {0.262445,	-0.142595	},
      {-0.0525825,	-0.00274234	},
      {-0.331047,	0.0574629},	
      {0.0576854,	0.180312	},
      {-0.032441,	-0.0558708},	
      {0.0184534,	0.115391	},
      {0.205915,	0.0141615	},
      {-0.0185253,	0.0750717},	
      {-0.0248972,	-0.131333},	
      {0.13198,	0.0213355	},
      {0.30741,	-0.142392	},
      {-0.0050033,	0.0715559},	
      {-0.0206994,	0.0597652},	
      {-0.0280606,	0.0191179},	
      {-0.063994,	-0.193636	},
      {-0.0154411,	0.0743834	},
      {0.0474232,	0.106252	},
      {0.0326014,	0.153431	},
      {-0.0389873,	-0.153607	},
      {0.0381289,	0.127884	},
      {-0.0577601,	-0.181246},	
      {0.0172092,	-0.0723689	},
      {-0.0514971,	-0.128117	},
      {0.0688424,	0.058101	},
      {-0.0359276,	0.101033	},
      {-0.0498981,	-0.174979	},
      {0.0248662,	0.150418	},
      {-0.0630962,	-0.165049	},
      {-0.0232841,	-0.133904	},
      {-0.0243514,	-0.0953876	},
      {-0.386157,	0.0715224},	
      {0.0262434,	0.173504	},
      {-0.0554428,	-0.179864},	
      {-0.0266665,	0.0169105},	
      {-0.0288051,	-0.13348},	
      {0.0599053,	0.193151	},
      {-0.0656851,	-0.202367},	
      {0.149923,	0.0224408	},
      {-0.0249476,	-0.14076	},
      {0.167238,	0.0457211	},
      {-0.0227089,	-0.067158	},
      {0.0421727,	0.115273	},
      {-0.0253109,	0.083775	},
      {0.209993,	0.00835865},	
      {-0.02409,	0.0400122},	
      {0.070004,	0.0555564}
   };
}

class TestLle
{
public:

   void test()
   {
      double points[][ 2 ] =
      {
         {-1, 0},
         {-0.5, -0.5},
         {-0.4, -0.8},
         {1, -1.1},
         {2, 0},
         {2.5, 1 },
         {1.5, 1.5},
         {1, 1}
      };

      typedef std::vector<double>   Point;
      typedef std::vector<Point>    Points;

      Points pointsT;
      for ( ui32 n = 0; n < core::getStaticBufferSize( points ); ++n )
         pointsT.push_back( core::make_vector<double>( points[ n ][ 0 ], points[ n ][ 1 ] ) );

      algorithm::LocallyLinearEmbedding lle;
      std::vector<Point> tfmPoints = lle.transform( pointsT, 1, 3 );
   }

   /**
    @brief test swiss roll against a common matlab implementation
    (see Matlab Toolbox for Dimensionality Reduction v0.7.2b)
    */
   void testSwissRoll()
   {
      typedef core::Buffer1D<double> Point;
      typedef algorithm::Classifier<Point>::Database Database;
      Database dat = createSwissRoll();
      typedef core::DatabaseInputAdapter< Database >   Adapter;

      Adapter inputs( dat );
      algorithm::LocallyLinearEmbedding lle;
      std::vector<Point> ps = lle.transform( inputs, 2, 4 );

      for ( ui32 n = 0; n < ps.size(); ++n )
      {
         std::cout << "p=" << ps[ n ][ 0 ] << " " << ps[ n ][ 1 ] << std::endl;
         TESTER_ASSERT( fabs( ps[ n ][ 0 ] - resultDiff[ n ][ 0 ] ) < 1e-2 );
         TESTER_ASSERT( fabs( ps[ n ][ 1 ] - resultDiff[ n ][ 1 ] ) < 1e-2 );
      }
   }

   void testSwissRoll2()
   {
      typedef core::Buffer1D<double> Point;
      typedef algorithm::Classifier<Point>::Database Database;
      Database dat = createSwissRoll();
      typedef core::DatabaseInputAdapter< Database >   Adapter;

      Adapter inputs( dat );
      algorithm::LocallyLinearEmbedding lle;
      std::vector<Point> ps = lle.transform( inputs, 2, 4 );

      for ( ui32 n = 0; n < ps.size(); ++n )
      {
         std::cout << "p=" << ps[ n ][ 0 ] << " " << ps[ n ][ 1 ] << std::endl;
         TESTER_ASSERT( fabs( ps[ n ][ 0 ] - resultDiff[ n ][ 0 ] ) < 1e-2 );
         TESTER_ASSERT( fabs( ps[ n ][ 1 ] - resultDiff[ n ][ 1 ] ) < 1e-2 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLle);
TESTER_TEST(testSwissRoll);
TESTER_TEST(test);
TESTER_TEST_SUITE_END();
#endif
