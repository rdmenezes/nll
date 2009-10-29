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

         nll::algorithm::FeatureTransformationNormalization<Input> preprocessor;
         preprocessor.compute( dat );
         Classifier::Database preprocessedDat = preprocessor.process( dat );
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
         ClassifierImpl c( 0.00001 );

         nll::algorithm::FeatureTransformationNormalization<Input> preprocessor;
         preprocessor.compute( dat );
         Classifier::Database preprocessedDat = preprocessor.process( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 3, 0.1, 3 ), preprocessedDat );
         TESTER_ASSERT( fabs( error ) <= 0.051 );
      }

      void testKnn()
      {
         srand( 0 );
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "wine.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::MetricManhattan<Input>           Metric;
         typedef nll::algorithm::ClassifierNearestNeighbor<Input, Metric> ClassifierImpl;

         Metric metric;
         ClassifierImpl c( &metric );

         nll::algorithm::FeatureTransformationNormalization<Input> preprocessor;
         preprocessor.compute( dat );
         Classifier::Database preprocessedDat = preprocessor.process( dat );
         double error = c.evaluate( nll::core::make_buffer1D<double>( 1 ), preprocessedDat );
         TESTER_ASSERT( fabs( error ) <= 0.09 );
      }

      void testGmm()
      {
         srand( 0 );
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "wine.data" );
         ensure( benchmark, "can't find benchmark" );
         const nll::benchmark::BenchmarkDatabases::Database& dat = benchmark->database;
         nll::algorithm::FeatureTransformationNormalization<Input> preprocessor;
         preprocessor.compute( dat );
         nll::benchmark::BenchmarkDatabases::Database preprocessedDat = preprocessor.process( dat );

         // define the classifier to be used
         typedef nll::algorithm::ClassifierGmm< std::vector<Input> > ClassifierImpl;
         ClassifierImpl::Database rDat;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const nll::benchmark::BenchmarkDatabases::Database::Sample& s = preprocessedDat[ n ];
            std::vector<Input> input( 1 );
            input[ 0 ] = s.input;
            rDat.add( ClassifierImpl::Database::Sample( input, s.output, (ClassifierImpl::Database::Sample::Type)s.type ) );
         }

         ClassifierImpl c;
         double error = c.evaluate( nll::core::make_buffer1D<double>( 3, 1 ), rDat );
         TESTER_ASSERT( fabs( error ) <= 0.012 );
      }

      void testSvmKPca()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         typedef nll::algorithm::KernelRbf<Input> Kernel;
         typedef nll::algorithm::FeatureTransformationKernelPca<Input, Kernel>   KernelPca;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "wine.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;
         std::cout << "size=" << dat.size() << std::endl;

         Kernel kernel( 0.5 );
         KernelPca kpca( kernel );
         kpca.compute( dat, 100 );
         Classifier::Database processedDat = kpca.process( dat );

         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         nll::algorithm::FeatureTransformationNormalization<Input> preprocessor;
         preprocessor.compute( processedDat );
         Classifier::Database preprocessedDat = preprocessor.process( processedDat );

         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.01, 50 ), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0 );
      }
   };

   TESTER_TEST_SUITE( TestWineDatabase );
    TESTER_TEST( testSvm );
    TESTER_TEST( testMlp );
    TESTER_TEST( testKnn );
    TESTER_TEST( testGmm );
    //TESTER_TEST( testSvmKPca );

   TESTER_TEST_SUITE_END();
}
}
