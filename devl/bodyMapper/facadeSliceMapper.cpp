#include "facadeSliceMapper.h"

namespace mvv
{
namespace mapper
{
   FacadeSliceMapper::FacadeSliceMapper( const FacadeSliceMapperParameters params )
      : _params( params ),
        _slicePreprocessing( params.slicePreprocessingParams ),
        _classifierPreprocessing( params.classifierPreprocessingParams,
                                  params.slicePreprocessingParams )
   {
   }

   void FacadeSliceMapper::createPreprocessedDatabase( const LandmarkDataset& datasets, const std::string& preprocessedSliceDatabaseOutput ) const
   {
      Database preprocessedDat = _slicePreprocessing.createPreprocessedDatabase( datasets );
      preprocessedDat.write( preprocessedSliceDatabaseOutput );
   }

   void FacadeSliceMapper::createClassifierFeatures( const std::string& preprocessedSliceDatabase, const std::string& classifierFeaturesOutput ) const
   {
      Database preprocessedDat;
      preprocessedDat.read( preprocessedSliceDatabase );

      SlicePreprocessingClassifierInput classifierPreprocessing = _classifierPreprocessing;
      classifierPreprocessing.computeClassifierFeatures( preprocessedDat );
      classifierPreprocessing.write( classifierFeaturesOutput );
   }

   void FacadeSliceMapper::createClassifierFeaturesDatabase( const std::string& preprocessedSliceDatabase, const std::string& classifierFeatures, const std::string& classifierFeaturesDatabaseOutput ) const
   {
      Database preprocessedDat;
      preprocessedDat.read( preprocessedSliceDatabase );

      SlicePreprocessingClassifierInput classifierPreprocessing = _classifierPreprocessing;
      classifierPreprocessing.read( classifierFeatures );
      std::vector<Database> dats = classifierPreprocessing.createClassifierInputDatabases( preprocessedDat );

      std::ofstream f( classifierFeaturesDatabaseOutput.c_str(), std::ios::binary );
      if ( !f.good() )
      {
         throw std::runtime_error( "cannot open file:" + classifierFeaturesDatabaseOutput );
      }
      nll::core::write< std::vector<Database> >( dats, f );
   }

   void FacadeSliceMapper::createClassifiersSvm( const std::string& classifierFeaturesDatabase, const std::string& sliceClassifiersOutput )
   {
      std::vector<Database> databases;

      std::ifstream f( classifierFeaturesDatabase.c_str(), std::ios::binary );
      if ( !f.good() )
      {
         throw std::runtime_error( "cannot open file:" + classifierFeaturesDatabase );
      }
      nll::core::read( databases, f );
      f.close();

      _classifiers.learn( _params.sliceMapperClassifierSvmParams, databases );
      _classifiers.write( sliceClassifiersOutput );
   }
}
}