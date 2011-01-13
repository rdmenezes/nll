#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;

namespace
{
   double test_data[][ 2 ] =
   {
      { 0.0699,    0.0389 },
      {-0.0007,   -0.0108 },
      {-0.2154,    0.0546 },
      { 0.2194,    0.0261 },
      {-0.1648,    0.0233 },
      { 0.1852,   -0.0516 }
   };
}

// example validated against matlab prototype
class TestKernelPca
{
public:
   /**
    */
   void simpleTest()
   {
      typedef std::vector<double>               Point;
      typedef std::vector<Point>                Points;
      typedef nll::algorithm::KernelRbf<Point>   Kernel;
      typedef nll::algorithm::KernelPca<Point, Kernel>  KernelPca;

      const unsigned size = nll::core::getStaticBufferSize( test_data );
      Points points;
      for ( unsigned n = 0; n < size; ++n )
      {
         points.push_back( nll::core::make_vector<double>( test_data[ n ][ 0 ], test_data[ n ][ 1 ] ) );
      }

      KernelPca kpca;
      Kernel rbfKernel( 0.02 );
      kpca.compute( points, 5, rbfKernel );

      KernelPca::Vector v = kpca.transform( points[ 0 ] );

      TESTER_ASSERT( v.size() == 5 );
      TESTER_ASSERT( fabs( v[ 0 ] - -0.893936 ) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 1 ] - 1.268     ) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 2 ] - -0.650698 ) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 3 ] - 0.661651  ) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 4 ] - 0.210299  ) < 1e-4 );
      v.print( std::cout );

      std::stringstream ss;
      kpca.write( ss );

      KernelPca kpca2;
      kpca2.read( ss );

      KernelPca::Vector v2 = kpca2.transform( points[ 0 ] );
      TESTER_ASSERT( v2.size() == 5 );
      TESTER_ASSERT( fabs( v2[ 0 ] - -0.893936 ) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 1 ] - 1.268     ) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 2 ] - -0.650698 ) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 3 ] - 0.661651  ) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 4 ] - 0.210299  ) < 1e-4 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKernelPca);
TESTER_TEST(simpleTest);
TESTER_TEST_SUITE_END();
#endif
