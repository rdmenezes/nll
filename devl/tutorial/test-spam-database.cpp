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
    @brief Test the accuracy on the spam database from UCI. Achieved 0.2% error rate with SVM after feature normalization
    */
   struct TestSpamDatabase
   { 
      
      void testSvm()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "spambase.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationNormalization<Input> preprocessor;
         preprocessor.compute( dat );
         Classifier::Database preprocessedDat = preprocessor.process( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 100, 100 ), preprocessedDat );
         TESTER_ASSERT( fabs( error ) <= 0.002 );
      }
   };

   TESTER_TEST_SUITE( TestSpamDatabase );
   TESTER_TEST( testSvm );
   TESTER_TEST_SUITE_END();
}
}
