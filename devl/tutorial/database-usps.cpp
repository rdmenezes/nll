#include "stdafx.h"
#include "load-database.h"
#include "static-plug.h"

using namespace nll;
typedef benchmark::BenchmarkDatabases::Database Database;

namespace
{
   Database loadUspsData( const std::string& path,
                          Database::Sample::Type type )
   {
      Database dat;
      std::ifstream f( path.c_str() );
      if ( !f.good() )
      {
         throw std::runtime_error( "cannot load the dataset:" + path );
      }

      std::string header;
      std::getline( f, header );

      std::stringstream ss( header );
      ui32 nbClasses = 0;
      ui32 nbFeatures = 0;
      ss >> nbClasses;
      ss >> nbFeatures;

      while ( !f.eof() )
      {
         std::string line;
         std::getline( f, line );
         std::stringstream ss( line );

         Database::Sample::Input i( nbFeatures );
         int output;
         ss >> output;
         if ( output == -1 )
            break;
         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            double val = 0;
            ss >> val;
            i[ n ] = val;
         }

         dat.add( Database::Sample( i, output, type ) );
      }
      return dat;
   }
}

void loadUSPSDatabase()
{
   const std::string pathT = NLL_DATABASE_PATH "usps/usps_test.txt";
   const std::string pathL = NLL_DATABASE_PATH "usps/usps_train.txt";

   Database test     = loadUspsData( pathT, Database::Sample::TESTING );
   Database learning = loadUspsData( pathL, Database::Sample::LEARNING );

   for ( ui32 n = 0; n < test.size(); ++n )
      learning.add( test[ n ] );
   nll::benchmark::BenchmarkDatabases::Benchmark benchmark( "usps", learning, "USPS dataset", -0 );
   nll::benchmark::BenchmarkDatabases::instance().add( benchmark );
}

STATIC_PLUG( loadUSPSDatabase );