#include "stdafx.h"
#include "database-benchmark.h"
#include "static-plug.h"
#include "decoder.h"

void loadYeastDatabase()
{
   nll::tutorial::DatabaseDecoderEncoder::AttributValues vals( 10 );
   vals[ 0 ] = nll::tutorial::DatabaseDecoderEncoder::DISCARD;
   vals[ 9 ] = nll::tutorial::DatabaseDecoderEncoder::CLASS;
   for ( unsigned n = 1; n < 9; ++n )
      vals[ n ] = nll::tutorial::DatabaseDecoderEncoder::FLOATING;
   nll::tutorial::DatabaseDecoderEncoder databaseCreator;
   std::ifstream f( NLL_DATABASE_PATH "uci/yeast/yeast.data" );
   ensure( f.good(), "error, can't find database: yeast.data" );
   nll::benchmark::BenchmarkDatabases::Database dat = databaseCreator.compute( vals, f );
   nll::benchmark::BenchmarkDatabases::instance().add(
      nll::benchmark::BenchmarkDatabases::Benchmark( "yeast.data",
                                                      dat,
                                                      "UCI yeast database, best accuracy:http://www.cs.cmu.edu/~epxing/Class/10701-06f/project-reports/buck_zhang.pdf 74.6%",
                                                      0.746 ) );
}

STATIC_PLUG( loadYeastDatabase );