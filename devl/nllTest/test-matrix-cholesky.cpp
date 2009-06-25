#include "stdafx.h"
#include <nll/nll.h>

class TestMatrixCholesky
{
public:
   /**
    test the cholesky decomposition on a serie of covariance matrices
    */
   void testMatrixCholesky2()
   {
      srand( 2 );
      typedef nll::core::Matrix<double, nll::core::IndexMapperRowMajorFlat2D> Matrix;
      typedef std::vector<double> Point;
      typedef std::vector<Point>  Points;
      
      for ( unsigned n = 0; n < 100; ++n )
      {
         const double meanx = ( ( rand() / (double)RAND_MAX ) - 0.5 ) * 100;
         const double meany = ( ( rand() / (double)RAND_MAX ) - 0.5 ) * 100;
         const double varx = ( rand() / (double)RAND_MAX ) * 100 + 0.1;
         const double vary = ( rand() / (double)RAND_MAX ) * 100 + 0.1;

         const unsigned nbPoints = 100;
         Points p( nbPoints );
         for ( unsigned n = 0; n < nbPoints; ++n )
         {
            double dx = nll::core::generateGaussianDistribution( meanx, varx );
            double dy = nll::core::generateGaussianDistribution( meany, vary );
            p[ n ] = nll::core::make_vector<double>( dx, dy );
         }

         nll::algorithm::Gmm gmm;
         gmm.em( p, 2, 1, 5 );


         Matrix cov;
         cov.clone( gmm.getGaussians()[ 0 ].covariance );

         nll::core::Buffer1D<nll::ui32> index;
         bool success = nll::core::choleskyDecomposition( cov );
         TESTER_ASSERT( success );

         Matrix covt;
         covt.clone( cov );
         nll::core::transpose( covt );
         cov = cov * covt;
         TESTER_ASSERT( cov.equal( gmm.getGaussians()[ 0 ].covariance, 1e-5 ) );
      }
    }

   /**
    Test a know matrix
    */
   void testMatrixCholesky()
   {
      double vals[] =
      {
         1,  1,  1,  1,
         1,  5,  5,  5,
         1,  5, 14, 14,
         1,  5, 14, 15
      };

      double ref[] =
      {
         1, 0, 0, 0,
         1, 2, 0, 0,
         1, 2, 3, 0,
         1, 2, 3, 1
      };
      nll::core::Matrix<double> m( nll::core::Buffer1D<double>( vals, 16, false ), 4, 4 );
      nll::core::Matrix<double, nll::core::IndexMapperRowMajorFlat2D> refmat( nll::core::Buffer1D<double>( ref, 16, false ), 4, 4 );

      bool success = nll::core::choleskyDecomposition( m );
      TESTER_ASSERT( success );
      TESTER_ASSERT( m.equal( refmat, 1e-5 ) );
   }

   /**
    Test multinormal generation
    */
   void testMultinormal()
   {
      srand( 0 );
      typedef nll::core::Matrix<double, nll::core::IndexMapperRowMajorFlat2D> Matrix;
      typedef std::vector<double> Point;
      typedef std::vector<Point>  Points;
      
      for ( unsigned n = 0; n < 50; ++n )
      {
         const double meanx = ( ( rand() / (double)RAND_MAX ) - 0.5 ) * 100;
         const double meany = ( ( rand() / (double)RAND_MAX ) - 0.5 ) * 100;
         const double varx = ( rand() / (double)RAND_MAX ) * 100 + 0.1;
         const double vary = ( rand() / (double)RAND_MAX ) * 100 + 0.1;

         const unsigned nbPoints = 50000;
         Points p( nbPoints );
         for ( unsigned n = 0; n < nbPoints; ++n )
         {
            double dx = nll::core::generateGaussianDistribution( meanx, varx );
            double dy = nll::core::generateGaussianDistribution( meany, vary );
            p[ n ] = nll::core::make_vector<double>( dx, dy );
         }

         nll::algorithm::Gmm gmm;
         gmm.em( p, 2, 1, 5 );

         typedef nll::core::Buffer1D<double> Point2;
         typedef std::vector<Point2>         Points2;
         nll::core::NormalMultiVariateDistribution generator( gmm.getGaussians()[ 0 ].mean,
                                                              gmm.getGaussians()[ 0 ].covariance );
         Points2 points2( nbPoints );
         for ( unsigned n = 0; n < nbPoints; ++n )
            points2[ n ] = generator.generate();

         nll::algorithm::Gmm gmm2;

         gmm2.em( points2, 2, 1, 5 );
         
         // test the mean. It has a relatively big error. However if the number of samples is increased,
         // the error rate is decreasing
         TESTER_ASSERT( nll::core::equal<double>( meanx,
                                                  gmm2.getGaussians()[ 0 ].mean[ 0 ], 1.5 ) );
         TESTER_ASSERT( nll::core::equal<double>( meany,
                                                  gmm2.getGaussians()[ 0 ].mean[ 1 ], 1.5 ) );

         // we only test the diagonal as the other are of relatively low importance
         // and it is discarded in the EM algorithm
         TESTER_ASSERT( nll::core::equal<double>( gmm.getGaussians()[ 0 ].covariance( 0, 0 ),
                                                  gmm2.getGaussians()[ 0 ].covariance( 0, 0 ), 
                                                  gmm.getGaussians()[ 0 ].covariance( 0, 0 ) * 0.05 ) );
         TESTER_ASSERT( nll::core::equal<double>( gmm.getGaussians()[ 0 ].covariance( 1, 1 ),
                                                  gmm2.getGaussians()[ 0 ].covariance( 1, 1 ), 
                                                  gmm.getGaussians()[ 0 ].covariance( 1, 1 ) * 0.05 ) );
         std::cout << "#";
      }
    }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMatrixCholesky);
 TESTER_TEST(testMatrixCholesky);
 TESTER_TEST(testMatrixCholesky2);
 TESTER_TEST(testMultinormal);
TESTER_TEST_SUITE_END();
#endif