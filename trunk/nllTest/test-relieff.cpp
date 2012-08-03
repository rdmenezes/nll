#include <nll/nll.h>
#include "database-builder.h"
#include <tester/register.h>


class TestRelieff
{
public:
   typedef nll::core::Buffer1D<float>              Point;
   typedef nll::algorithm::ClassifierSvm<Point>    Classifier;
   typedef Classifier::Database                    Database;

   void testRelieff1()
   {
      srand( 11 );

      // load the database
      Database dat = loadDatabaseSpect<Point>();
      Classifier classifier;

      // we only want to train on the LEARNING database as for the test
      Database dat2 = nll::core::filterDatabase( dat, nll::core::make_vector<size_t>( Database::Sample::LEARNING ), Database::Sample::LEARNING );

      
      nll::core::Buffer1D<double> params = nll::core::make_buffer1D<double>( 10, 100 );
      nll::algorithm::FeatureSelectionFilterRelieff<Point> relief( 13 );
      relief.compute( dat2 );
      Database datp = relief.transform( dat2 );
      
      // evaluate the solution
      classifier.learn( dat2, params );
      Classifier::Result results = classifier.test( dat );

      TESTER_ASSERT( results.testingError < 0.11 );
      TESTER_ASSERT( results.learningError < 0.1 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRelieff);
TESTER_TEST(testRelieff1);
TESTER_TEST_SUITE_END();
#endif
