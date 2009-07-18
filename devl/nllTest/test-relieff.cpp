#include "stdafx.h"
#include <nll/nll.h>


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
      Database dat2 = nll::core::filterDatabase( dat, nll::core::make_vector<nll::ui32>( Database::Sample::LEARNING ), Database::Sample::LEARNING );

      // find the optimized parameters on his database
      Classifier::OptimizerClientClassifier classifierOptimizer = classifier.createOptimizer( dat2 );
      nll::algorithm::OptimizerGridSearch parametersOptimizer;
      std::vector<double> optm = parametersOptimizer.optimize( classifierOptimizer, Classifier::buildParameters() );

      //
      nll::core::Buffer1D<double> params = nll::core::make_buffer1D<double>( optm[ 0 ], optm[ 1 ] );
     // nll::core::Buffer1D<double> params = nll::core::make_buffer1D<double>( 9.76563, 16 );
      
      // run relieff
      nll::algorithm::FeatureSelectionFilterRelieff<Point> relief( 18 );
      relief.compute( dat2 );
      Database datp = relief.process( dat2 );
      
      // evaluate the soluion
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