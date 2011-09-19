#include <tester/register.h>
#include <bodyMapper/datasetSliceMapper.h>
#include <bodyMapper/facadeSliceMapper.h>
#include <bodyMapper/sliceMapperVisualization.h>

#define SLICE_MAPPER_PREPROCESSED_DATABASE               "../../bodyMapper/data/sliceMapperPreprocessedDatabase.dat"
#define SLICE_MAPPER_CLASSIFIER_FEATURES_DATABASE        "../../bodyMapper/data/sliceClassifierFeatureDatabase.dat"
#define SLICE_MAPPER_CLASSIFIER_FEATURES_PREPROCESSING   "../../bodyMapper/data/sliceClassifierFeaturePreprocessing.dat"
#define SLICE_MAPPER_CLASSIFIERS                         "../../bodyMapper/data/sliceClassifiers.dat"

using namespace mvv::mapper;
/**
 * Test harness for bodyMapperTest.
 *
 * @version 1.0
 * @author 
 */
class TestBodyMapper
{
public:
   void createSliceMapperPreview()
   {
      LandmarkDataset dataset;
      SliceMapperVisualization vizualization;
      vizualization.createPreview( dataset );
   }

   /**
    @brief Simply a visualization of the preprocessed slices
    */
   void exportPreprocessedSliceDebug()
   {
      LandmarkDataset dataset;
      for ( unsigned v = 0; v <24; ++v )
      {
         std::auto_ptr<LandmarkDataset::Volume> data = dataset.loadData( v );

         SliceBasicPreprocessing sliceMapper;
         for ( unsigned n = 0; n < data->getSize()[ 2 ]; ++n )
         {
            std::cout << "SLICE=" << n << std::endl;
            SliceBasicPreprocessing::Image i = sliceMapper.preprocessSlice( *data, n );
            nll::core::extend( i, 3 );
            nll::core::writeBmp( i, "c:/tmp/vol" + nll::core::val2str( v ) + "-" + nll::core::val2str( n ) + ".bmp" );
         }
      }
   }

   // here we are creating the preprocessed slices, basically a simple preprocessing to clean the image to remove
   // the variance in the data. This preprocessing is not related to classifier preprocessing, so that we can regenerate
   // this database much less often
   void createPreprocessedDatabase()
   {
      std::cout << "createPreprocessedDatabase()" << std::endl;
      LandmarkDataset datasets;
      SliceMapperDataset sliceMapperDatasets( datasets );

      FacadeSliceMapper sliceMapper;
      sliceMapper.createPreprocessedDatabase( sliceMapperDatasets, SLICE_MAPPER_PREPROCESSED_DATABASE );
   }

   // now create feature processing, features that will be fed to the classifier
   void createClassifierFeaturesPreprocessing()
   {
      std::cout << "createClassifierFeaturesPreprocessing()" << std::endl;

      FacadeSliceMapper sliceMapper;
      sliceMapper.createClassifierFeatures( SLICE_MAPPER_PREPROCESSED_DATABASE, SLICE_MAPPER_CLASSIFIER_FEATURES_PREPROCESSING );
   }

   void createClassifierFeaturesDatabase()
   {
      std::cout << "createClassifierFeaturesDatabase()" << std::endl;

      FacadeSliceMapper sliceMapper;
      sliceMapper.createClassifierFeaturesDatabase( SLICE_MAPPER_PREPROCESSED_DATABASE, SLICE_MAPPER_CLASSIFIER_FEATURES_PREPROCESSING, SLICE_MAPPER_CLASSIFIER_FEATURES_DATABASE );
   }

   void createSliceClassifiers()
   {
      std::cout << "createSliceClassifiers()" << std::endl;
      FacadeSliceMapper sliceMapper;
      sliceMapper.createClassifiersSvm( SLICE_MAPPER_CLASSIFIER_FEATURES_DATABASE, SLICE_MAPPER_CLASSIFIERS );
   }
};

TESTER_TEST_SUITE(TestBodyMapper);

//TESTER_TEST(createSliceMapperPreview);
TESTER_TEST(createPreprocessedDatabase);
TESTER_TEST(createClassifierFeaturesPreprocessing);
TESTER_TEST(createClassifierFeaturesDatabase);
TESTER_TEST(createSliceClassifiers);
TESTER_TEST_SUITE_END();
