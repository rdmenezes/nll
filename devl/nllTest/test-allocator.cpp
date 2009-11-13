#include "stdafx.h"
#include <nll/nll.h>

class TestAllocator
{
public:
   void testAllocatorAligned16Byte()
   {
      typedef nll::core::Allocator16ByteAligned<float>   Allocator;
      typedef nll::core::Allocator16ByteAligned<char>   Allocator2;
      typedef nll::core::Buffer1D<float, nll::core::IndexMapperFlat1D, Allocator>   AlignedBuffer;
      typedef nll::core::Buffer1D<char, nll::core::IndexMapperFlat1D, Allocator2>   AlignedBuffer2;

      for ( int n = 0; n < 100; ++n )
      {
         AlignedBuffer test( 3 );
         AlignedBuffer2 test2( 4 );
         AlignedBuffer2 test3( 5 );
         AlignedBuffer2 test4( 6 );
         AlignedBuffer2 test5( 2 );
         AlignedBuffer2 test6( 1 );
         ++test[ 0 ];
         ++test2[ 0 ];
         ++test3[ 0 ];
         ++test4[ 0 ];
         ++test5[ 0 ];
         ++test6[ 0 ];
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestAllocator);
TESTER_TEST(testAllocatorAligned16Byte);
TESTER_TEST_SUITE_END();
#endif