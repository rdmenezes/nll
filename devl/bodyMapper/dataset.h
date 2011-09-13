#ifndef MVV_MAPPER_DATASET_H_
# define MVV_MAPPER_DATASET_H_

# include "def.h"
# include <nll/nll.h>

# pragma warning( push )
# pragma warning( disable:4251 ) // dll interface for STL

# define LANDMARK_DEFAULT_INDEX      "../../bodyMapper/index/index.txt"
# define LANDMARK_DEFAULT_DIR        "../../bodyMapper/landmarks/"
# define LANDMARK_DATA_DEFAULT_DIR   "C:/DicomDataRepositery/"

# define LANDMARK_DEFAULT_INDEX_LEARNING    "../../bodyMapper/index/learning.txt"
# define LANDMARK_DEFAULT_INDEX_TESTING     "../../bodyMapper/index/testing.txt"
# define LANDMARK_DEFAULT_INDEX_VALIDATING  "../../bodyMapper/index/validating.txt"

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
         std::string fileId;

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

            std::getline( f, line );
            splits = nll::core::split( line, '=' );
            ensure( splits.size() == 2, "expecting caseId=XXX" );
            fileId = splits[ 1 ];
         }
      };

      /**
       @param index the index of the cases to handle
       @param landmarksDirectory the directory where the landmarks for each case are stored
       @param dataDirectory where the data are stored
       @param learningIndex contains the ID of the cases read in <index> that will be used to train the models
       @param testingIndex contains the ID of the cases read in <index> that will be used to test the models
       @param validateIndex contains the ID of the cases read in <index> that will be used to by algorithms requiring validation of the models
       */
      LandmarkDataset( const std::string index                = LANDMARK_DEFAULT_INDEX,
                       const std::string landmarksDirectory   = LANDMARK_DEFAULT_DIR,
                       const std::string dataDirectory        = LANDMARK_DATA_DEFAULT_DIR,
                       const std::string learningIndex        = LANDMARK_DEFAULT_INDEX_LEARNING,
                       const std::string testingIndex         = LANDMARK_DEFAULT_INDEX_TESTING,
                       const std::string validatingIndex      = LANDMARK_DEFAULT_INDEX_VALIDATING )
      {
         _populatePath( index, landmarksDirectory, dataDirectory );

         _learning   = _readIndex( learningIndex );
         _testing    = _readIndex( testingIndex );
         _validating = _readIndex( validatingIndex );

         _validateDataPartitions();
      }

      unsigned size() const
      {
         return (unsigned) _dataPath.size();
      }

      const Dataset& operator[]( unsigned caseid ) const
      {
         return _landmarks[ caseid ];
      }

      std::auto_ptr<Volume> loadData( unsigned id ) const;

      const std::vector<unsigned>& getTesting() const
      {
         return _testing;
      }

      const std::vector<unsigned>& getLearning() const
      {
         return _learning;
      }

      const std::vector<unsigned>& getValidating() const
      {
         return _validating;
      }

      // return the type {LEARNING, TESTING, VALIDATING} of a case
      int getSampleType( unsigned caseid ) const
      {
         std::vector<unsigned>::const_iterator itTesting = std::find( getTesting().begin(),
                                                                      getTesting().end(),
                                                                      caseid );
         if ( itTesting != getTesting().end() )
         {
            return (int)nll::core::ClassificationSample<int,int>::TESTING;
         }

         std::vector<unsigned>::const_iterator itLearning = std::find( getLearning().begin(),
                                                                       getLearning().end(),
                                                                       caseid );
         if ( itLearning != getLearning().end() )
         {
            return (int)nll::core::ClassificationSample<int,int>::LEARNING;
         }
         std::vector<unsigned>::const_iterator itValidating = std::find( getValidating().begin(),
                                                                         getValidating().end(),
                                                                         caseid );
         if ( itValidating != getValidating().end() )
         {
            return (int)nll::core::ClassificationSample<int,int>::VALIDATION;
         }
         throw std::runtime_error( "can't reach: data doesn't have role, but this was validated!" );
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

         size_t nbLandmarks = 0;
         while ( !f.eof() )
         {
            std::string id;
            std::getline( f, id );

            Dataset dataset;
            std::string idTfm = _dirToLandmarkId( id );
            dataset.read( landmarksDir + idTfm + ".txt" );
            
            _landmarks.push_back( dataset );
            _dataPath.push_back( dataDir + id );
            if ( nbLandmarks == 0 )
            {
               nbLandmarks = dataset.landmarks.size();
            }
         }

         for ( size_t n = 0; n < _landmarks.size(); ++n )
         {
            ensure( _landmarks[ n ].landmarks.size() == nbLandmarks, "the number of landmarks between landmark files are not identical!" );
         }
      }

      std::vector<unsigned> _readIndex( const std::string& file )
      {
         std::vector<unsigned> ids;

         std::ifstream f( file.c_str() );
         if ( !f.good() )
         {
            throw std::runtime_error( "cannot open file=" + file );
         }

         while ( !f.eof() )
         {
            std::string line;
            std::getline( f, line );
            if ( line != "" )
            {
               ids.push_back( nll::core::str2val<unsigned>( line ) );
            };
         }

         return ids;
      }

      void _validateDataPartitions()
      {
         std::set<unsigned> learning;
         std::set<unsigned> testing;
         std::set<unsigned> validating;

         for ( unsigned n = 0; n < _learning.size(); ++n )
         {
            unsigned id = _learning[ n ];
            learning.insert( id );
         }

         for ( unsigned n = 0; n < _testing.size(); ++n )
         {
            unsigned id = _testing[ n ];
            testing.insert( id );
            std::set<unsigned>::const_iterator it1 = learning.find( id );
            if ( it1 != learning.end() )
            {
               throw std::runtime_error( "learning and testing datasets have common sample:" + nll::core::val2str( id ) );
            }
         }

         for ( unsigned n = 0; n < _validating.size(); ++n )
         {
            unsigned id = _validating[ n ];
            validating.insert( id );
            std::set<unsigned>::const_iterator it1 = learning.find( id );
            std::set<unsigned>::const_iterator it2 = testing.find( id );
            if ( it1 != learning.end() )
            {
               throw std::runtime_error( "learning and validating datasets have common sample:" + nll::core::val2str( id ) );
            }

            if ( it2 != testing.end() )
            {
               throw std::runtime_error( "testing and validating datasets have common sample:" + nll::core::val2str( id ) );
            }
         }

         if ( _learning.size() + _testing.size() + validating.size() != _dataPath.size() )
         {
            throw std::runtime_error( "data validation: a data has a role missing (LEARNING|TESTING|VALIDATING)" );
         }
      }

   private:
      std::vector<std::string>   _dataPath;
      std::vector<Dataset>       _landmarks;
      std::vector<unsigned>      _learning;
      std::vector<unsigned>      _testing;
      std::vector<unsigned>      _validating;
   };
}
}

#endif

# pragma warning( pop )