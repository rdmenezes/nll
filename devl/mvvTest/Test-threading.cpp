#include "stdafx.h"

/**
 * Test harness for mvvTest.
 *
 * @version 1.0
 * @author 
 */
class TestmvvTest : public CppUnit::TestFixture  
{
public:
   /**
    * Called before the test begins to set up the test.
    */
   void setUp()
   {
   }
   /**
    * Called after the test finishes.
    */
   void tearDown()
   {
   }
   /**
    * 
    */
   void test1()
   {
   }

   CPPUNIT_TEST_SUITE(TestmvvTest);
   CPPUNIT_TEST(test1);
   CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestmvvTest);
