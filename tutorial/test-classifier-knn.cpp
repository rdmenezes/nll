#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
	struct TestClassifierKnn
	{
		typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
		typedef nll::algorithm::Classifier<Input>                            Classifier;

		/**
	     @brief In this test a nearest neighbour will be used.
		*/
		void test()
		{
			// find the correct benchmark
			const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "cancer1.dt" );
			ensure( benchmark, "can't find benchmark" );
			Classifier::Database dat = benchmark->database;

         typedef nll::algorithm::MetricEuclidian<Input>                          MetricEuclidian;
         typedef algorithm::ClassifierNearestNeighbor<Input, MetricEuclidian >	ClassifierImpl;
			
			// choose the metric to be used
			const MetricEuclidian metric;
			
			// create the classifier
			ClassifierImpl c( &metric );

         // learn and test the classifier
         c.learn( dat, core::make_buffer1D<double>( 1.0 ) );
         Classifier::Result rr = c.test( dat );
         TESTER_ASSERT( rr.testingError < 0.045 );
		}
   };

   TESTER_TEST_SUITE( TestClassifierKnn );
   TESTER_TEST( test );
   TESTER_TEST_SUITE_END();
}
}
