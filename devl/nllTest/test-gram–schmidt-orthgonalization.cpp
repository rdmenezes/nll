#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

class TestGramSchmidt
{
public:
   void testBasic()
   {
      std::vector< std::vector< double > > vectors;
      vectors.push_back( core::make_vector<double>( 3, 1) );
      vectors.push_back( core::make_vector<double>( 2, 2) );
      std::vector< std::vector< double > > r = core::GramSchmidt<>::orthogonalize( vectors );

      TESTER_ASSERT( fabs( r[ 0 ][ 0 ] - 0.948683298 ) < 1e-4 );
      TESTER_ASSERT( fabs( r[ 0 ][ 1 ] - 0.316227766 ) < 1e-4 );

      TESTER_ASSERT( fabs( r[ 1 ][ 0 ] - -0.316227766 ) < 1e-4 );
      TESTER_ASSERT( fabs( r[ 1 ][ 1 ] - 0.948683298 ) < 1e-4 );

      core::GramSchmidt<>::orthogonalizeLast( r );
      TESTER_ASSERT( fabs( core::dot( r[ 0 ], r[ 1 ] ) ) < 1e-8 );

      TESTER_ASSERT( fabs( r[ 0 ][ 0 ] - 0.948683298 ) < 1e-4 );
      TESTER_ASSERT( fabs( r[ 0 ][ 1 ] - 0.316227766 ) < 1e-4 );

      TESTER_ASSERT( fabs( r[ 1 ][ 0 ] - -0.316227766 ) < 1e-4 );
      TESTER_ASSERT( fabs( r[ 1 ][ 1 ] - 0.948683298 ) < 1e-4 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGramSchmidt);
TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif