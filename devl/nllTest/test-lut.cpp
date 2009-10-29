#include "stdafx.h"
#include <nll/nll.h>
#include <sstream>

class TestLut
{
public:
   /**
    */
   void simpleTest()
   {
      nll::imaging::LookUpTransformWindowingRGB lut( 0, 99, 10, 3 );

      lut.createGreyscale();

      {
         const unsigned char* v1 = lut.transform( -10 );
         TESTER_ASSERT( v1[ 0 ] == 0 );
         TESTER_ASSERT( v1[ 1 ] == 0 );
         TESTER_ASSERT( v1[ 2 ] == 0 );
      }

      {
         const unsigned char* v1 = lut.transform( 9 );
         TESTER_ASSERT( v1[ 0 ] == 0 );
         TESTER_ASSERT( v1[ 1 ] == 0 );
         TESTER_ASSERT( v1[ 2 ] == 0 );
      }

      {
         const unsigned char* v1 = lut.transform( 10 );
         TESTER_ASSERT( v1[ 0 ] == (int)( 10.0 / 100 * 256 ) );
         TESTER_ASSERT( v1[ 1 ] == (int)( 10.0 / 100 * 256 ) );
         TESTER_ASSERT( v1[ 2 ] == (int)( 10.0 / 100 * 256 ) );
      }

      nll::imaging::LookUpTransformWindowingRGB lut2( 0, 255, 256, 1 );
      lut2.createGreyscale();
      TESTER_ASSERT( *lut2.transform( 10 ) == 10 );
      TESTER_ASSERT( *lut2.transform( 0 ) == 0 );
      TESTER_ASSERT( *lut2.transform( 255 ) == 255 );
      TESTER_ASSERT( *lut2.transform( 127 ) == 127 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLut);
TESTER_TEST(simpleTest);
TESTER_TEST_SUITE_END();
#endif
