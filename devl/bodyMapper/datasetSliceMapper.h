#ifndef MVV_MAPPER_DATASET_SLICE_MAPPER_H_
# define MVV_MAPPER_DATASET_SLICE_MAPPER_H_

# include "dataset.h"

# pragma warning( push )
# pragma warning( disable:4251 ) // dll interface for STL

namespace mvv
{
namespace mapper
{
   /**
    @brief Hold the slice of interest that will be used for the mapping of a volume.

    This database is generated from the landmark dataset
    */
   class BODYMAPPER_API SliceMapperDataset
   {
   public:
      typedef nll::imaging::VolumeSpatial<float>   Volume;
      typedef nll::core::vector3f                  vector3f;

   public:
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
            LD_LUNG                      = 2,
            LD_AORTIC_ARCH               = 3,
            LD_HUMERUS                   = 4,
            LD_SBLADE                    = 5,
            LD_HEART_VEIN_CAVE_ENTRANCE  = 6,
            LD_CARINA                    = 7,
            LD_PORTAL_VEIN               = 8,
            LD_SPLEEN_HEAD               = 9,
            LD_KIDNEY                    = 10,
            LD_HIPS                      = 11,
            LD_COCCYX                    = 12
         };

         unsigned size() const
         {
            return (unsigned)landmarks.size();
         }

         const vector3f& operator[]( unsigned id ) const
         {
            return landmarks[ id ];
         }

         // check the position of a landmark is > 0, meaning the landmark was set
         bool isValid( unsigned landmarkId ) const
         {
            return landmarkId < landmarks.size() && landmarks[ landmarkId ][ 0 ] >= 0 &&
                                                    landmarks[ landmarkId ][ 1 ] >= 0 &&
                                                    landmarks[ landmarkId ][ 2 ] >= 0;
         }
      };

      SliceMapperDataset( const LandmarkDataset& dat, const std::string sliceMapperMergingConfiguration = "../../bodyMapper/index/sliceMapperMergingConfiguration.txt" ) : _landmarkDataset( dat )
      {
         _generateDataset( dat, sliceMapperMergingConfiguration );
      }

      unsigned size() const
      {
         return _landmarkDataset.size();
      }

      const Dataset& operator[]( unsigned caseid ) const
      {
         return _landmarks[ caseid ];
      }

      std::auto_ptr<Volume> loadData( unsigned id ) const
      {
         return _landmarkDataset.loadData( id );
      }

      const std::vector<unsigned>& getTesting() const
      {
         return _landmarkDataset.getTesting();
      }

      const std::vector<unsigned>& getLearning() const
      {
         return _landmarkDataset.getLearning();
      }

      const std::vector<unsigned>& getValidating() const
      {
         return _landmarkDataset.getValidating();
      }

      // return the type {LEARNING, TESTING, VALIDATING} of a case
      int getSampleType( unsigned caseid ) const
      {
         return _landmarkDataset.getSampleType( caseid );
      }

   private:
      void _generateDataset( const LandmarkDataset& dat, const std::string& sliceMapperMergingConfiguration )
      {
         if ( dat.size() == 0 )
            return;

         const unsigned nbLandmarks = (unsigned)dat[ 0 ].landmarks.size();

         // read the merger config
         std::ifstream f( sliceMapperMergingConfiguration.c_str() );
         if ( !f.good() )
            throw std::runtime_error( "cannot load merger configuration:" + sliceMapperMergingConfiguration );
         std::vector< std::vector< unsigned > > merging;
         while ( !f.eof() )
         {
            std::string line;
            std::getline( f, line );
            if ( line != "" )
            {
               std::vector<const char*> indexesStr = nll::core::split( line, ' ' );
               std::vector<unsigned> indexes;
               for ( size_t n = 0; n < indexesStr.size(); ++n )
               {
                  const unsigned index = nll::core::str2val<unsigned>( indexesStr[ n ] );
                  ensure( index < nbLandmarks, "slice merger config incorrect: index greater than total number of landmarks" );
                  indexes.push_back( index );
               }
               merging.push_back( indexes );
            }
         }

         // now merge the landmarks if needed
         typedef LandmarkDataset::Dataset DatasetL;
         for ( unsigned n = 0; n < dat.size(); ++n )
         {
            const DatasetL& src = dat[ n ];
            Dataset d;
            d.landmarks = std::vector<vector3f>( merging.size() );
            d.fileId = src.fileId;

            // now combine the 'left' and 'right' landmarks
            for ( size_t mergeId = 0; mergeId < merging.size(); ++mergeId )
            {
               float accum = 0;
               unsigned nb = 0;
               for ( size_t mergeElement = 0; mergeElement < merging[ mergeId ].size(); ++mergeElement )
               {
                  const unsigned element = merging[ mergeId ][ mergeElement ];
                  if ( _landmarkDataset[ n ].isValid( element ) )
                  {
                     ++nb;
                     accum += src[ element ][ 2 ]; // we are just interested in the z position
                  }
               }

               if ( nb )
               {
                  // reuse one of the position
                  d.landmarks[ mergeId ][ 0 ] = src.landmarks[ merging[ mergeId ][ 0 ] ][ 0 ];
                  d.landmarks[ mergeId ][ 1 ] = src.landmarks[ merging[ mergeId ][ 0 ] ][ 1 ];
                  d.landmarks[ mergeId ][ 2 ] = accum / nb;
               } else {
                  d.landmarks[ mergeId ][ 0 ] = -1;  // undefined slice of interest
                  d.landmarks[ mergeId ][ 1 ] = -1;  // undefined slice of interest
                  d.landmarks[ mergeId ][ 2 ] = -1;  // undefined slice of interest
               }
            }

            _landmarks.push_back( d );
         }
      }

   private:
      LandmarkDataset            _landmarkDataset;
      std::vector<Dataset>       _landmarks;
   };
}
}

# pragma warning( pop )

#endif