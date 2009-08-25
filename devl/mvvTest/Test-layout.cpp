#include "stdafx.h"
#include <mvv/layout.h>


class TestLayout
{
public:
   // test horizontal layout
   void testLayout1()
   {
      mvv::PaneListHorizontal* p1 = new mvv::PaneListHorizontal( nll::core::vector2ui( 1000, 10000 ),
                                                                 nll::core::vector2ui( 10, 100 ) );

      mvv::Pane* p11 = new mvv::PaneListHorizontal( nll::core::vector2ui( 0, 0 ),
                                                    nll::core::vector2ui( 0, 0 ) );
      mvv::Pane* p12 = new mvv::PaneListHorizontal( nll::core::vector2ui( 0, 0 ),
                                                    nll::core::vector2ui( 0, 0 ) );
      mvv::Pane* p13 = new mvv::PaneListHorizontal( nll::core::vector2ui( 0, 0 ),
                                                    nll::core::vector2ui( 0, 0 ) );
      p1->addChild( p11, 0.2 );
      p1->addChild( p12, 0.3 );
      p1->addChild( p13, 0.5 );

      p1->updateLayout();
      std::cout << "end";
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLayout);
TESTER_TEST(testLayout1);
TESTER_TEST_SUITE_END();
#endif