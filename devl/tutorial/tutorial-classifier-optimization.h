#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
   /**
    @ingroup tutorial
    @brief Classifier's model optimization tutorial

    The goals of this tutorial are to:
    - Learn how to optimize the parameters'model of a classifier
    - Train a classifier on the <code>LEARNING|VALIDATION</code> database
    - Evaluate the classifier on the <code>TESTING</code> database
    */
   struct TutorialClassifierOptimization
   {
      typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
      typedef nll::algorithm::Classifier<Input>                            Classifier;

      void optimization()
      {
         // find the database
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "cancer1.dt" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         
         // define the classifier to be used
         // a SVM classifier is used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         // optimize the parameters of the classifier on the original dataset
         // we will use a grid search algorithm that will test points on this grid
         // for each point, the classifier is evaluated: a 10-fold cross validation is
         // runned on the learning database
         Classifier::OptimizerClientClassifier classifierOptimizer = c.createOptimizer( dat );
         nll::algorithm::OptimizerGridSearch parametersOptimizer;
         std::vector<double> params = parametersOptimizer.optimize( classifierOptimizer, ClassifierImpl::buildParameters() );

         // test that the solution we found is suitable on the test/learning database
         double error = c.evaluate( nll::core::make_buffer1D( params ), dat );
         TESTER_ASSERT( error < 0.002 );
         
         // learn the TEST and VALIDATION database with the optimized parameters, and test the classifier
         // on the TESTING database
         c.learnTrainingDatabase( dat, nll::core::make_buffer1D( params ) );
         Classifier::Result rr = c.test( dat );
         TESTER_ASSERT( rr.testingError < 0.023 );
      }
   };
}
}
