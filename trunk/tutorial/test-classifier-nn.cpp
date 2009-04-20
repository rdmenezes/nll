#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
   struct TestClassifierNN
   {
      typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
      typedef nll::algorithm::Classifier<Input>                            Classifier;

      /**
        In this test a neural network will be optimized using an harmony search algorithm.
       */
      void test()
      {
         srand( 1 );

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "cancer1.dt" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // use a multi layered perceptron as a classifier
         typedef algorithm::ClassifierMlp<Input> ClassifierImpl;
         ClassifierImpl classifier;

         // optimize the parameters of the classifier on the original dataset
         // we will use an harmony search algorithm.
         // for each point, the classifier is evaluated: a 10-fold cross validation is
         // runned on the learning database
         Classifier::OptimizerClientClassifier classifierOptimizer = classifier.createOptimizer( dat );

         nll::algorithm::StopConditionIteration stop( 10 );
         nll::algorithm::MetricEuclidian<nll::algorithm::OptimizerHarmonySearchMemory::TMetric::value_type> metric;

         nll::algorithm::OptimizerHarmonySearchMemory parametersOptimizer( 5, 0.8, 0.1, 1, &stop, 0.01, &metric );
         std::vector<double> params = parametersOptimizer.optimize( classifierOptimizer, ClassifierImpl::buildParameters() );
         
         // learn the TEST and VALIDATION database with the optimized parameters, and test the classifier
         // on the TESTING database
         classifier.learnTrainingDatabase( dat, nll::core::make_buffer1D( params ) );
         Classifier::Result rr = classifier.test( dat );

         TESTER_ASSERT( rr.testingError < 0.025 );
      }
   };

   TESTER_TEST_SUITE( TestClassifierNN );
   TESTER_TEST( test );
   TESTER_TEST_SUITE_END();
}
}
