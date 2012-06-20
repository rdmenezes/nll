#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

class TestGaussianPdf
{
   typedef core::ProbabilityDistributionFunctionGaussian<double> Pdf;
   typedef Pdf::Matrix  Matrix;
   typedef Pdf::Vector  Vector;

public:
   static double dummyPdf( const Vector& x, const Vector& mean, const Matrix& cov, bool& isBad )
   {
      double det = 0;
      Matrix covinv;
      covinv.clone( cov );
      bool inv = core::inverse( covinv, &det );
      if ( !inv || det <= 0 )
      {
         isBad = true;
         return 0;
      }

      Vector tmp( x.size() );
      for ( ui32 n = 0; n < x.size(); ++n )
      {
         tmp[ n ] = x[ n ] - mean[ n ];
      }

      const double cte = std::pow( ( 2 * core::PI ), -static_cast<double>( x.size() ) / 2 );
      const double val = core::fastDoubleMultiplication( tmp, covinv );
      if ( val <= 0 )
      {
         isBad = true;
         return 0;
      }
      const double expeval = std::exp( -0.5 * val );
      const double expected = cte / sqrt( det ) * expeval;
      return expected;
   }

   void testCovFullFullMean()
   {
      srand( 0 );
      ui32 nbTests = 0;
      for ( ui32 nn = 0; nn < 100; ++nn )
      {
         const ui32 size = 3;
         Matrix cov( size, size );

         for ( ui32 n = 0; n < size * size; ++n )
         {
            cov[ n ] = core::generateUniformDistribution( 0.1, 1.1 );
         }

         Vector mean( size );
         for ( ui32 n = 0; n < size; ++n )
            mean[ n ] = core::generateUniformDistribution( -1, 1 );

         for ( ui32 nnn = 0; nnn < 500; ++nnn )
         {
            Vector x( size );
            for ( ui32 n = 0; n < size; ++n )
              x[ n ] = core::generateUniformDistribution( -1, 1 );

            bool isWrong = false;
            const double expected = dummyPdf( x, mean, cov, isWrong );
            if ( isWrong )
               continue;

            Pdf pdf( cov, mean );
            const double result = pdf.eval( x );
            bool isEqual = core::equal( result, expected, 1e-10 );
            if (!isEqual)
            {
               std::cout << "Error step=" << nn << ", found=" << result << " expected=" << expected << std::endl;
            }
            TESTER_ASSERT( isEqual );
            ++nbTests;
         }
      }

      TESTER_ASSERT( nbTests > 500 );
   }

   void testCovFullNoMean()
   {
      srand( 0 );
      ui32 nbTests = 0;
      for ( ui32 nn = 0; nn < 100; ++nn )
      {
         const ui32 size = 3;
         Matrix cov( size, size );

         for ( ui32 n = 0; n < size * size; ++n )
         {
            cov[ n ] = core::generateUniformDistribution( 0.1, 1.1 );
         }

         Vector mean( size );
         for ( ui32 n = 0; n < size; ++n )
            mean[ n ] = 0;

         for ( ui32 nnn = 0; nnn < 500; ++nnn )
         {
            Vector x( size );
            for ( ui32 n = 0; n < size; ++n )
               x[ n ] = core::generateUniformDistribution( -1, 1 );
            bool isWrong = false;
            const double expected = dummyPdf( x, mean, cov, isWrong );
            if ( isWrong )
               continue;

            Pdf pdf( cov, mean );
            const double result = pdf.eval( x );
            bool isEqual = core::equal( result, expected, 1e-10 );
            if (!isEqual)
            {
               std::cout << "Error step=" << nn << ", found=" << result << " expected=" << expected << std::endl;
            }
            TESTER_ASSERT( isEqual );
            ++nbTests;
         }
      }

      TESTER_ASSERT( nbTests > 500 );
   }

   void testCovNoFullFullMean()
   {
      srand( 0 );
      ui32 nbTests = 0;
      for ( ui32 nn = 0; nn < 100; ++nn )
      {
         const ui32 size = 3;
         Matrix cov( size, size );

         for ( ui32 y = 0; y < size; ++y )
         {
            for ( ui32 x = 0; x < size; ++x )
            {
               if ( x == y )
               {
                  cov( y, x ) = core::generateUniformDistribution( 0.1, 1.1 );
               }
            }
         }

         Vector mean( size );
         for ( ui32 n = 0; n < size; ++n )
            mean[ n ] = core::generateUniformDistribution( -1, 1 );

         for ( ui32 nnn = 0; nnn < 500; ++nnn )
         {
            Vector x( size );
            for ( ui32 n = 0; n < size; ++n )
               x[ n ] = core::generateUniformDistribution( -1, 1 );

            bool isWrong = false;
            const double expected = dummyPdf( x, mean, cov, isWrong );
            if ( isWrong )
               continue;

            Pdf pdf( cov, mean );
            const double result = pdf.eval( x );
            bool isEqual = core::equal( result, expected, 1e-10 );
            if (!isEqual)
            {
               std::cout << "Error step=" << nn << ", found=" << result << " expected=" << expected << std::endl;
            }
            TESTER_ASSERT( isEqual );
            ++nbTests;
         }
      }

      TESTER_ASSERT( nbTests > 500 );
   }

   void testCovNoFullNoFullMean()
   {
      srand( 0 );
      ui32 nbTests = 0;
      for ( ui32 nn = 0; nn < 100; ++nn )
      {
         const ui32 size = 3;
         Matrix cov( size, size );

         for ( ui32 y = 0; y < size; ++y )
         {
            for ( ui32 x = 0; x < size; ++x )
            {
               if ( x == y )
               {
                  cov( y, x ) = core::generateUniformDistribution( 0.1, 1.1 );
               }
            }
         }

         Vector mean( size );
         for ( ui32 n = 0; n < size; ++n )
            mean[ n ] = 0;

         for ( ui32 nnn = 0; nnn < 500; ++nnn )
         {
            bool isWrong = false;

            Vector x( size );
            for ( ui32 n = 0; n < size; ++n )
               x[ n ] = core::generateUniformDistribution( -1, 1 );

            const double expected = dummyPdf( x, mean, cov, isWrong );
            if ( isWrong )
               continue;

         
            Pdf pdf( cov, mean );
            const double result = pdf.eval( x );
            bool isEqual = core::equal( result, expected, 1e-10 );
            if (!isEqual)
            {
               std::cout << "Error step=" << nn << ", found=" << result << " expected=" << expected << std::endl;
            }
            TESTER_ASSERT( isEqual );
            ++nbTests;
         }
      }

      TESTER_ASSERT( nbTests > 500 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianPdf);
TESTER_TEST(testCovFullFullMean);
TESTER_TEST(testCovFullNoMean);
TESTER_TEST(testCovNoFullFullMean);
TESTER_TEST(testCovNoFullNoFullMean);
TESTER_TEST_SUITE_END();
#endif