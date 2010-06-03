#ifndef READ_RESULT_H_
# define READ_RESULT_H_

# include <vector>
# include <map>
# include <fstream>
# include "globals.h"
# include "features.h"

namespace nll
{
namespace detect
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

      static void generateSourceDatabase( const std::string& input_cfg, const std::string& outputDatabase, ui32 sliceIncrement = 5 )
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
            nll::imaging::VolumeSpatial<float> volume;
            const std::string filename = std::string( DATA_PATH "case" ) + core::val2str( results[ n ].id ) + ".mf2";

            std::cout << "generating case:" << filename << std::endl;
            bool loaded = loadSimpleFlatFile( filename, volume );
            ensure( loaded, "error: can't load case:" + filename );

            // export
            ui32 size = volume.size()[ 2 ];
            for ( ui32 nn = 1; nn < size; nn += sliceIncrement )
            {
               if ( fabs( nn - results[ n ].neckStart ) < sliceIncrement ||
                    fabs( nn - results[ n ].lungStart ) < sliceIncrement ||
                    fabs( nn - results[ n ].heartStart ) < sliceIncrement )
               {
                  // skip: too close from the marker
                  continue;
               }
               core::vector3f center = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                               volume.size()[ 1 ] / 2.0f,
                                                                               nn ) );
               core::Image<ui8> mpr_xy = extractSlice( volume, center[ 2 ] );

               //
               // transform to features
               //

               core::extend( mpr_xy, 3 );
               writeBmp( mpr_xy, std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( nn ) + ".bmp" );

               /*
               Point input = 

               Point output( 1 );
               output[ 0 ] = results[ n ].neckStart / volume.size()[ 2 ];

               output.print( std::cout );

               database.add( Sample( input, output, n < 53 ? Sample::LEARNING : Sample::TESTING, core::make_buffer1D_from_string( filename ) ) );
               */
            }
         }

         database.write( outputDatabase );
      }
   };
}
}

#endif