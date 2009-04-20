#include "stdafx.h"
#include <nll/nll.h>

class TestNllLabelize
{
public:
   void testNllLabelize()
   {
      typedef nll::algorithm::Labelize<nll::ui8,
                                       nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                                       nll::algorithm::RegionPixelSimilar<nll::ui8> > Labelize;
      //typedef nll::algorithm::Labelize<nll::ui8> Labelize;

      
      nll::core::Image<nll::ui8> i(10, 6, 1);
      i(2, 2, 0) = 1;
      i(3, 2, 0) = 1;
      i(4, 2, 0) = 1;
      i(5, 2, 0) = 1;
      i(6, 2, 0) = 1;
      i(7, 2, 0) = 1;
      i(8, 2, 0) = 1;

      i(3, 3, 0) = 1;

      i(5, 3, 0) = 1;
      i(5, 4, 0) = 1;

      i(7, 3, 0) = 1;

      i(9, 3, 0) = 1;
      i(9, 4, 0) = 1;

      nll::core::readBmp(i, "data/image/test-image2.bmp");

      nll::core::Timer t1;
      //Labelize::DifferentPixel different( 1 );
      Labelize::DifferentPixel different( 3, 40 );
      Labelize labelize( different );
      
      Labelize::ComponentsInfo info = labelize.run( i, true );
      t1.end();
      std::cout << "label=" << t1.getTime() << std::endl;

      nll::core::Image<nll::ui8> ii;
      nll::utility::maskExport( info.labels, ii );
      nll::core::writeBmp( ii, "data/label.bmp");
      //TESTER_ASSERT
      // TODO finish automatic testing
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllLabelize);
TESTER_TEST(testNllLabelize);
TESTER_TEST_SUITE_END();
#endif
