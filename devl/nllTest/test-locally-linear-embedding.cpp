#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>

#include "database-builder.h"

using namespace nll;

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

   void testSwissRoll()
   {
      typedef std::vector<double> Point;
      typedef algorithm::Classifier<Point>::Database Database;
      Database dat = createSwissRoll();

      Database test;
      for ( ui32 n = 0; n < 15; ++n )
         test.add( dat[ n ] );

      typedef core::DatabaseInputAdapter< Database >   Adapter;

      Adapter inputs( test );
      algorithm::LocallyLinearEmbedding lle;
      std::vector<Point> ps = lle.transform( inputs, 2, 3 );

      for ( ui32 n = 0; n < ps.size(); ++n )
      {
         std::cout << "p=" << ps[ n ][ 0 ] << " " << ps[ n ][ 1 ] << std::endl;
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLle);
TESTER_TEST(testSwissRoll);
TESTER_TEST(test);
TESTER_TEST_SUITE_END();
#endif
