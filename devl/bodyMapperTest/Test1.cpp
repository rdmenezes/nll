#include <tester/register.h>
#include <bodyMapper/sliceMapper.h>

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
      for ( unsigned v = 0; v < 24; ++v )
      {
         std::auto_ptr<LandmarkDataset::Volume> data = dataset.loadData( v );

         SliceMapper sliceMapper;
         for ( unsigned n = 0; n < data->getSize()[ 2 ]; ++n )
         {
            std::cout << "SLICE=" << n << std::endl;
            SliceMapper::Image i = sliceMapper.preprocessSlice( *data, n );
            nll::core::extend( i, 3 );
            nll::core::writeBmp( i, "c:/tmp/vol" + nll::core::val2str( v ) + "-" + nll::core::val2str( n ) + ".bmp" );
         }
      }
   }

};

TESTER_TEST_SUITE(TestBodyMapper);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
