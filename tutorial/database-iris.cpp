#include "stdafx.h"
#include "database-benchmark.h"
#include "static-plug.h"
#include "decoder.h"

void loadIrisDatabase()
{
   nll::tutorial::DatabaseDecoderEncoder::AttributValues vals( 5 );
   vals[ 4 ] = nll::tutorial::DatabaseDecoderEncoder::CLASS;
   for ( unsigned n = 0; n < 4; ++n )
      vals[ n ] = nll::tutorial::DatabaseDecoderEncoder::FLOATING;
   nll::tutorial::DatabaseDecoderEncoder databaseCreator;
   std::ifstream f( NLL_DATABASE_PATH "uci/iris/bezdekIris.data" );
   ensure( f.good(), "error, can't find database: bezdekIris.data" );
   nll::benchmark::BenchmarkDatabases::Database dat = databaseCreator.compute( vals, f, ',' );
   nll::benchmark::BenchmarkDatabases::instance().add(
      nll::benchmark::BenchmarkDatabases::Benchmark( "iris.data",
                                                      dat,
                                                      "http://archive.ics.uci.edu/ml/machine-learning-databases/iris/",
                                                      0.746 ) );
}

STATIC_PLUG( loadIrisDatabase );
