#include "stdafx.h"
#include "database-benchmark.h"
#include "static-plug.h"
#include "decoder.h"

void loadWineDatabase()
{
   nll::tutorial::DatabaseDecoderEncoder::AttributValues vals( 14 );
   vals[ 0 ] = nll::tutorial::DatabaseDecoderEncoder::CLASS;
   for ( unsigned n = 1; n < 14; ++n )
      vals[ n ] = nll::tutorial::DatabaseDecoderEncoder::FLOATING;
   nll::tutorial::DatabaseDecoderEncoder databaseCreator;
   std::ifstream f( NLL_DATABASE_PATH "uci/wine/wine.data" );
   ensure( f.good(), "error, can't find database: wine.data" );
   nll::benchmark::BenchmarkDatabases::Database dat = databaseCreator.compute( vals, f,',' );
   nll::benchmark::BenchmarkDatabases::instance().add(
      nll::benchmark::BenchmarkDatabases::Benchmark( "wine.data",
                                                      dat,
                                                      "UCI wine database. 100% accuracy using LDA",
                                                      1.0) );
}

STATIC_PLUG( loadWineDatabase );
