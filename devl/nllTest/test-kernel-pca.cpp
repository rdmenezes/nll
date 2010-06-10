#include <nll/nll.h>
#include <tester/register.h>


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

      Points points;
      points.push_back( nll::core::make_vector<double>( 2, 1.1 ) );
      points.push_back( nll::core::make_vector<double>( 1, 1 ) );
      points.push_back( nll::core::make_vector<double>( -1, 0.9 ) );
      points.push_back( nll::core::make_vector<double>( 1.5, 1.2 ) );

      KernelPca kpca;
      Kernel rbfKernel( 0.1 );
      kpca.compute( points, 3, rbfKernel );

      nll::core::Buffer1D<float> point = nll::core::make_buffer1D<float>( 1.5f, 0.9f );
      KernelPca::Vector v = kpca.transform( point );
      
      // TODO VALIDATE AGAINST MATLAB PROTOTYPE
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKernelPca);
TESTER_TEST(simpleTest);
TESTER_TEST_SUITE_END();
#endif
