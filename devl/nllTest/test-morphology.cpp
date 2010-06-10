#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

class TestNllMorphology
{
public:

   void testNllMorphology()
   {
      typedef nll::core::Image<nll::ui8> Image;
      Image i1(NLL_TEST_PATH "data/image/test-image1.bmp");
      Image i2(NLL_TEST_PATH "data/image/test-image2.bmp");
      Image i3(NLL_TEST_PATH "data/image/test-image2.bmp");

      nll::core::decolor( i1 );
      nll::core::binarize( i1, nll::core::ThresholdGreater<nll::ui8>( 200 ) );

      nll::core::extend( i1, 3 );
      nll::core::writeBmp( i1, NLL_TEST_PATH "data/test-morpho1o.bmp" );
      nll::core::decolor( i1 );

      nll::core::ImageMask mask = nll::core::createMask( i1 );
      nll::debug::writeDebug( NLL_TEST_PATH "data/dbgmask.txt", mask.getBuf(), mask.sizex(), mask.sizey() );
      nll::core::dilate( mask );
      nll::core::erode( mask );
      nll::core::convert( mask, i1 );
      nll::core::extend( i1, 3 );
      nll::core::writeBmp( i1, NLL_TEST_PATH "data/test-morpho1.bmp" );

      nll::core::dilate( i2 );
      nll::core::writeBmp( i2, NLL_TEST_PATH "data/test-morpho-greydilate.bmp" );

      nll::core::erode( i3 );
      nll::core::writeBmp( i3, NLL_TEST_PATH "data/test-morpho-greyerode.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllMorphology);
TESTER_TEST(testNllMorphology);
TESTER_TEST_SUITE_END();
#endif
