#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"
#include "utils.h"

namespace nll
{
namespace tutorial
{
   struct TestUspsDatabase
   { 
      void testSvm()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;
         std::cout << "size=" << dat.size() << std::endl;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationPca<Input> preprocesser;
         preprocesser.compute( dat, 32 );

         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.007, 100 ), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.0199 );
      }

      void testNN()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierMlp<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationPca<Input> preprocesser;
         preprocesser.compute( dat, 32 );

         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 100, 0.3, 60 ), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.033 );
      }

      void testQda()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierDiscriminant<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationPca<Input> preprocesser;
         preprocesser.compute( dat, 25 );

         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::Buffer1D<double>(), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.034 );
      }

      void testNNeighbour()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::MetricEuclidian<Input>                          MetricEuclidian;
         typedef nll::algorithm::ClassifierNearestNeighbor<Input, MetricEuclidian> ClassifierImpl;
         MetricEuclidian metric;
         ClassifierImpl c( &metric );

         nll::algorithm::FeatureTransformationPca<Input> preprocesser;
         preprocesser.compute( dat, 25 );

         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 5 ), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.031 );
      }
   };

   TESTER_TEST_SUITE( TestUspsDatabase );
    TESTER_TEST( testSvm );
    TESTER_TEST( testNN );
    TESTER_TEST( testQda );
    TESTER_TEST( testNNeighbour );
   TESTER_TEST_SUITE_END();
}
}