#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

struct TestBreastCancer
{
   typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
   typedef nll::algorithm::Classifier<Input>                            Classifier;

   /**
    @brief Test Relieff with the cancer1.dt database
    */
   void testFeatureSelectionRelieff()
   {
   }
};
/*
TESTER_TEST_SUITE( TestBreastCancer );
TESTER_TEST( testFeatureSelectionRelieff );
TESTER_TEST_SUITE_END();
*/
