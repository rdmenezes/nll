#ifndef READ_RESULT_H_
# define READ_RESULT_H_

# include <vector>
# include <map>
# include <fstream>
# include "globals.h"
# include "features.h"

# pragma warning( push )
# pragma warning( disable:4996 ) // unsafe functions

namespace nll
{
namespace detect
{
   class RegionResult
   {
   public:
      struct Result
      {
         ui32  id;
         float  neckStart;
         float  neckEnd;
         float  heartStart;
         float  heartEnd;
         float  lungStart;
         float  lungEnd;
         float  skullStart;
      };

   public:
      typedef core::Buffer1D<double>                     Point;
      typedef core::ClassificationSample<Point, ui32>    Sample;
      typedef core::Database<Sample>                     Database;

      static std::vector<Result> readResults( const std::string& input_cfg )
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
            float n1;
            float n2;
            sscanf( line.c_str(), "neck:%f:%f", &n1, &n2 );
            r.neckStart = static_cast<float>( n1 );
            r.neckEnd = static_cast<float>( n2 );

            std::getline( f, line );
            sscanf( line.c_str(), "heart:%f:%f", &n1, &n2 );
            r.heartStart = static_cast<float>( n1 );
            r.heartEnd = static_cast<float>( n2 );

            std::getline( f, line );
            sscanf( line.c_str(), "lung:%f:%f", &n1, &n2 );
            r.lungStart = static_cast<float>( n1 );
            r.lungEnd = static_cast<float>( n2 );

            std::getline( f, line );
            sscanf( line.c_str(), "skull:%f:%f", &n1, &n2 );
            r.skullStart = static_cast<float>( n1 );

            std::getline( f, line );
            results.push_back( r );
         }

         return results;
      }

      static void generateSourceDatabase( const std::string& input_cfg, const std::string& outputDatabase )
      {
         // read the cases
         std::vector<Result> results = readResults( input_cfg );
         

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
            for ( ui32 nn = 0; nn < size; nn += DATABASE_MIN_INTERVAL )
            {
               if ( fabs( nn - results[ n ].neckStart  ) < DATABASE_MIN_INTERVAL_ROI ||
                    fabs( nn - results[ n ].lungStart  ) < DATABASE_MIN_INTERVAL_ROI ||
                    fabs( nn - results[ n ].heartStart ) < DATABASE_MIN_INTERVAL_ROI ||
                    fabs( nn - results[ n ].skullStart ) < DATABASE_MIN_INTERVAL_ROI )
               {
                  // skip: too close from the marker
                  continue;
               }
               

               const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( nn ) + ".bmp";
               Point input = _convert( volume, nn, sliceName );

               // we know all these slices are not what we are looking for...
               ui32 output = 0;
               database.add( Sample( input, output, Sample::LEARNING, core::make_buffer1D_from_string( filename + "-slice-" + core::val2str( nn ) ) ) );
            }

            // export the interesting slices
            // note that we need to substract 1 for each slice, as the file is based on 1-XX notation
            if ( results[ n ].neckStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].neckStart ) + "-neck.bmp";
                  Point input = _convert( volume, (ui32)results[ n ].neckStart - 1, sliceName );
                  ui32 output = 1;
                  database.add( Sample( input, output, Sample::LEARNING, core::make_buffer1D_from_string( filename + "-slice-" + core::val2str( results[ n ].neckStart ) ) ) );
               }
            }

            if ( results[ n ].heartStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].heartStart - 1 ) + "-heart.bmp";
                  Point input = _convert( volume, (ui32)results[ n ].heartStart - 1, sliceName );
                  ui32 output = 2;
                  database.add( Sample( input, output, Sample::LEARNING, core::make_buffer1D_from_string( filename + "-slice-" + core::val2str( results[ n ].heartStart ) ) ) );
               }
            }

            if ( results[ n ].lungStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].lungStart - 1 ) + "-lung.bmp";
                  Point input = _convert( volume, (ui32)results[ n ].lungStart - 1, sliceName );
                  ui32 output = 3;
                  database.add( Sample( input, output, Sample::LEARNING, core::make_buffer1D_from_string( filename + "-slice-" + core::val2str( results[ n ].lungStart ) ) ) );
               }
            }

            if ( results[ n ].skullStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].skullStart ) + "-skull.bmp";
                  ui32 index = std::min( (ui32)results[ n ].skullStart - 1, volume.size()[ 2 ] - 2 );
                  if ( index < volume.size()[ 2 ] )
                  {
                     Point input = _convert( volume, index, sliceName );  // we can't take the first slice as it is empty due to interpolation...
                     ui32 output = 4;
                     database.add( Sample( input, output, Sample::LEARNING, core::make_buffer1D_from_string( filename + "-slice-" + core::val2str( results[ n ].skullStart ) ) ) );
                  }
               }
            }
         }

         database.write( outputDatabase );
      }

      static void generateFeatureDatabase()
      {
         std::cout << "generate feature database..." << std::endl;
         algorithm::Haar2dFeatures::Features features = _generateRandomFeatures();
         algorithm::Haar2dFeatures::write( features, HAAR_FEATURES );

         std::cout << " read source database..." << std::endl;
         Database sourceDatabase;
         sourceDatabase.read( DATABASE_SOURCE );

         std::cout << " computes haar features..." << std::endl;
         Database haarDatabase;
         ui32 nbLearning = static_cast<ui32>( 0.75f * sourceDatabase.size() );
         for ( ui32 n = 0; n < sourceDatabase.size(); ++n )
         {
            // recreate the normalized image
            core::Image<Point::value_type> image( sourceDatabase[ n ].input, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y, 1 );

            // process it
            Point point = algorithm::Haar2dFeatures::process( features, image );

            Database::Sample::Type type = ( n < nbLearning ) ? Database::Sample::LEARNING: Database::Sample::TESTING;
            haarDatabase.add( Database::Sample( point, sourceDatabase[ n ].output, type, sourceDatabase[ n ].debug ) );
         }

         haarDatabase.write( DATABASE_HAAR );

         std::cout << " normalize data..." << std::endl;
         algorithm::FeatureTransformationNormalization<Point> featureNormalization;
         featureNormalization.compute( haarDatabase );
         featureNormalization.write( PREPROCESSING_HAAR );

         Database haarDatabaseNormalized = featureNormalization.transform( haarDatabase );
         haarDatabaseNormalized.write( NORMALIZED_HAAR );

      }

   private:
      static algorithm::Haar2dFeatures::Features _generateRandomFeatures( int seed = 0 )
      {
         algorithm::Haar2dFeatures::Features features;

         srand( seed );
         for ( ui32 n = 0; n < HAAR_FEATURE_SIZE; ++n )
         {
            core::vector2d v1( core::generateUniformDistribution( 0, 1 ),
                               core::generateUniformDistribution( 0, 1 ) );
            core::vector2d v2( core::generateUniformDistribution( 0, 1 ),
                               core::generateUniformDistribution( 0, 1 ) );
            if ( fabs( v1[ 0 ] - v2[ 0 ] ) < 10.0 / REGION_DETECTION_SOURCE_IMG_X ||    // allow a minimum of 4 pixels features
                 fabs( v1[ 1 ] - v2[ 1 ] ) < 10.0 / REGION_DETECTION_SOURCE_IMG_Y )     //
            {
               --n;
               continue;
            }
            if ( v1[ 0 ] > v2 [ 0 ] )
               std::swap( v1[ 0 ], v2[ 0 ] );
            if ( v1[ 1 ] > v2 [ 1 ] )
               std::swap( v1[ 1 ], v2[ 1 ] );
            ui32 dir = rand() % 2;

            features.push_back( algorithm::Haar2dFeatures::Feature( (algorithm::Haar2dFeatures::Feature::Direction)dir,
                                                                v1,
                                                                v2 ) );
         }
         return features;
      }

      static Point _convert( const Volume& volume, ui32 sliceIndex, const std::string& sliceName )
      {
         core::vector3f center = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                         volume.size()[ 1 ] / 2.0f,
                                                                         static_cast<f32>( sliceIndex ) ) );
         core::Image<ui8> mpr_xy = extractSlice( volume, center[ 2 ] );

         //
         // transform to features
         //
         Point result( mpr_xy.size(), false );
         for ( ui32 n = 0; n < result.size(); ++n )
         {
            result[ n ] = static_cast<Point::value_type>( mpr_xy[ n ] ) / 256.0;
         }

         // export the slice
         core::extend( mpr_xy, 3 );
         writeBmp( mpr_xy, sliceName );
         return result;
      }
   };
}
}

# pragma warning( pop )

#endif