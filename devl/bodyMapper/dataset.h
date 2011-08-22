#ifndef MVV_MAPPER_DATASET_H_
# define MVV_MAPPER_DATASET_H_

# include "def.h"
# include <nll/nll.h>

#define LANDMARK_DEFAULT_INDEX      "../../bodyMapper/index/index.txt"
#define LANDMARK_DEFAULT_DIR        "../../bodyMapper/landmarks/"
#define LANDMARK_DATA_DEFAULT_DIR   "C:/DicomDataRepositery/"
#define LANDMARK_NB                 17
namespace mvv
{
namespace mapper
{
   class BODYMAPPER_API LandmarkDataset
   {
   public:
      typedef nll::imaging::VolumeSpatial<float>   Volume;
      typedef nll::core::vector3f                  vector3f;

      /**
       @brief Stores a collection of landmarks
       */
      struct Dataset
      {
         /// landmarks, position is indicated in voxels
         std::vector<vector3f> landmarks;

         /// this may not be exhaustive and will depend on the landmark definitions, we can use it as shortcut only...
         enum LandmarkId
         {
            NOTHING                      = 0,   // this is an empty landmark
            LD_DENS                      = 1,
            LD_LUNGR                     = 2,
            LD_LUNGL                     = 3,
            LD_AORTIC_ARCH               = 4,
            LD_HUMERUSR                  = 5,
            LD_HUMERUSL                  = 6,
            LD_SBLADER                   = 7,
            LD_SBLADEL                   = 8,
            LD_HEART_VEIN_CAVE_ENTRANCE  = 9,
            LD_CARINA                    = 10,
            LD_PORTAL_VEIN               = 11,
            LD_SPLEEN_HEAD               = 12,
            LD_KIDNEYR                   = 13,
            LD_KIDNEYL                   = 14,
            LD_HIPS                      = 15,
            LD_FEMURR                    = 16,
            LD_FEMURL                    = 17,
            LD_COCCYX                    = 18
         };

         bool isValid( unsigned landmarkId ) const
         {
            return landmarkId < landmarks.size() && landmarks[ landmarkId ][ 0 ] >= 0 &&
                                                    landmarks[ landmarkId ][ 1 ] >= 0 &&
                                                    landmarks[ landmarkId ][ 2 ] >= 0;
         }

         unsigned size() const
         {
            return (unsigned)landmarks.size();
         }

         const vector3f& operator[]( unsigned id ) const
         {
            return landmarks[ id ];
         }

         void read( const std::string& file )
         {
            std::ifstream f( file.c_str() );
            if ( !f.good() )
            {
               throw std::runtime_error( "cannot read landmark data:" + file );
            }

            std::string line;
            std::getline( f, line );
            std::vector<const char*> splits = nll::core::split( line, '=' );
            ensure( splits.size() == 2, "expecting nbFeatures=XXX" );
            ensure( std::string( splits[ 0 ] ) == "nbFeatures", "expecting nbFeatures=XXX" );

            unsigned nbLandmarks = nll::core::str2val<unsigned>( splits[ 1 ] ) + 1; // we need to reserve landmark[ 0 ] = NO LANDMARK
            landmarks.resize( nbLandmarks );

            for ( unsigned n = 0; n < nbLandmarks; ++n )
            {
               landmarks[ n ] = vector3f( -1, -1, -1 );
            }

            for ( unsigned n = 0; n < nbLandmarks - 1; ++n )
            {
               std::getline( f, line );
               std::vector<const char*> splits = nll::core::split( line, '=' );
               ensure( splits.size() == 2, "expecting id-XX=XXX" );
               
               std::string left( splits[ 0 ] );
               std::vector<const char*> splitsLandmarkId = nll::core::split( left, '-' );
               ensure( splitsLandmarkId.size() == 2, "expecting id-XX=XXX" );
               unsigned landmarkId = nll::core::str2val<unsigned>( splitsLandmarkId[ 1 ] );

               std::string right( splits[ 1 ] );
               std::vector<const char*> splitsLandmarkPos = nll::core::split( right, ',' );
               ensure( splitsLandmarkPos.size() == 3, "expecting id-XX=XXX, XXX, XXX" );

               vector3f pos( nll::core::str2val<float>( splitsLandmarkPos[ 0 ] ),
                             nll::core::str2val<float>( splitsLandmarkPos[ 1 ] ),
                             nll::core::str2val<float>( splitsLandmarkPos[ 2 ] ) );
               landmarks[ landmarkId ] = pos;
            }
         }
      };

      LandmarkDataset( const std::string& index                = LANDMARK_DEFAULT_INDEX,
                       const std::string& landmarksDirectory   = LANDMARK_DEFAULT_DIR,
                       const std::string& dataDirectory        = LANDMARK_DATA_DEFAULT_DIR )
      {
         _populatePath( index, landmarksDirectory, dataDirectory );
      }

      unsigned size() const
      {
         return (unsigned) _dataPath.size();
      }

      std::auto_ptr<Volume> loadData( unsigned id ) const
      {
         return std::auto_ptr<Volume>();
      }

   private:
      std::string _dirToLandmarkId( const std::string& dir )
      {
         std::string id = dir;
         for ( size_t n = 0; n < id.size(); ++n )
         {
            if ( id[ n ] == '_' || id[ n ] == '/' || id[ n ] == '\\' )
            {
               id[ n ] = '-';
            }
         }
         return id;
      }

      void _populatePath( const std::string& index, const std::string& landmarksDir, const std::string& dataDir )
      {
         _landmarks.clear();
         _dataPath.clear();
         std::ifstream f( index.c_str() );
         if ( !f.good() )
         {
            throw std::runtime_error( "cannot open index=" + index );
         }

         while ( !f.eof() )
         {
            std::string id;
            std::getline( f, id );

            Dataset dataset;
            std::string idTfm = _dirToLandmarkId( id );
            dataset.read( landmarksDir + idTfm + ".txt" );
            
            _landmarks.push_back( dataset );
            _dataPath.push_back( dataDir + id );
         }
      }

   private:
      std::vector<std::string>   _dataPath;
      std::vector<Dataset>       _landmarks;
   };
}
}

#endif