#include <nll/nll.h>
#include <tester/register.h>
#include <vector>
#include <sstream>
#include "config.h"

namespace nll
{
namespace imaging
{
   

   
}
}

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;
using namespace nll::imaging;

class TestVolumeCompressor
{
public:
   void testBasic()
   {
      srand( 0 );
      const double minValueSlice0 = -10;
      const double maxValueSlice0 = 20;
      VolumeMemoryBuffer<double> input( 3, 6, 70 );
      input( 0, 0, 0 ) = 3;
      input( 1, 1, 0 ) = minValueSlice0;
      input( 2, 0, 0 ) = maxValueSlice0;

      for ( size_t z = 1; z < input.getSize()[ 2 ] - 1; ++z ) // we want specif cases for empty values & maximal range test
      {
         for ( size_t y = 0; y < input.getSize()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < input.getSize()[ 0 ]; ++x )
            {
               const double val = core::generateUniformDistribution( -10000, 50000 );
               input( x, y, z ) = val;
            }
         }
      }

      VolumeMemoryBuffer<ui8> output;
      std::vector<Rsi> rsi;
      VolumeDiscretizer discretizer;
      discretizer.discretize( input, output, rsi );

      for ( size_t z = 0; z < input.getSize()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < input.getSize()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < input.getSize()[ 0 ]; ++x )
            {
               const double expectedVal = input( x, y, z );
               const double val = output( x, y, z );
               const double computedVal = val * rsi[ z ].slope + rsi[ z ].intercept;
               TESTER_ASSERT( fabs( expectedVal - computedVal ) < rsi[ z ].slope );
            }
         }
      }

      TESTER_ASSERT( fabs( rsi[ 0 ].intercept - minValueSlice0 ) < 1e-5 );
      const double expectedSlopeSlice0 = ( maxValueSlice0 - minValueSlice0 ) / 255.0;
      TESTER_ASSERT( fabs( rsi[ 0 ].slope - expectedSlopeSlice0 ) < 1e-5 );
   }

   void testCompression()
   {
      for ( size_t n = 0; n < 500; ++n )
      {
         const size_t size = ( rand() % 500 ) + 50;
         std::vector<size_t> test;
         for ( size_t nn = 0; nn < size; ++nn )
         {
            const size_t v = rand() % 3;
            test.push_back( v );
         }

         DataCompressorCount compressor( 2 );

         DataCompressorCount::Chunks<size_t> chunks;
         compressor.compress( test.begin(), test.end(), chunks );
      
         const size_t nbData = chunks.dataSize();
         TESTER_ASSERT( nbData == size );

         std::vector<size_t> res( size );
         chunks.decode( &res[ 0 ] );

         TESTER_ASSERT( res == test );

         std::stringstream ss;
         chunks.write( ss );

         std::vector<size_t> resRead( size );
         DataCompressorCount::Chunks<size_t> chunksRead;
         chunksRead.read( ss );
         chunks.decode( &resRead[ 0 ] );
         TESTER_ASSERT( chunksRead.dataSize() == size );
         TESTER_ASSERT( resRead == test );
      }
   }

   void testReadWriteMf3()
   {
      const std::string out = "c:/tmp/test.mf3";
      VolumeMf3Format mf3File;

      VolumeSpatial<double> volume;
      VolumeSpatial<double> volume2;

      {
         core::Timer importTime;
         imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/pet-NAC.mf2", volume );
         std::cout << "importTimeToBeat=" << importTime.getCurrentTime() << std::endl;

         std::ofstream f( out.c_str(), std::ios::binary );
         core::Timer exportTime;
         mf3File.write( volume, f );
         std::cout << "exportTime=" << exportTime.getCurrentTime() << std::endl;
      }

      {
         core::Timer importTime;
         std::ifstream f2( out.c_str(), std::ios::binary );
         mf3File.read( volume2, f2 );
         std::cout << "importTimeNewVersion=" << importTime.getCurrentTime() << std::endl;
      }

      TESTER_ASSERT( volume.getSize() == volume2.getSize() );
      for ( size_t z = 0; z < volume.getSize()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < volume.getSize()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < volume.getSize()[ 0 ]; ++x )
            {
               const double expectedVal = volume( x, y, z );
               const double expectedVal2 = volume2( x, y, z );
               TESTER_ASSERT( fabs( expectedVal - expectedVal2 ) < 1.0 );
            }
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeCompressor);
TESTER_TEST(testBasic);
TESTER_TEST(testCompression);
TESTER_TEST(testReadWriteMf3);
TESTER_TEST_SUITE_END();
#endif