#include "stdafx.h"
#include <nll/nll.h>

class TestNllClassifierSvm
{
public:
   void testNllClassifierSvm()
   {
      typedef float                                TYPE;
      typedef nll::core::Buffer1D<TYPE>            Point;
      typedef nll::algorithm::ClassifierSvm<Point> Svm;

      Svm::Database dat = buildXorDatabase<Point>();

      Svm svm;
      Svm::BaseClassifier* base = &svm;
      base->learn( dat, nll::core::make_buffer1D<double>( 0.01, 100 ) );
      Svm::Result res = base->test( dat );
      
      TESTER_ASSERT( res.learningError <= 0 && res.testingError <= 0 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllClassifierSvm);
TESTER_TEST(testNllClassifierSvm);
TESTER_TEST_SUITE_END();
#endif
