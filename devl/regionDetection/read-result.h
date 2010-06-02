#ifndef READ_RESULT_H_
# define READ_RESULT_H_

# include <vector>
# include <map>
# include <fstream>
# include "globals.h"
# include "create-features.h"

namespace nll
{
namespace utility
{
   class RegionResult
   {
      struct Result
      {
         ui32  id;
         float  neckStart;
         float  neckEnd;
         float  heartStart;
         float  heartEnd;
         float  lungStart;
         float  lungEnd;
      };

   public:
      typedef core::Buffer1D<double>                     Point;
      typedef core::ClassificationSample<Point, Point>   Sample;
      typedef core::Database<Sample>                     Database;

      static void generateSourceDatabase( const std::string& input_cfg, const std::string& outputDatabase )
      {
         std::ifstream f( input_cfg.c_str() );
         if ( !f.good() )
            throw std::exception( "can't find input results" );

         // read the cases
         std::vector<Result> results;
         while ( !f.eof() )
         {
            Result r;

            std::string line;
            std::getline( f, line );
            sscanf( line.c_str(), "case%u:", &r.id );

            std::getline( f, line );
            unsigned int n1;
            unsigned int n2;
            sscanf( line.c_str(), "neck:%u:%u", &n1, &n2 );
            r.neckStart = static_cast<float>( n1 );
            r.neckEnd = static_cast<float>( n2 );

            std::getline( f, line );
            sscanf( line.c_str(), "heart:%u:%u", &n1, &n2 );
            r.heartStart = static_cast<float>( n1 );
            r.heartEnd = static_cast<float>( n2 );

            std::getline( f, line );
            sscanf( line.c_str(), "lung:%u:%u", &n1, &n2 );
            r.lungStart = static_cast<float>( n1 );
            r.lungEnd = static_cast<float>( n2 );

            std::getline( f, line );
            results.push_back( r );
         }

         // generate the database
         Database database;
         for ( size_t n = 0; n < results.size(); ++n )
         {
            // normalize images
            nll::imaging::VolumeSpatial<float> volume;
            const std::string filename = std::string( DATA_PATH "case" ) + core::val2str( results[ n ].id ) + ".mf2";

            std::cout << "generating case:" << filename << std::endl;
            bool loaded = loadSimpleFlatFile( filename, volume );
            ensure( loaded, "error: can't load case:" + filename );

            // export
            Point input = createFeatures( volume );

            Point output( 3 );
            output[ 0 ] = results[ n ].neckStart / volume.size()[ 2 ];
            //output[ 1 ] = results[ n ].neckEnd / volume.size()[ 2 ];
            output[ 1 ] = results[ n ].heartStart / volume.size()[ 2 ];
            //output[ 3 ] = results[ n ].heartEnd / volume.size()[ 2 ];
            output[ 2 ] = results[ n ].lungStart / volume.size()[ 2 ];
            //output[ 5 ] = results[ n ].lungEnd / volume.size()[ 2 ];

            output.print( std::cout );

            database.add( Sample( input, output, n < 53 ? Sample::LEARNING : Sample::TESTING, core::make_buffer1D_from_string( filename ) ) );
         }

         database.write( outputDatabase );
      }
   };
}
}

#endif