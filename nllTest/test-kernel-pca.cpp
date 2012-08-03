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

   double test_data2[][ 2 ] =
   {
      { 0.0699,    1.0389 },
      {-1.0007,   -2.0108 },
      {-1.2154,   -1.0546 },
      { 3.2194,    3.0261 },
      {-2.1648,    -1.0233 },
      { 1.6852,   -0.0516 },
      {-2.9648,    -1.2233 },
      { -0.6852,   0.0516 }
   };

   double test_points[][ 2 ] =
   {
      { -2.2154,    -0.446 },
      { -1.2154,    -1.446 },
      { -0.2154,    -2.446 },
      { 0.56,       0.56   },
      { 1.56,       0.86   }
   };

   double test_points_expected[][ 2 ] =
   {
      { -2.0830,    -2.6640 },
      {-1.1868,     -1.5756},
      {-1.1323,     -2.0077},
      {0.5007,      0.4046},
      {0.9746,      0.0897}
   };
}

// example validated against matlab prototype
// http://www.heikohoffmann.de/documents/hoffmann_kpca_preprint.pdf
// http://www.heikohoffmann.de/publications.html#hoffmann06b
// http://www.heikohoffmann.de/kpca/kpca.zip
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
      TESTER_ASSERT( fabs( v[ 0 ] - -0.36494782009061227 ) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 1 ] - 0.51765993789384512) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 2 ] - -0.26564637758736048) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 3 ] - 0.27011779549760634) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 4 ] - 0.085854182000892176) < 1e-4 );
      v.print( std::cout );

      std::stringstream ss;
      kpca.write( ss );

      KernelPca kpca2;
      kpca2.read( ss );

      KernelPca::Vector v2 = kpca2.transform( points[ 0 ] );
      TESTER_ASSERT( v2.size() == 5 );
      TESTER_ASSERT( fabs( v2[ 0 ] - -0.36494782009061227 ) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 1 ] - 0.51765993789384512) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 2 ] - -0.26564637758736048) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 3 ] - 0.27011779549760634) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 4 ] - 0.085854182000892176) < 1e-4 );
   }

   //
   // checked against http://cmp.felk.cvut.cz/cmp/software/stprtool/manual/kernels/preimage/list/rbfpreimg3.html
   //
   void simplePreimageTest()
   {
      typedef std::vector<double>               Point;
      typedef std::vector<Point>                Points;
      typedef nll::algorithm::KernelRbf<Point>   Kernel;
      typedef nll::algorithm::KernelPca<Point, Kernel>  KernelPca;

      const unsigned size = nll::core::getStaticBufferSize( test_data2 );
      Points points;
      for ( unsigned n = 0; n < size; ++n )
      {
         points.push_back( nll::core::make_vector<double>( test_data2[ n ][ 0 ], test_data2[ n ][ 1 ] ) );
      }

      KernelPca kpca;
      Kernel rbfKernel( 2.0 );
      kpca.compute( points, 5, rbfKernel );


      nll::algorithm::KernelPreImageMDS<Point, Kernel> preimageGenerator( kpca );
      const unsigned sizeTest = nll::core::getStaticBufferSize( test_points_expected );
      for ( size_t n = 0; n < sizeTest; ++n )
      {
         nll::core::Buffer1D<double> f = kpca.transform( nll::core::make_vector<double>( test_points[ n ][ 0 ], test_points[ n ][ 1 ] ) );
         Point res = preimageGenerator.preimage( f, 3 );

         TESTER_ASSERT( fabs( res[ 0 ] - test_points_expected[ n ][ 0 ] ) < 1e-2 );
         TESTER_ASSERT( fabs( res[ 1 ] - test_points_expected[ n ][ 1 ] ) < 1e-2 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKernelPca);
TESTER_TEST(simpleTest);
TESTER_TEST(simplePreimageTest);
TESTER_TEST_SUITE_END();
#endif
