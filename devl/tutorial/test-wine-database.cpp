#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
   /**
    @ingroup tutorial
    @brief Test the accuracy on the wine database from UCI. Achieved 0% error rate with SVM after feature normalization
    */
   struct TestWineDatabase
   { 
      void testSvm()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "wine.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationNormalization<Input> preprocesser;
         preprocesser.compute( dat );
         Classifier::Database preprocessedDat = preprocesser.process( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.01, 50 ), preprocessedDat );
         TESTER_ASSERT( fabs( error ) <= 0 );
      }

      void testMlp()
      {
         srand( 0 );
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "wine.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierMlp<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationNormalization<Input> preprocesser;
         preprocesser.compute( dat );
         Classifier::Database preprocessedDat = preprocesser.process( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 10, 0.05, 3 ), preprocessedDat );
         TESTER_ASSERT( fabs( error ) <= 0.07 );
      }
   };

   TESTER_TEST_SUITE( TestWineDatabase );
   TESTER_TEST( testSvm );
   TESTER_TEST( testMlp );
   TESTER_TEST_SUITE_END();
}
}