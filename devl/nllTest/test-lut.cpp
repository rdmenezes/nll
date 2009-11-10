#include "stdafx.h"
#include <nll/nll.h>
#include <sstream>

#include <emmintrin.h>

namespace nll
{
namespace imaging
{
   
}
}

// alignment float* alignedArray = (array + 15) & (~0x0F);
const unsigned size = 2048 * 4;

class TestLut
{
public:
   typedef nll::imaging::MapperLutColor<unsigned char>               Mapper;
   typedef nll::imaging::LookUpTransform<unsigned char, Mapper>      Lut;

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

   Lut createLut()
   {
      Mapper mapperLut( 256, 3 );
      Lut lut( mapperLut, -100, 1001 );
      return lut;
   }

   void testTransformComp()
   {
      srand( 0 );
      Lut lut = createLut();
      nll::core::Image<float> input( size, size, 1 );
      nll::core::Image<nll::ui32> output( size, size, 1 );
      nll::core::Image<nll::ui32> output2( size, size, 1 );
      for ( nll::core::Image<float>::iterator it = input.begin(); it != input.end(); ++it )
      {
         *it = (float)(rand() % 500);
      }

      nll::core::Image<float>::DirectionalIterator it = input.beginDirectional();
      nll::core::Image<nll::ui32>::DirectionalIterator out = output.beginDirectional();

      nll::core::Timer t1;
      for ( unsigned n = 0; n < input.sizex() * input.sizex(); ++n, ++it, ++out )
      {
         *out = lut.transformToIndex( *it );
      }
      std::cout << "LUT Time no multithreaded SSE=" << t1.getCurrentTime() << std::endl;

      nll::core::Timer t2;
      lut.transformToIndex( input.begin(), input.end(), output2.begin() );
      std::cout << "LUT Time with multithreaded SSE=" << t2.getCurrentTime() << std::endl;

      for ( unsigned n = 0; n < output.size(); ++n )
      {
         if ( output[ n ] != output2[ n ] )
            std::cout << "error val=" << output[ n ] << " " << output2[ n ] << std::endl;
         TESTER_ASSERT( output[ n ] == output2[ n ] );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLut);
TESTER_TEST(simpleTest);
TESTER_TEST(testTransformComp);
TESTER_TEST_SUITE_END();
#endif
