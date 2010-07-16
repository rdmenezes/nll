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
      struct      Measure
      {
         Measure( ui32 i, ui32 nb, float h ) : id( i ), numberOfSlices( nb ), height( h )
         {
            heightNeck  = -1;
            heightHeart = -1;
            heightLung  = -1;
            heightSkull = -1;
            heightHips  = -1;
         }

         Measure()
         {
            heightNeck  = -1;
            heightHeart = -1;
            heightLung  = -1;
            heightSkull = -1;
            heightHips  = -1;
         }

         core::Buffer1D<float> toArray() const
         {
            return core::make_buffer1D<float>( 0,
                                               heightNeck,
                                               heightHeart,
                                               heightLung,
                                               heightSkull,
                                               heightHips );
         }

         ui32     id;
         ui32     numberOfSlices;
         float    height;              // in mm

         float    heightNeck;          // in mm, 0 correspond to the first slice, -1 not defined
         float    heightHeart;         // in mm, 0 correspond to the first slice, -1 not defined
         float    heightLung;          // in mm, 0 correspond to the first slice, -1 not defined
         float    heightSkull;         // in mm, 0 correspond to the first slice, -1 not defined
         float    heightHips;          // in mm, 0 correspond to the first slice, -1 not defined
      };

      struct    Result
      {
         ui32   id;
         float  neckStart;
         float  neckEnd;
         float  heartStart;
         float  heartEnd;
         float  lungStart;
         float  lungEnd;
         float  skullStart;
         float  hipsStart;
      };

   public:
      typedef core::Buffer1D<double>                     Point;
      typedef core::ClassificationSample<Point, ui32>    Sample;
      typedef core::Database<Sample>                     Database;

      static void writeMeasures( const std::vector<Measure>& m, const std::string& input_cfg )
      {
         std::ofstream f( input_cfg.c_str() );
         if ( !f.good() )
            throw std::exception( "can't find output results" );

         for ( ui32 n = 0; n < (ui32)m.size(); ++n )
         {
            f << "case" << m[ n ].id << ":" << std::endl;
            f << "numberOfSlices:" << m[ n ].numberOfSlices << std::endl;
            f << "height:" << m[ n ].height << std::endl;
            f << "heightNeck:"  << m[ n ].heightNeck  << std::endl;
            f << "heightHeart:" << m[ n ].heightHeart << std::endl;
            f << "heightLung:"  << m[ n ].heightLung  << std::endl;
            f << "heightSkull:" << m[ n ].heightSkull << std::endl;
            f << "heightHips:"  << m[ n ].heightHips;
            if ( ( n + 1 ) != m.size() )
            {
               f << std::endl;
               f << std::endl;
            }
         }
      }

      static std::vector<Measure> readMeasures( const std::string& input_cfg )
      {
         std::ifstream f( input_cfg.c_str() );
         if ( !f.good() )
            throw std::exception( "can't find input results" );

         // read the cases
         std::vector<Measure> results;
         while ( !f.eof() )
         {
            Measure r;

            std::string line;
            std::getline( f, line );
            sscanf( line.c_str(), "case%u:", &r.id );

            std::getline( f, line );
            float n1;
            ui32  intval;
            sscanf( line.c_str(), "numberOfSlices:%u", &intval );
            r.numberOfSlices = intval;


            std::getline( f, line );
            sscanf( line.c_str(), "height:%f", &n1 );
            r.height = static_cast<float>( n1 );



            std::getline( f, line );
            sscanf( line.c_str(), "heightNeck:%f", &n1 );
            r.heightNeck = static_cast<float>( n1 );

            std::getline( f, line );
            sscanf( line.c_str(), "heightHeart:%f", &n1 );
            r.heightHeart = static_cast<float>( n1 );

            std::getline( f, line );
            sscanf( line.c_str(), "heightLung:%f", &n1 );
            r.heightLung = static_cast<float>( n1 );

            std::getline( f, line );
            sscanf( line.c_str(), "heightSkull:%f", &n1 );
            r.heightSkull = static_cast<float>( n1 );

            std::getline( f, line );
            sscanf( line.c_str(), "heightHips:%f", &n1 );
            r.heightHips = static_cast<float>( n1 );

            std::getline( f, line );
            results.push_back( r );
         }

         return results;
      }

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
            r.neckStart = static_cast<float>( n1 ) - 1;
            r.neckEnd = static_cast<float>( n2 ) - 1;

            std::getline( f, line );
            sscanf( line.c_str(), "heart:%f:%f", &n1, &n2 );
            r.heartStart = static_cast<float>( n1 ) - 1;
            r.heartEnd = static_cast<float>( n2 ) - 1;

            std::getline( f, line );
            sscanf( line.c_str(), "lung:%f:%f", &n1, &n2 );
            r.lungStart = static_cast<float>( n1 ) - 1;
            r.lungEnd = static_cast<float>( n2 ) - 1;

            std::getline( f, line );
            sscanf( line.c_str(), "skull:%f:%f", &n1, &n2 );
            r.skullStart = static_cast<float>( n1 ) - 2;          // TODO -1 bias

            std::getline( f, line );
            sscanf( line.c_str(), "hips:%f:%f", &n1, &n2 );
            r.hipsStart = static_cast<float>( n1 ) - 1;

            std::getline( f, line );
            results.push_back( r );
         }

         return results;
      }

      //
      // ID:
      //   0 : nothing
      //   1 : neck
      //   2 : heart
      //   3 : lung
      //   4 : skull
      //   5 : hips
      //
      static void generateSourceDatabase( const std::string& input_cfg, const std::string& outputDatabase )
      {
         // read the cases
         std::vector<Result> results = readResults( input_cfg );
         std::vector<Measure> measures;
         

         // generate the database
         Database database;
         for ( size_t n = 0; n < results.size(); ++n )
         {
            nll::imaging::VolumeSpatial<float> volume;
            const std::string filename = std::string( DATA_PATH "case" ) + core::val2str( results[ n ].id ) + ".mf2";

            std::cout << "generating case:" << filename << std::endl;
            bool loaded = loadSimpleFlatFile( filename, volume );
            ensure( loaded, "error: can't load case:" + filename );

            Measure measure( results[ n ].id, volume.size()[ 2 ], volume.size()[ 2 ] * volume.getSpacing()[ 2 ] );

            // export
            ui32 size = volume.size()[ 2 ];
            for ( ui32 nn = 0; nn < size; nn += DATABASE_MIN_INTERVAL )
            {
               std::string sliceDebugName = ( nn == 0 ) ? core::val2str( results[ n ].id ) : "";
               if ( fabs( nn - results[ n ].neckStart  ) < DATABASE_MIN_INTERVAL_ROI ||
                    fabs( nn - results[ n ].lungStart  ) < DATABASE_MIN_INTERVAL_ROI ||
                    fabs( nn - results[ n ].heartStart ) < DATABASE_MIN_INTERVAL_ROI ||
                    fabs( nn - results[ n ].skullStart ) < DATABASE_MIN_INTERVAL_ROI ||
                    fabs( nn - results[ n ].hipsStart  ) < DATABASE_MIN_INTERVAL_ROI )
               {
                  // skip: too close from the marker
                  continue;
               }
               

               const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( nn ) + ".bmp";
               Point input = _convert( volume, nn, sliceName );

               // we know all these slices are not what we are looking for...
               ui32 output = 0;

               // for the skull: specific rule: all slices between start of the neck + DATABASE_MIN_INTERVAL_ROI and base of the skull will have 'skull' label
               if ( results[ n ].skullStart > 0 &&
                    results[ n ].neckStart > 0 &&
                    nn >=  ( results[ n ].neckStart + DATABASE_MIN_INTERVAL_ROI ) &&
                    nn <= results[ n ].skullStart )
               {
                  continue; // skip the data
               }

               std::cout << "case:" << results[ n ].id << " slice=" << nn << " class=" << output << std::endl;
               database.add( Sample( input, output, Sample::LEARNING, core::make_buffer1D_from_string( sliceDebugName ) ) );
            }

            // export the interesting slices
            // note that we need to substract 1 for each slice, as the file is based on 1-XX notation
            if ( results[ n ].neckStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].neckStart ) + "-neck.bmp";
                  Point input = _convert( volume, (ui32)results[ n ].neckStart, sliceName );
                  ui32 output = 1;
                  database.add( Sample( input, output, Sample::LEARNING ) );
               }

               // measure the heigh of the ROI relatively to the volume origin
               core::vector3f pos = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                            volume.size()[ 1 ] / 2.0f,
                                                                            results[ n ].neckStart ) );
               measure.heightNeck = pos[ 2 ] - volume.getOrigin()[ 2 ];
               std::cout << "heightNeck=" << measure.heightNeck << std::endl;
            }

            if ( results[ n ].heartStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].heartStart ) + "-heart.bmp";
                  Point input = _convert( volume, (ui32)results[ n ].heartStart, sliceName );
                  ui32 output = 2;
                  database.add( Sample( input, output, Sample::LEARNING ) );
               }

               // measure the heigh of the ROI relatively to the volume origin
               core::vector3f pos = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                            volume.size()[ 1 ] / 2.0f,
                                                                            (f32)results[ n ].heartStart ) );
               measure.heightHeart = pos[ 2 ] - volume.getOrigin()[ 2 ];
               std::cout << "heightHeart=" << measure.heightHeart << std::endl;
            }

            if ( results[ n ].lungStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].lungStart ) + "-lung.bmp";
                  Point input = _convert( volume, (ui32)results[ n ].lungStart, sliceName );
                  ui32 output = 3;
                  database.add( Sample( input, output, Sample::LEARNING ) );
               }

               // measure the heigh of the ROI relatively to the volume origin
               core::vector3f pos = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                            volume.size()[ 1 ] / 2.0f,
                                                                            (f32)results[ n ].lungStart ) );
               measure.heightLung = pos[ 2 ] - volume.getOrigin()[ 2 ];
               std::cout << "heightLung=" << measure.heightLung << std::endl;
            }

            if ( results[ n ].skullStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].skullStart ) + "-skull.bmp";
                  ui32 index = std::min( (ui32)results[ n ].skullStart, volume.size()[ 2 ] - 1 );
                  if ( index < volume.size()[ 2 ] )
                  {
                     Point input = _convert( volume, index, sliceName );  // we can't take the first slice as it is empty due to interpolation...
                     ui32 output = 4;
                     database.add( Sample( input, output, Sample::LEARNING ) );
                  }
               }

               /*
               // create additional slice for the skull
               if ( results[ n ].neckStart > 0 && ( results[ n ].skullStart - results[ n ].neckStart ) > 40 )
               {
                  ui32 sl1 = (ui32)( results[ n ].skullStart - 1 * ( results[ n ].skullStart - results[ n ].neckStart ) / 4 );
                  ui32 sl2 = (ui32)( results[ n ].skullStart - 2 * ( results[ n ].skullStart - results[ n ].neckStart ) / 4 );

                  const std::string sliceName1 = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].skullStart ) + "-skull-add1.bmp";
                  Point input = _convert( volume, sl1, sliceName1 );  // we can't take the first slice as it is empty due to interpolation...
                  ui32 output = 4;
                  database.add( Sample( input, output, Sample::LEARNING ) );

                  const std::string sliceName2 = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].skullStart ) + "-skull-add2.bmp";
                  input = _convert( volume, sl2, sliceName2 );  // we can't take the first slice as it is empty due to interpolation...
                  database.add( Sample( input, output, Sample::LEARNING ) );
               }*/

               // measure the heigh of the ROI relatively to the volume origin
               core::vector3f pos = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                            volume.size()[ 1 ] / 2.0f,
                                                                            (f32)results[ n ].skullStart ) );
               measure.heightSkull = pos[ 2 ] - volume.getOrigin()[ 2 ];
               std::cout << "heightSkull=" << measure.heightSkull << std::endl;
            }

            if ( results[ n ].hipsStart > 0 )
            {
               {
                  const std::string sliceName = std::string( "c:/tmp/case-" ) + core::val2str( n ) + "-slice-" + core::val2str( results[ n ].lungStart ) + "-hips.bmp";
                  Point input = _convert( volume, (ui32)results[ n ].hipsStart, sliceName );
                  ui32 output = 5;
                  database.add( Sample( input, output, Sample::LEARNING ) );
               }

               // measure the heigh of the ROI relatively to the volume origin
               core::vector3f pos = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                            volume.size()[ 1 ] / 2.0f,
                                                                            (f32)results[ n ].hipsStart ) );
               measure.heightHips = pos[ 2 ] - volume.getOrigin()[ 2 ];
               std::cout << "heightHips=" << measure.heightHips << std::endl;
            }

            measures.push_back( measure );
         }

         writeMeasures( measures, DATABASE_MEASURES );
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
         for ( ui32 n = 0; n < sourceDatabase.size(); ++n )
         {
            // recreate the normalized image
            core::Image<Point::value_type> image( sourceDatabase[ n ].input, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y, 1 );

            // process it
            Point point = algorithm::Haar2dFeatures::process( features, image );

            Database::Sample::Type type = Database::Sample::LEARNING;
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
      static algorithm::Haar2dFeatures::Features _generateRandomFeatures()
      {
         algorithm::Haar2dFeatures::Features features;

         for ( ui32 n = 0; n < HAAR_FEATURE_SIZE; ++n )
         {
            core::vector2d v1( core::generateUniformDistribution( 0, 1 ),
                               core::generateUniformDistribution( 0, 1 ) );
            core::vector2d v2( core::generateUniformDistribution( 0, 1 ),
                               core::generateUniformDistribution( 0, 1 ) );
            if ( fabs( v1[ 0 ] - v2[ 0 ] ) < 10.0 / REGION_DETECTION_SOURCE_IMG_X ||    // allow a minimum of XX pixels features
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