#include "stdafx.h"
#include "load-database.h"
#include "static-plug.h"

void loadCancerDatabase()
{
   nll::benchmark::EncoderClassification encoder;

   {
      nll::benchmark::ProbenDatabase::Database dat = nll::benchmark::ProbenDatabase::load( PROBEN_PATH + std::string( "cancer/cancer1.dt" ), &encoder );
      nll::benchmark::BenchmarkDatabases::Benchmark benchmark( "cancer1.dt", dat, "wisconsin breast cancer database, partition1", 0 );
      nll::benchmark::BenchmarkDatabases::instance().add( benchmark );
   }

   {
      nll::benchmark::ProbenDatabase::Database dat = nll::benchmark::ProbenDatabase::load( PROBEN_PATH + std::string( "cancer/cancer2.dt" ), &encoder );
      nll::benchmark::BenchmarkDatabases::Benchmark benchmark( "cancer2.dt", dat, "wisconsin breast cancer database, partition2", 0 );
      nll::benchmark::BenchmarkDatabases::instance().add( benchmark );
   }

   {
      nll::benchmark::ProbenDatabase::Database dat = nll::benchmark::ProbenDatabase::load( PROBEN_PATH + std::string( "cancer/cancer3.dt" ), &encoder );
      nll::benchmark::BenchmarkDatabases::Benchmark benchmark( "cancer3.dt", dat, "wisconsin breast cancer database, partition3", 0 );
      nll::benchmark::BenchmarkDatabases::instance().add( benchmark );
   }
}

STATIC_PLUG( loadCancerDatabase );
