#include <tester/register.h>
#include <bodyMapper/sliceMapper.h>

#define SLICE_MAPPER_PREPROCESSED_DATABASE      "../../bodyMapper/data/sliceMapperPreprocessedDatabase.dat"
#define SLICE_MAPPER_CLASSIFIER_INPUT_DATABASE  "../../bodyMapper/data/sliceMapperClassifierInputDatabase.dat"

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
   void test1()
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
      LandmarkDataset datasets;
      SliceBasicPreprocessing sliceMapper;

      SliceBasicPreprocessing::Database preprocessedDatabase = sliceMapper.createPreprocessedDatabase( datasets );
      preprocessedDatabase.write( SLICE_MAPPER_PREPROCESSED_DATABASE );

      std::ofstream desc( SLICE_MAPPER_PREPROCESSED_DATABASE ".desc" );
      sliceMapper.getPreprocessingParameters().print( desc );
   }

   // now create the database feeding the classifier
   void createClassifierInputDatabase()
   {
      SliceBasicPreprocessing::Database preprocessedDatabase;
      preprocessedDatabase.read( SLICE_MAPPER_PREPROCESSED_DATABASE );
      
      SliceMapperPreprocessingParameters                 paramBasicPreprocessing;
      SliceMapperPreprocessingClassifierParametersInput  paramClassifierPreprocessing;

      SlicePreprocessingClassifierInput classifierInputPreprocessor( paramClassifierPreprocessing, paramBasicPreprocessing );
      classifierInputPreprocessor.computeClassifierFeatures( preprocessedDatabase );
      std::vector<SliceBasicPreprocessing::Database> dats = classifierInputPreprocessor.createClassifierInputDatabases( preprocessedDatabase );
   }
};

TESTER_TEST_SUITE(TestBodyMapper);
//TESTER_TEST(createPreprocessedDatabase);
TESTER_TEST(createClassifierInputDatabase);
TESTER_TEST_SUITE_END();
