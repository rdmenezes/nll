#ifndef MVV_FACADE_SLICE_MAPPER_H_
# define MVV_FACADE_SLICE_MAPPER_H_

# include "def.h"
# include "sliceMapper.h"

# pragma warning( push )
# pragma warning( disable:4251 ) // dll interface for STL

namespace mvv
{
namespace mapper
{
   struct BODYMAPPER_API FacadeSliceMapperParameters
   {
      SliceMapperPreprocessingParameters                 slicePreprocessingParams;
      SliceMapperPreprocessingClassifierParametersInput  classifierPreprocessingParams;
   };

   /**
    @brief Present in a coherent way the different actions possible with the slice mapper subsystem
    */
   class BODYMAPPER_API FacadeSliceMapper
   {
      typedef SliceBasicPreprocessing::Database Database;

   public:
      FacadeSliceMapper( const FacadeSliceMapperParameters params = FacadeSliceMapperParameters() );

      /**
       @brief This will create the preprocessed slice database

         This will preprocess the slices. This preprocessing is as much as possible independent of the underlying
         classifiers, so that this database is regenerated very often, compared to the classifier's feature database.

         The function will export the preprocessed database and is a prerequisite for more advanced functions.
       */
      void createPreprocessedDatabase( const LandmarkDataset& datasets, const std::string& preprocessedSliceDatabaseOutput ) const;

      /**
       @brief This will create the database 
       */
      //void createClassifierFeatures( const std::string& preprocessedSliceDatabase, const std::string&  ) const;

   private:
      FacadeSliceMapperParameters         _params;
      SliceBasicPreprocessing             _slicePreprocessing;
      SlicePreprocessingClassifierInput   _classifierPreprocessing;
   };
}
}

# pragma warning( pop )

#endif