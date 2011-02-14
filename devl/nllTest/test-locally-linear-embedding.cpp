#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>

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

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLle);
TESTER_TEST(test);
TESTER_TEST_SUITE_END();
#endif
