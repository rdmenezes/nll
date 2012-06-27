#include "stdafx.h"
#include "database-benchmark.h"
#include "static-plug.h"
#include "decoder.h"

void loadSpamDatabase()
{
   nll::tutorial::DatabaseDecoderEncoder::AttributValues vals( 58 );
   vals[ 57 ] = nll::tutorial::DatabaseDecoderEncoder::CLASS;
   for ( unsigned n = 0; n < 57; ++n )
      vals[ n ] = nll::tutorial::DatabaseDecoderEncoder::FLOATING;
   nll::tutorial::DatabaseDecoderEncoder databaseCreator;
   std::ifstream f( NLL_DATABASE_PATH "uci/spam/spambase.data" );
   ensure( f.good(), "error, can't find database: spambase.data" );
   nll::benchmark::BenchmarkDatabases::Database dat = databaseCreator.compute( vals, f, ',' );
   nll::benchmark::BenchmarkDatabases::instance().add(
      nll::benchmark::BenchmarkDatabases::Benchmark( "spambase.data",
                                                      dat,
                                                      "UCI: 7%error, 20-25% with 0 ",
                                                      0.746 ) );
}

STATIC_PLUG( loadSpamDatabase );
