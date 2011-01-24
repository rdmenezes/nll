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

         typedef nll::core::DatabaseInputAdapterRead<Classifier::Database> InputAdapter;
         InputAdapter adapter( preprocessedDat );

         for ( ui32 f1 = 0; f1 < 4; ++f1 )
         {
            for ( ui32 f2 = f1 + 1; f2 < 4; ++f2 )
            {
               const double independence = nll::core::checkStatisticalIndependence( adapter, 0, 3 );
               TESTER_ASSERT( fabs( independence ) < 1e-10 ); // variable must be statistically independent
            }
         }

         nll::algorithm::FeatureTransformationNormalization<Input> normalizer;
         normalizer.compute( preprocessedDat );
         Classifier::Database preprocessedDatNorm = normalizer.transform( preprocessedDat );

         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.01, 10 ), preprocessedDatNorm );
         std::cout << "ICA error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.027 );
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

         nll::algorithm::FeatureTransformationNormalization<Input> preprocesser;
         preprocesser.compute( dat );
         Classifier::Database preprocessedDat = preprocesser.transform( dat );
         double error = c.evaluate( nll::core::Buffer1D<double>(), preprocessedDat );
         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.061 );
      }

      void testKernelKpca()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "iris.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;
         
         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         typedef nll::algorithm::KernelRbf<Input> Kernel;
         typedef nll::algorithm::FeatureTransformationKernelPca<Input, Kernel > KPca;

         nll::algorithm::FeatureTransformationNormalization<Input> preprocesser;
         preprocesser.compute( dat );
         Classifier::Database preprocessedDat = preprocesser.transform( dat );

         KPca pca( Kernel( 30 ) );
         bool computed = pca.compute( preprocessedDat, 16 );
         TESTER_ASSERT( computed );

         Classifier::Database data = pca.transform( preprocessedDat );


         ClassifierImpl c;
         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.00008, 100 ), data );


         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.034 );
      }

      void testKernelKpca2()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "iris.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;
         
         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         typedef nll::algorithm::KernelPolynomial<Input> Kernel;
         typedef nll::algorithm::FeatureTransformationKernelPca<Input, Kernel > KPca;

         KPca pca( Kernel( 1 ) );
         pca.compute( dat, 4 );
         Classifier::Database data = pca.transform( dat );

         ClassifierImpl c;
         double error = c.evaluate( nll::core::make_buffer1D<double>( 0.000008, 100 ), data );

         std::cout << "error=" << error << std::endl;
         TESTER_ASSERT( fabs( error ) <= 0.021 );
      }

      void testSammon()
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

         c.learn( preprocessedDat, nll::core::Buffer1D<double>() );


         {
            nll::core::Image<nll::ui8> i = nll::utility::printProjection( 512, 512, preprocessedDat );
            nll::core::writeBmp( i, NLL_DATABASE_PATH "sammon_iris.bmp" );
         }


         {
            nll::core::Image<nll::ui8> i = nll::utility::printProjection( 512, 512, preprocessedDat, c );
            nll::core::writeBmp( i, NLL_DATABASE_PATH "sammon_irisc.bmp" );
         }
      }
   };

   TESTER_TEST_SUITE( TestIrisDatabase );
   TESTER_TEST( testSammon );
   TESTER_TEST( testSvmIca );
   TESTER_TEST( testSvm );
   TESTER_TEST( testSvmPca );
   TESTER_TEST( testRbf );
   TESTER_TEST( testQda );
   TESTER_TEST( testBayes );
   TESTER_TEST( testKernelKpca );
   TESTER_TEST( testKernelKpca2 );
   TESTER_TEST_SUITE_END();
}
}
