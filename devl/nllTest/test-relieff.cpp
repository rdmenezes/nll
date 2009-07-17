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
      Database dat = loadDatabaseSpect<Point>();
      Classifier classifier;


      Database dat2 = nll::core::filterDatabase( dat, nll::core::make_vector<nll::ui32>( Database::Sample::LEARNING ), Database::Sample::LEARNING );
      Classifier::OptimizerClientClassifier classifierOptimizer = classifier.createOptimizer( dat2 );
      nll::algorithm::OptimizerGridSearch parametersOptimizer;
      std::vector<double> optm = parametersOptimizer.optimize( classifierOptimizer, Classifier::buildParameters() );


      nll::core::Buffer1D<double> params = nll::core::make_buffer1D<double>( optm[ 0 ], optm[ 1 ] );
     // nll::core::Buffer1D<double> params = nll::core::make_buffer1D<double>( 9.76563, 16 );
      /*
      nll::algorithm::FeatureSelectionFilterRelieff<Point> relief( 18 );
      relief.compute( dat );
      Database datp = relief.process( dat );
      */
      classifier.learn( dat, params );
      Classifier::Result results = classifier.test( dat );
      std::cout << "l=" << results.learningError << std::endl;
      std::cout << "t=" << results.testingError << std::endl;
   }
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRelieff);
TESTER_TEST(testRelieff1);
TESTER_TEST_SUITE_END();
//#endif