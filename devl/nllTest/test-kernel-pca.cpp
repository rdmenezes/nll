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

class TestKernelPca
{
public:
   /**
    */
   void simpleTest()
   {
      typedef std::vector<double>               Point;
      typedef std::vector<Point>                Points;
      //typedef nll::algorithm::KernelPolynomial<Point>   Kernel;
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
      //Kernel rbfKernel( 1 );
      kpca.compute( points, 6, rbfKernel );

      KernelPca::Vector v = kpca.transform( points[ 0 ] );
      v.print( std::cout );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKernelPca);
TESTER_TEST(simpleTest);
TESTER_TEST_SUITE_END();
#endif
