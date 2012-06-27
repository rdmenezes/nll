#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

class TestNllRegionGrowing
{
public:
   void testRegionGrowing()
   {
      typedef nll::algorithm::RegionGrowing<nll::ui8> RegionGrowingTest;

      
      nll::core::Image<nll::ui8> img( NLL_TEST_PATH "data/image/test-image4.bmp");
      RegionGrowingTest::DifferentPixel different( 3 );
      RegionGrowingTest rgrowing( different );
      nll::core::ImageMask out;

      rgrowing.grow( img, nll::core::vector2i( 20, 10 ), 1, out );
      rgrowing.grow( img, nll::core::vector2i( 206, 256 - 185 ), 2, out );
      nll::core::Image<nll::ui8> t1 = nll::core::extract( img, out );

      for ( nll::ui32 x = 0; x < 63; ++x)
         for ( nll::ui32 y = 0; y < 127; ++y)
            TESTER_ASSERT( out( x, y, 0 ) == 1 );

      TESTER_ASSERT( out( 206, 256 - 185, 0 ) == 2 );
      TESTER_ASSERT( out( 206 + 10, 256 - 185, 0 ) == 2 );
      TESTER_ASSERT( out( 206 - 10, 256 - 185, 0 ) == 2 );

      nll::core::writeBmp( t1, NLL_TEST_PATH "data/test-e1.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllRegionGrowing);
TESTER_TEST(testRegionGrowing);
TESTER_TEST_SUITE_END();
#endif
