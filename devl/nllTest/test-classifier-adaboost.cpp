# include <nll/nll.h>
#include "database-builder.h"
#include <tester/register.h>

using namespace nll::core;
using namespace nll::algorithm;

class TestBoosting
{
public:
   void test1()
   {
      srand( 2 );
      typedef Buffer1D<double>   Point;
      typedef ClassifierAdaboostM1<Point, ClassifierMlp >  Boosting;

      FactoryClassifierMlp<Point> factory;

      Boosting boosting( factory, 20, 0.3 );
      Boosting::Database dat = loadDatabaseSpect<Point>();

      boosting.learn( dat, nll::core::make_buffer1D<double>( 4, 0.1, 1 ) );
      Boosting::Result r = boosting.test( dat );
      std::cout << "adaboost learning error=" << r.learningError << " test error=" << r.testingError <<std::endl;
      TESTER_ASSERT( r.learningError < 0.3 );
      TESTER_ASSERT( r.testingError < 0.3 );

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