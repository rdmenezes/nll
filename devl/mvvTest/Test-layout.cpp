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

      TESTER_ASSERT( p11->getSize()[ 0 ] == 2 );
      TESTER_ASSERT( p11->getSize()[ 1 ] == p1->getSize()[ 1 ] );
      TESTER_ASSERT( p11->getOrigin()[ 0 ] == 1000 );
      TESTER_ASSERT( p11->getOrigin()[ 1 ] == 10000 );

      TESTER_ASSERT( p12->getSize()[ 0 ] == 3 );
      TESTER_ASSERT( p12->getSize()[ 1 ] == p1->getSize()[ 1 ] );
      TESTER_ASSERT( p12->getOrigin()[ 0 ] == 1002 );
      TESTER_ASSERT( p12->getOrigin()[ 1 ] == 10000 );

      TESTER_ASSERT( p13->getSize()[ 0 ] == 5 );
      TESTER_ASSERT( p13->getSize()[ 1 ] == p1->getSize()[ 1 ] );
      TESTER_ASSERT( p13->getOrigin()[ 0 ] == 1005 );
      TESTER_ASSERT( p13->getOrigin()[ 1 ] == 10000 );

      p13->setVisible( false );
      p1->updateLayout();

      TESTER_ASSERT( p11->getSize()[ 0 ] == 4 );
      TESTER_ASSERT( p11->getSize()[ 1 ] == p1->getSize()[ 1 ] );
      TESTER_ASSERT( p11->getOrigin()[ 0 ] == 1000 );
      TESTER_ASSERT( p11->getOrigin()[ 1 ] == 10000 );

      TESTER_ASSERT( p12->getSize()[ 0 ] == 6 );
      TESTER_ASSERT( p12->getSize()[ 1 ] == p1->getSize()[ 1 ] );
      TESTER_ASSERT( p12->getOrigin()[ 0 ] == 1004 );
      TESTER_ASSERT( p12->getOrigin()[ 1 ] == 10000 );
   }

   // test horizontal layout
   void testLayout2()
   {
      mvv::PaneListVertical* p1 = new mvv::PaneListVertical( nll::core::vector2ui( 10000, 1000 ),
                                                             nll::core::vector2ui( 100, 10 ) );

      mvv::Pane* p11 = new mvv::PaneListVertical( nll::core::vector2ui( 0, 0 ),
                                                  nll::core::vector2ui( 0, 0 ) );
      mvv::Pane* p12 = new mvv::PaneListVertical( nll::core::vector2ui( 0, 0 ),
                                                  nll::core::vector2ui( 0, 0 ) );
      mvv::Pane* p13 = new mvv::PaneListVertical( nll::core::vector2ui( 0, 0 ),
                                                  nll::core::vector2ui( 0, 0 ) );
      p1->addChild( p11, 0.2 );
      p1->addChild( p12, 0.3 );
      p1->addChild( p13, 0.5 );
      p1->updateLayout();

      TESTER_ASSERT( p11->getSize()[ 1 ] == 2 );
      TESTER_ASSERT( p11->getSize()[ 0 ] == p1->getSize()[ 0 ] );
      TESTER_ASSERT( p11->getOrigin()[ 1 ] == 1000 );
      TESTER_ASSERT( p11->getOrigin()[ 0 ] == 10000 );

      TESTER_ASSERT( p12->getSize()[ 1 ] == 3 );
      TESTER_ASSERT( p12->getSize()[ 0 ] == p1->getSize()[ 0 ] );
      TESTER_ASSERT( p12->getOrigin()[ 1 ] == 1002 );
      TESTER_ASSERT( p12->getOrigin()[ 0 ] == 10000 );

      TESTER_ASSERT( p13->getSize()[ 1 ] == 5 );
      TESTER_ASSERT( p13->getSize()[ 0 ] == p1->getSize()[ 0 ] );
      TESTER_ASSERT( p13->getOrigin()[ 1 ] == 1005 );
      TESTER_ASSERT( p13->getOrigin()[ 0 ] == 10000 );

      p13->setVisible( false );
      p1->updateLayout();

      TESTER_ASSERT( p11->getSize()[ 1 ] == 4 );
      TESTER_ASSERT( p11->getSize()[ 0 ] == p1->getSize()[ 0 ] );
      TESTER_ASSERT( p11->getOrigin()[ 1 ] == 1000 );
      TESTER_ASSERT( p11->getOrigin()[ 0 ] == 10000 );

      TESTER_ASSERT( p12->getSize()[ 1 ] == 6 );
      TESTER_ASSERT( p12->getSize()[ 0 ] == p1->getSize()[ 0 ] );
      TESTER_ASSERT( p12->getOrigin()[ 1 ] == 1004 );
      TESTER_ASSERT( p12->getOrigin()[ 0 ] == 10000 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLayout);
TESTER_TEST(testLayout1);
TESTER_TEST(testLayout2);
TESTER_TEST_SUITE_END();
#endif