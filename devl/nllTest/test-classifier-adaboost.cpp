#include "stdafx.h"
# include <nll/nll.h>

using namespace nll::core;
using namespace nll::algorithm;

class TestBoosting
{
public:
   void test1()
   {
      srand( 0 );
      typedef Buffer1D<double>   Point;
      typedef ClassifierAdaboostM1<Point, ClassifierMlp >  Boosting;

      FactoryClassifierMlp<Point> factory;

      Boosting boosting( factory, 30, 0.5 );
      Boosting::Database dat = loadDatabaseSpect<Point>();

      boosting.learn( dat, nll::core::make_buffer1D<double>( 2, 0.1, 0.1 ) );
      Boosting::Result r = boosting.test( dat );
      TESTER_ASSERT( r.learningError < 0.15 );
      TESTER_ASSERT( r.testingError < 0.25 );

      boosting.write( "tmpboosting.bin" );
      Boosting boosting2( factory, 0, 0 );
      boosting2.read( "tmpboosting.bin" );
      Boosting::Result r2 = boosting.test( dat );
      TESTER_ASSERT( r.learningError == r2.learningError );
      TESTER_ASSERT( r.testingError == r2.testingError );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestBoosting);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif