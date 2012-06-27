#include <nll/nll.h>
#include <tester/register.h>

struct TestConverter
{
   void testId()
   {
      std::vector<double> v1 = nll::core::make_vector<double>( 1, 2, 3, 4 );
      std::vector<double> v2;

      nll::core::convert( v1, v2 );
      TESTER_ASSERT( v2.size() == 4 );
      TESTER_ASSERT( fabs( v2[ 0 ] - v1[ 0 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 1 ] - v1[ 1 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 2 ] - v1[ 2 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 3 ] - v1[ 3 ] ) < 1e-8 );
   }

   void testVec()
   {
      std::vector<double> v1 = nll::core::make_vector<double>( 1, 2, 3, 4 );
      std::vector<float> v2;

      nll::core::convert( v1, v2 );
      TESTER_ASSERT( v2.size() == 4 );
      TESTER_ASSERT( fabs( v2[ 0 ] - v1[ 0 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 1 ] - v1[ 1 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 2 ] - v1[ 2 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 3 ] - v1[ 3 ] ) < 1e-8 );
   }

   void testBuf()
   {
      std::vector<double> v1 = nll::core::make_vector<double>( 1, 2, 3, 4 );
      nll::core::Buffer1D<float> v2;

      nll::core::convert( v1, v2 );
      TESTER_ASSERT( v2.size() == 4 );
      TESTER_ASSERT( fabs( v2[ 0 ] - v1[ 0 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 1 ] - v1[ 1 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 2 ] - v1[ 2 ] ) < 1e-8 );
      TESTER_ASSERT( fabs( v2[ 3 ] - v1[ 3 ] ) < 1e-8 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestConverter);
 TESTER_TEST(testId);
 TESTER_TEST(testVec);
TESTER_TEST_SUITE_END();
#endif