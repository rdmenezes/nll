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
    @brief Test the accuracy on the yeast database from UCI. Achieved 5.8% error rate
    */
   struct TestYeastDatabase
   { 
      typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
      typedef nll::algorithm::Classifier<Input>                            Classifier;

      void test()
      {
         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "yeast.data" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         double error = c.evaluate( nll::core::make_buffer1D<double>( 160, 100 ), dat );
         TESTER_ASSERT( fabs( error ) < 0.06 );
      }
   };

   TESTER_TEST_SUITE( TestYeastDatabase );
   TESTER_TEST( test );
   TESTER_TEST_SUITE_END();
}
}