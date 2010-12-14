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
         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.001, 100 ), preprocessedDat );
         TESTER_ASSERT( fabs( error ) <= 0.03 );
      }

      void testSvmIca()
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

         nll::algorithm::FeatureTransformationIca<Input> preprocesser;
         preprocesser.compute( dat, 4 );
         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.01, 10 ), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.021 );
      }

      void testSvmPca()
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

         nll::algorithm::FeatureTransformationPca<Input> preprocesser;
         preprocesser.compute( dat, 4 );
         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.01, 10 ), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.021 );
      }

      void testRbf()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "iris.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierRbf<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationPca<Input> preprocesser;
         preprocesser.compute( dat, 4 );
         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 5, 0.1, 5 ), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.041 );
      }

      void testQda()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "iris.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierDiscriminant<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationPca<Input> preprocesser;
         preprocesser.compute( dat, 4 );
         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::Buffer1D<double>(), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.021 );
      }

      void testBayes()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "iris.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierNaiveBayes<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationPca<Input> preprocesser;
         preprocesser.compute( dat, 4 );
         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::Buffer1D<double>(), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.07 );
      }
   };

   TESTER_TEST_SUITE( TestIrisDatabase );
   TESTER_TEST( testSvm );
   TESTER_TEST( testSvmIca );
   TESTER_TEST( testSvmPca );
   TESTER_TEST( testRbf );
   TESTER_TEST( testQda );
   TESTER_TEST( testBayes );
   TESTER_TEST_SUITE_END();
}
}
