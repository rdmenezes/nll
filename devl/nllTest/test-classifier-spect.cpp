#include "stdafx.h"
#include <nll/nll.h>

# pragma warning( push )
# pragma warning( disable:4996 ) // unsecure function...
# pragma warning( disable:4189 ) // local variable not referenced
# pragma warning( disable:4101 ) // unreferenced

class TestNllClassifierSpect
{
public:
   typedef nll::core::Buffer1D<double>             Point;
   typedef nll::algorithm::ClassifierMlp<Point>    Mlp;
   typedef Mlp::Database                           Database;

   void testNllClassifierSpect()
   {
      Database dat = loadDatabaseSpect<Point>();
     

      srand((nll::ui32)time(0));
      Mlp::BaseClassifier* mlp = new Mlp();

      //nll::algorithm::OptimizerGeneticAlgorithm gaOptimizer( 10, 10, 3 );
      //std::vector<double> optimizedParametersStl = gaOptimizer.optimize( mlp->createOptimizer(dat), Mlp::buildParameters() );
      //nll::core::Buffer1D<double> optimizedParameters;
      //nll::core::convert( optimizedParametersStl, optimizedParameters, (nll::ui32)optimizedParametersStl.size() );

      nll::core::Buffer1D<nll::f64> optimizedParameters = nll::core::make_buffer1D<nll::f64>(5, 0.001, 2);

      nll::algorithm::FeatureSelectionWrapper<Point>* fs = new nll::algorithm::FeatureSelectionBestFirst<Point>();
      nll::core::Buffer1D<bool> result = fs->compute( mlp, optimizedParameters, dat );

      Database newdat = fs->process( dat );
      mlp->learn(newdat, optimizedParameters);
      const Mlp::Result resultTest = mlp->test(newdat);
      std::cout << "BestClassifierTestingErrorRate=" << resultTest.testingError << std::endl;
      TESTER_ASSERT( resultTest.testingError < 0.185 );
   }

   void testNllPcaClassifierSpect()
   {
      typedef nll::algorithm::ClassifierSvm<Point> Classifier;

      Database dat = loadDatabaseSpect<Point>();
      nll::algorithm::FeatureTransformationPca<Point> pca;
      pca.compute( dat, 9 );
      Database datProcessed = pca.process( dat );

      Classifier classifier;
      classifier.learn( datProcessed, nll::core::make_buffer1D<double>( 10, 100 ) );
      Classifier::Result result = classifier.test( datProcessed );

      TESTER_ASSERT( result.testingError < 0.097 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllClassifierSpect);
# ifndef DONT_RUN_VERY_SLOW_TEST
#  ifndef DONT_RUN_SLOW_TEST
TESTER_TEST(testNllClassifierSpect);
#  endif
# endif
TESTER_TEST(testNllPcaClassifierSpect);
TESTER_TEST_SUITE_END();
#endif

# pragma warning( pop )
