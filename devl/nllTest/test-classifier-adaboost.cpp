#include "stdafx.h"
#include <nll/nll.h>
#include <time.h>

class TestNllAlgorithmClassifierAdaboost
{
public:
   void testAdaboostClassifier1()
   {
      typedef nll::core::Buffer1D<double>                               Point;
      typedef nll::algorithm::ClassifierAdaboost<Point, nll::algorithm::ClassifierSvm>  ClassifierAdaboost;
      typedef nll::algorithm::ClassifierSvm<Point>                      Base;
      typedef nll::algorithm::Classifier<Point>                         Classifier;

      srand( 0 );
      Base* b = new Base();
      Classifier* adaboost = new ClassifierAdaboost( b, 10, 0.5 );

      Classifier::Database dat = loadDatabaseSpect<Point>();
      adaboost->learn( dat, nll::core::make_buffer1D<double>( 1, 1 ) );

      Classifier::Result r = adaboost->test( dat );
      TESTER_ASSERT( r.testingError < 0.8 );
   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllAlgorithmClassifierAdaboost);
 TESTER_TEST(testAdaboostClassifier1);
TESTER_TEST_SUITE_END();
#endif
