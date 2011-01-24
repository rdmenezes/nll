#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"
#include "utils.h"

namespace nll
{
namespace tutorial
{
   /**
    @ingroup tutorial
    @brief Test the accuracy on the spam database from UCI. Achieved 8% error rate with SVM after feature normalization
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
         Classifier::Database preprocessedDat = preprocessor.transform( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.001, 100 ), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.08 );
      }

      void testDisplay()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "spambase.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         {
            nll::core::Image<nll::ui8> i = nll::utility::printProjection( 512, 512, dat );
            nll::core::writeBmp( i, NLL_DATABASE_PATH "sammon.bmp" );
         }
      }
   };

   TESTER_TEST_SUITE( TestSpamDatabase );
   //TESTER_TEST( testSvm );
   TESTER_TEST( testDisplay );
   TESTER_TEST_SUITE_END();
}
}
