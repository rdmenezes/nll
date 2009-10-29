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
    @brief Test the accuracy on the iris database from UCI. Achieved XXX% error rate with SVM after feature normalization
    */
   struct TestIrisDatabase
   { 
      void testSvm()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "iris.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationNormalization<Input> preprocesser;
         preprocesser.compute( dat );
         Classifier::Database preprocessedDat = preprocesser.process( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 1, 50 ), preprocessedDat );
         TESTER_ASSERT( fabs( error ) <= 0 );
      }
   };

   TESTER_TEST_SUITE( TestIrisDatabase );
   TESTER_TEST( testSvm );
   TESTER_TEST_SUITE_END();
}
}
