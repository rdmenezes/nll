#include "facadeSliceMapper.h"

namespace mvv
{
namespace mapper
{
   FacadeSliceMapper::FacadeSliceMapper( const FacadeSliceMapperParameters params ) : _params( params ),
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
}
}