#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"
#include "utils.h"

namespace nll
{
namespace tutorial
{
   struct TestKernelPca
   { 
      void test1()
      {
         typedef nll::benchmark::BenchmarkDatabases::Database Database;
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;
         std::cout << "size=" << dat.size() << std::endl;

         typedef nll::core::DatabaseInputAdapterRead<Database> Adapter;

         Adapter adapter( dat );
         typedef nll::algorithm::KernelRbf<Input>  Kernel;
         typedef nll::algorithm::KernelPca<Input, Kernel> KernelPca;

         KernelPca kpca;
         Kernel kernel( 2.0 );
         kpca.compute( adapter, 200, kernel );
      }
   };

   TESTER_TEST_SUITE( TestKernelPca );
    TESTER_TEST( test1 );
   TESTER_TEST_SUITE_END();
}
}