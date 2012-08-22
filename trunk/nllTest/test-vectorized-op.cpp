#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

#define NLL_INSTRUCTION_SET_BASIC      0
#define NLL_INSTRUCTION_SET_SSE        1
#define NLL_INSTRUCTION_SET_SSE2       2
#define NLL_INSTRUCTION_SET_SSE3       3
#define NLL_INSTRUCTION_SET_SSSE3      4
#define NLL_INSTRUCTION_SET_SSE41      5
#define NLL_INSTRUCTION_SET_SSE42      6
#define NLL_INSTRUCTION_SET_AVX        7
#define NLL_INSTRUCTION_SET_AVX2       8

/**
 @brief Defines the current processor instruction set supported
 */
# define NLL_INSTRUCTION_SET NLL_INSTRUCTION_SET_SSE42

namespace nll
{
namespace vectorized
{
   float accumulate_vectorized4( const float* start, const float* end )
   {
      assert( start < end );
      const size_t nbVectorizedLoops = ( end - start ) / 8;
      const float* endVectorized = start + nbVectorizedLoops * 8;

      float accum = 0;
      for( ; start != endVectorized; start += 8 )
      {
         // loop unrolling to start simultaneous computations
         Vec4f r1;
         r1.load( start );
         const float accum1 = horizontal_add( r1 );

         Vec4f r2;
         r2.load( start + 4 );
         const float accum2 = horizontal_add( r2 );

         accum += accum1 + accum2;
      }

      for( ; start != end; ++start )
      {
         accum += *start;
      }

      return accum;
   }

   void add_vectorized4( const float* start, const float* end, float* out )
   {
      assert( start < end );
      const size_t nbVectorizedLoops = ( end - start ) / 4;
      const float* endVectorized = start + nbVectorizedLoops * 4;

      for( ; start != endVectorized; start += 4, out += 4 )
      {
         Vec4f r1;
         r1.load( start );

         Vec4f r2;
         r2.load( out );

         r2 += r1;
         r2.store( out );
      }

      for( ; start != end; ++start, ++out )
      {
         *out += *start;
      }
   }

   void addmul_vectorized4( float val1, const float* start, const float* end, float val2, float* out )
   {
      assert( start < end );
      const size_t nbVectorizedLoops = ( end - start ) / 4;
      const float* endVectorized = start + nbVectorizedLoops * 4;

      for( ; start != endVectorized; start += 4, out += 4 )
      {
         Vec4f r1;
         r1.load( start );

         Vec4f r2;
         r2.load( out );

         r2 = r2 * val2 + r1 * val1;
         r2.store( out );
      }

      for( ; start != end; ++start, ++out )
      {
         *out = *start * val1 + *out * val2;
      }
   }
}
}

void addDummy( const float* start, const float* end, float* out )
{
   for( ; start != end; ++start, ++out )
   {
      *out += *start;
   }
}

void addMulDummy( float val1, const float* start, const float* end, float val2, float* out )
{
   for( ; start != end; ++start, ++out )
   {
      *out = *start * val1 + *out * val2;
   }
}

class TestVectorizedOp
{
   typedef core::Allocator16ByteAligned<float>  AllocatorAligned;
   typedef core::Image<float, core::IndexMapperRowMajorFlat2DColorRGBn> Image;

public:
   void test_vec4f_add()
   {
      for ( size_t n = 0; n < 1000; ++n )
      {
         float vals[ 4 ] =
         {
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 )
         };

         float vals2[ 4 ] =
         {
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 )
         };

         vectorized::Vec4f v;
         v.load( vals );

         vectorized::Vec4f v2;
         v2.load( vals2 );

         v2 += v;

         for ( size_t n = 0; n < 4; ++n )
         {
            TESTER_ASSERT( core::equal<float>( v2.extract( n ), vals[ n ] + vals2[ n ], 5e-6f ) );
         }

         v.load( vals );
         v2.load( vals2 );

         vectorized::Vec4f v3 = v2 + v;

         for ( size_t n = 0; n < 4; ++n )
         {
            TESTER_ASSERT( core::equal<float>( v3.extract( n ), vals[ n ] + vals2[ n ], 5e-6f ) );
         }
      }
   }

   void test_vec4f_truncatef()
   {
      {
         vectorized::Vec4f v( 0.1, 0.4, 0.6, 0.9 );
         vectorized::Vec4i vi = vectorized::truncatei( v );

         TESTER_ASSERT( vi.extract( 0 ) == 0 );
         TESTER_ASSERT( vi.extract( 1 ) == 0 );
         TESTER_ASSERT( vi.extract( 2 ) == 0 );
         TESTER_ASSERT( vi.extract( 3 ) == 0 );
      }

      {
         // truncate toward 0
         vectorized::Vec4f v( -0.1, -0.4, -0.6, -0.9 );
         vectorized::Vec4i vi = vectorized::truncatei( v );

         TESTER_ASSERT( vi.extract( 0 ) == 0 );
         TESTER_ASSERT( vi.extract( 1 ) == 0 );
         TESTER_ASSERT( vi.extract( 2 ) == 0 );
         TESTER_ASSERT( vi.extract( 3 ) == 0 );
      }

      {
         // truncate toward 0
         vectorized::Vec4f v( 1.1, 1.4, 1.6, 1.9 );
         vectorized::Vec4i vi = vectorized::truncatei( v );

         TESTER_ASSERT( vi.extract( 0 ) == 1 );
         TESTER_ASSERT( vi.extract( 1 ) == 1 );
         TESTER_ASSERT( vi.extract( 2 ) == 1 );
         TESTER_ASSERT( vi.extract( 3 ) == 1 );
      }
   }

   void test_vec4f_Horizontal()
   {
      for ( size_t n = 0; n < 1000; ++n )
      {
         float vals[ 4 ] =
         {
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 ),
            core::generateUniformDistributionf( -10, 10 )
         };

         vectorized::Vec4f v;
         v.load( vals );

         const float result = vectorized::horizontal_add( v );
         const float expected = vals[ 0 ] + vals[ 1 ] + vals[ 2 ] + vals[ 3 ];

         TESTER_ASSERT( core::equal<float>( result, expected, 5e-6f ) );
      }
   }

   void testAccumulateFloat()
   {
      for ( size_t n = 0; n < 20; ++n )
      {
         const size_t size = core::generateUniformDistributioni( 300, 600 );
         Image image1( size, size, 1 );

         for ( Image::iterator it = image1.begin(); it != image1.end(); ++it )
         {
            *it = core::generateUniformDistributionf( -10.0f , 10.0f );
         }

         core::Timer timer;
         float accum1 = 0;
         for ( size_t n = 0; n < 1000; ++n )
         {
            accum1 += vectorized::accumulate_vectorized4( image1.begin(), image1.end() );
         }
         const float time1 = timer.getCurrentTime();

         core::Timer timer2;
         float accum2 = 0;
         for ( size_t n = 0; n < 1000; ++n )
         {
            accum2 += std::accumulate( image1.begin(), image1.end(), 0.0f );
         }
         const float time2 = timer2.getCurrentTime();

         // we test we have the same results, we accept a relative error due to consequtive rounding errors
         const float tol = fabs( accum1 * 1e-4f );
         std::cout << "time (vectorized)=" << time1 << " time STL=" << time2 << " tolerance=" << tol << std::endl;
         TESTER_ASSERT( core::equal<float>( accum1, accum2, tol ) );
         TESTER_ASSERT( 2 * time1 < time2 );
      }
   }

   static void generate( core::Buffer1D<float>& vals )
   {
      for ( Image::iterator it = vals.begin(); it != vals.end(); ++it )
      {
         *it = core::generateUniformDistributionf( -10.0f , 10.0f );
      }
   }

   void testAddFloat()
   {
      for ( size_t n = 0; n < 20; ++n )
      {
         const size_t nbLoops = 1000;
         const size_t size = core::generateUniformDistributioni( 600, 1000 );

         Image image1( size, size, 1 );
         Image image2( size, size, 1 );

         srand( 0 );
         generate( image1 );
         generate( image2 );

         core::Timer timer1;
         for ( size_t n = 0; n < nbLoops; ++n )
         {
            vectorized::add_vectorized4( image1.begin(), image1.end(), image2.begin() );
         }

         const float time1 = timer1.getCurrentTime();
         Image found;
         found.clone( image2 );

         // reinit
         srand( 0 );
         generate( image1 );
         generate( image2 );
     
         core::Timer timer2;
         for ( size_t n = 0; n < nbLoops; ++n )
         {
            addDummy( image1.begin(), image1.end(), image2.begin() );
         }
         const float time2 = timer2.getCurrentTime();

         std::cout << "time (vectorized)=" << time1 << " time STL=" << time2 << std::endl;

         for ( size_t n = 0; n < found.size(); ++n )
         {
            TESTER_ASSERT( core::equal<float>( found[ n ], image2[ n ], 0.1 ) );
         }
         TESTER_ASSERT( 1.5 * time1 < time2 );
      }
   }

   void testAddMulFloat()
   {
      for ( size_t n = 0; n < 20; ++n )
      {
         const size_t nbLoops = 1000;
         const size_t size = core::generateUniformDistributioni( 600, 1000 );

         Image image1( size, size, 1 );
         Image image2( size, size, 1 );

         srand( 0 );
         generate( image1 );
         generate( image2 );

         core::Timer timer1;
         for ( size_t n = 0; n < nbLoops; ++n )
         {
            vectorized::addmul_vectorized4( 0.3, image1.begin(), image1.end(), 1.5, image2.begin() );
         }

         const float time1 = timer1.getCurrentTime();
         Image found;
         found.clone( image2 );

         // reinit
         srand( 0 );
         generate( image1 );
         generate( image2 );
     
         core::Timer timer2;
         for ( size_t n = 0; n < nbLoops; ++n )
         {
            addMulDummy( 0.3, image1.begin(), image1.end(), 1.5, image2.begin() );
         }
         const float time2 = timer2.getCurrentTime();

         std::cout << "time (vectorized)=" << time1 << " time STL=" << time2 << std::endl;

         for ( size_t n = 0; n < found.size(); ++n )
         {
            TESTER_ASSERT( core::equal<float>( found[ n ], image2[ n ], 0.1 ) );
         }
         TESTER_ASSERT( 1.5 * time1 < time2 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVectorizedOp);
 TESTER_TEST(test_vec4f_Horizontal);
 TESTER_TEST(testAccumulateFloat);
 TESTER_TEST(test_vec4f_add);
 TESTER_TEST(testAddFloat);
 TESTER_TEST(testAddMulFloat);
 TESTER_TEST(test_vec4f_truncatef);
TESTER_TEST_SUITE_END();
#endif