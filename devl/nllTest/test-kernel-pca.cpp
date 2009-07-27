#include "stdafx.h"
#include <nll/nll.h>


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
      v.print( std::cout );
   }

   /**
    Compute a dummy kpca. Check the kernel matrix is correctly centered in feature space.
    We want sum H(x_k) = 0
    => sum H(x_k)H(x_i) = 0, for i=0..size
    => sum kernel(x_k, x_i) = 0 for i=0..size

    //Test if kernel matrix is centered in feature space
    Eigen vectors in feature space are normalized
    */
   /*
   void testCentered()
   {
      typedef std::vector<double>               Point;
      typedef std::vector<Point>                Points;
      //typedef nll::algorithm::KernelPolynomial<Point>   Kernel;
      typedef nll::algorithm::KernelRbf<Point>   Kernel;
      typedef nll::algorithm::KernelPca<Point, Kernel>  KernelPca;

      // we want to keep the values small as we are using a polynomial kernel of degree 10
      Points points;
      points.push_back( nll::core::make_vector<double>( 2, 1.1 ) );
      points.push_back( nll::core::make_vector<double>( 1, 1 ) );
      points.push_back( nll::core::make_vector<double>( -1, 0.9 ) );
      points.push_back( nll::core::make_vector<double>( 1.5, 1.2 ) );
      //points.push_back( nll::core::make_vector<double>( 1.8, 1.01 ) );
      //points.push_back( nll::core::make_vector<double>( 2, 1.2 ) );

      KernelPca kpca;
      Kernel rbfKernel( 0.1 );
      kpca.compute( points, 3, rbfKernel );

      // test kernel is centered
      nll::core::Matrix<double> kernel = kpca.getKernelMatrix();
      for ( unsigned n = 0; n < points.size(); ++n )
      {
         double sum = 0;
         for ( unsigned nn = 0; nn < points.size(); ++nn )
            sum += kernel( n, nn );
         std::cout << "sum=" << sum << std::endl;
         TESTER_ASSERT( nll::core::equal<double>( sum, 0, 1e-5 ) );
      }

      // test orthogonal eig
      for ( unsigned nb = 0; nb < kpca.getEigenValues().size(); ++nb )
      {
         for ( unsigned nb2 = nb + 1; nb2 < kpca.getEigenValues().size(); ++nb2 )
         {
            Point p( kpca.getEigenVectors().sizey() );
            for ( unsigned n = 0; n < kpca.getEigenVectors().sizey(); ++n )
            {
               p[ n ] = kpca.getEigenVectors()( n, nb );
            }

            Point p2( kpca.getEigenVectors().sizey() );
            for ( unsigned n = 0; n < kpca.getEigenVectors().sizey(); ++n )
            {
               p2[ n ] = kpca.getEigenVectors()( n, nb2 );
            }
            TESTER_ASSERT( nll::core::equal<double>( rbfKernel( p, p2 ), 0, 1e-5 ) );
         }
      }

      nll::core::Buffer1D<float> point = nll::core::make_buffer1D<float>( 1.5f, 0.9f );
      KernelPca::Vector v = kpca.transform( point );
      v.print( std::cout );
   }

   void test2()
   {
      nll::core::Matrix<double> points(3, 3);
      points( 0, 0 ) = 4;
      points( 0, 2 ) = 10;
      points( 0, 1 ) = 1;

      points( 1, 0 ) = 8;
      points( 1, 2 ) = 10.1;
      points( 1, 1 ) = 1;

      points( 2, 0 ) = 6;
      points( 2, 2 ) = 9.9;
      points( 2, 1 ) = 1;
      nll::core::Matrix<double> cov = nll::core::covariance( points, 0, 2 );

      nll::core::Buffer1D<double> eiv;
      nll::core::Matrix<double> eig;

      nll::core::svdcmp( cov, eiv, eig );
      eig.print( std::cout );
      eiv.print( std::cout );
   }
   */
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKernelPca);
TESTER_TEST(simpleTest);
//TESTER_TEST(testCentered);
//TESTER_TEST(test2);
TESTER_TEST_SUITE_END();
//#endif
