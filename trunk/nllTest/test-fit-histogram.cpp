#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

class TestHistogramFit
{
public:
   std::vector<double> readHistogram( const std::string& file )
   {
      std::vector<double> points;
      std::ifstream f( file.c_str() );
      if ( !f.good() )
         throw std::runtime_error( "can't open the file" );
      while ( !f.eof() )
      {
         std::string str;
         std::getline( f, str );
         if ( str != "" )
         {
            points.push_back( atof( str.c_str() ) );
         }
      }

      return points;
   }

   void testGaussian1()
   {
      srand(0);
      std::vector<double> points = readHistogram(NLL_TEST_PATH "data/histogram/gaussian1.txt");

      algorithm::HistogramFitGaussian fiter;
      fiter.fit( points, 1, 0 );

      std::cout << "mean value=" << fiter.getGaussians()[ 0 ].mean[ 0 ] << std::endl;
      std::cout << "mean index=" << fiter.getGaussians()[ 0 ].mean[ 1 ] << std::endl;
      TESTER_ASSERT( fabs( fiter.getGaussians()[ 0 ].mean[ 1 ] - 72 ) < 10 );
   }

   void testGaussian2()
   {
      srand(0);
      std::vector<double> points = readHistogram(NLL_TEST_PATH "data/histogram/gaussian1.txt");

      algorithm::HistogramFitGaussian fiter;
      fiter.fit( points, 2, 0 );

      std::cout << "mean value=" << fiter.getGaussians()[ 0 ].mean[ 0 ] << std::endl;
      std::cout << "mean index=" << fiter.getGaussians()[ 0 ].mean[ 1 ] << std::endl;

      std::cout << "mean value=" << fiter.getGaussians()[ 1 ].mean[ 0 ] << std::endl;
      std::cout << "mean index=" << fiter.getGaussians()[ 1 ].mean[ 1 ] << std::endl;
      TESTER_ASSERT( fabs( fiter.getGaussians()[ 0 ].mean[ 1 ] - 72 ) < 10 );
      TESTER_ASSERT( fabs( fiter.getGaussians()[ 1 ].mean[ 1 ] - 100 ) < 10 );
   }

   void testGaussian3()
   {
      srand(0);
      std::vector<double> points = readHistogram(NLL_TEST_PATH "data/histogram/gaussian2.txt");

      algorithm::HistogramFitGaussian fiter;
      fiter.fit( points, 2, 0 );

      std::cout << "mean value=" << fiter.getGaussians()[ 0 ].mean[ 0 ] << std::endl;
      std::cout << "mean index=" << fiter.getGaussians()[ 0 ].mean[ 1 ] << std::endl;

      std::cout << "mean value=" << fiter.getGaussians()[ 1 ].mean[ 0 ] << std::endl;
      std::cout << "mean index=" << fiter.getGaussians()[ 1 ].mean[ 1 ] << std::endl;
      TESTER_ASSERT( fabs( fiter.getGaussians()[ 0 ].mean[ 1 ] - 2 ) < 10 );
      TESTER_ASSERT( fabs( fiter.getGaussians()[ 1 ].mean[ 1 ] - 62 ) < 10 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestHistogramFit);
TESTER_TEST(testGaussian3);
TESTER_TEST(testGaussian2);
TESTER_TEST(testGaussian1);
TESTER_TEST_SUITE_END();
#endif
