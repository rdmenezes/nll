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
      for ( size_t n = 0; n < x.size(); ++n )
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
      size_t nbTests = 0;
      for ( size_t nn = 0; nn < 100; ++nn )
      {
         const size_t size = 3;
         Matrix cov( size, size );

         for ( size_t n = 0; n < size * size; ++n )
         {
            cov[ n ] = core::generateUniformDistribution( 0.1, 1.1 );
         }

         Vector mean( size );
         for ( size_t n = 0; n < size; ++n )
            mean[ n ] = core::generateUniformDistribution( -1, 1 );

         for ( size_t nnn = 0; nnn < 500; ++nnn )
         {
            Vector x( size );
            for ( size_t n = 0; n < size; ++n )
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
      size_t nbTests = 0;
      for ( size_t nn = 0; nn < 100; ++nn )
      {
         const size_t size = 3;
         Matrix cov( size, size );

         for ( size_t n = 0; n < size * size; ++n )
         {
            cov[ n ] = core::generateUniformDistribution( 0.1, 1.1 );
         }

         Vector mean( size );
         for ( size_t n = 0; n < size; ++n )
            mean[ n ] = 0;

         for ( size_t nnn = 0; nnn < 500; ++nnn )
         {
            Vector x( size );
            for ( size_t n = 0; n < size; ++n )
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
      size_t nbTests = 0;
      for ( size_t nn = 0; nn < 100; ++nn )
      {
         const size_t size = 3;
         Matrix cov( size, size );

         for ( size_t y = 0; y < size; ++y )
         {
            for ( size_t x = 0; x < size; ++x )
            {
               if ( x == y )
               {
                  cov( y, x ) = core::generateUniformDistribution( 0.1, 1.1 );
               }
            }
         }

         Vector mean( size );
         for ( size_t n = 0; n < size; ++n )
            mean[ n ] = core::generateUniformDistribution( -1, 1 );

         for ( size_t nnn = 0; nnn < 500; ++nnn )
         {
            Vector x( size );
            for ( size_t n = 0; n < size; ++n )
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
      size_t nbTests = 0;
      for ( size_t nn = 0; nn < 100; ++nn )
      {
         const size_t size = 3;
         Matrix cov( size, size );

         for ( size_t y = 0; y < size; ++y )
         {
            for ( size_t x = 0; x < size; ++x )
            {
               if ( x == y )
               {
                  cov( y, x ) = core::generateUniformDistribution( 0.1, 1.1 );
               }
            }
         }

         Vector mean( size );
         for ( size_t n = 0; n < size; ++n )
            mean[ n ] = 0;

         for ( size_t nnn = 0; nnn < 500; ++nnn )
         {
            bool isWrong = false;

            Vector x( size );
            for ( size_t n = 0; n < size; ++n )
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

   void testGaussianPdf1d()
   {
      for ( size_t n = 0; n < 1000; ++n )
      {
         const double x = core::generateUniformDistribution( -10, 10 );

         const double mean = core::generateUniformDistribution( -10, 10 );
         const double variance = core::generateUniformDistribution( 0.1, 3 );
         const double stddev = sqrt( variance );

         // expected value
         const double expected = 1 / (sqrt( 2 * core::PI ) * stddev) * std::exp( - core::sqr( x - mean) / ( 2 * stddev * stddev ) );

         // set up PDF
         Matrix cov( 1, 1 );
         cov( 0, 0 ) = stddev * stddev;
         Vector meanV( 1 );
         meanV[ 0 ] = mean;
         Pdf pdf( cov, meanV );
         const double result = pdf.eval( core::make_buffer1D<double>( x ) );

         bool isEqual = core::equal( result, expected, 1e-10 );
         if (!isEqual)
         {
            std::cout << "Error step=" << n << ", found=" << result << " expected=" << expected << std::endl;
         }
         TESTER_ASSERT( isEqual );
      }
   }

   void testGaussianSampling()
   {
      srand( 0 );
      for ( size_t nn = 0; nn < 100; ++nn )
      {
         const double mean = core::generateUniformDistribution( -100, 100 );
         const double variance = core::generateUniformDistribution( 0.1, 4 );

         std::vector<double> vals;
         vals.reserve( 100000 );
         for ( size_t n = 0; n < 100000; ++n )
         {
            vals.push_back( core::generateGaussianDistribution( mean, variance ) );
         }

         // now compute the mean/variance from the sampling
         const double foundMean = std::accumulate( vals.begin(), vals.end(), 0.0 ) / vals.size();
         double accum = 0;
         for ( size_t n = 0; n < vals.size(); ++n )
         {
            accum += core::sqr( vals[ n ] - mean );
         }
         const double foundVariance = accum / vals.size();
         std::cout << "mean=" << mean << "|" << foundMean << " var=" << variance << "|" << foundVariance << std::endl;
         TESTER_ASSERT( core::equal( mean, foundMean, 0.1 ) );
         TESTER_ASSERT( core::equal( variance, foundVariance, 0.1 ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianPdf);
TESTER_TEST(testGaussianSampling);
TESTER_TEST(testGaussianPdf1d);
TESTER_TEST(testCovFullFullMean);
TESTER_TEST(testCovFullNoMean);
TESTER_TEST(testCovNoFullFullMean);
TESTER_TEST(testCovNoFullNoFullMean);
TESTER_TEST_SUITE_END();
#endif