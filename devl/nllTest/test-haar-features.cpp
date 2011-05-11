#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;




class TestHaarFeature
{
public:
   void testBasic()
   {
      // create a basic image
      Image<ui8> i( 8, 10, 1 );
      for ( ui32 m = 0; m < i.sizey(); ++m )
         for ( ui32 n = 0; n < i.sizex(); ++n )
            i( n, m, 0 ) = static_cast<ui8>( n + m * i.sizex() + 1 );

      // computes the Integral image
      IntegralImage integral;
      integral.process( i );
      
      srand( 0 );
      for ( ui32 n = 0; n < 500; ++n )
      {
         
         ui32 x1 = rand() % i.sizex();
         ui32 x2 = rand() % i.sizex();
         ui32 y1 = rand() % i.sizey();
         ui32 y2 = rand() % i.sizey();
         
         if ( x1 > x2 )
            std::swap( x1, x2 );
         if ( y1 > y2 )
            std::swap( y1, y2 );

         double sum = 0;
         for ( ui32 ny = y1; ny <= y2; ++ny )
         {
            for ( ui32 nx = x1; nx <= x2; ++nx )
            {
               sum += i( nx, ny, 0 );
            }
         }

         TESTER_ASSERT( equal<double>( integral.getSum( vector2ui( x1, y1 ), vector2ui( x2, y2 ) ), sum ) );
      }
   }

   void testHaar()
   {
      // create a basic image
      Image<ui8> i1( 8, 8, 1 );
      Image<ui8> i2( 8, 8, 1 );
      for ( ui32 m = 0; m < i1.sizey(); ++m )
         for ( ui32 n = 0; n < i1.sizex(); ++n )
         {
            i1( n, m, 0 ) = static_cast<ui8>( n );
            i2( n, m, 0 ) = static_cast<ui8>( m );
         }

      HaarFeatures2d::Features features;
      features.push_back( HaarFeatures2d::Feature( HaarFeatures2d::Feature::HORIZONTAL, vector2d( 0, 0 ), vector2d( 0.5, 0.5 ) ) );
      features.push_back( HaarFeatures2d::Feature( HaarFeatures2d::Feature::VERTICAL, vector2d( 0, 0 ), vector2d( 0.5, 0.5 ) ) );

      HaarFeatures2d::Buffer result1 = HaarFeatures2d::process( features, i1 );
      HaarFeatures2d::Buffer result2 = HaarFeatures2d::process( features, i2 );

      TESTER_ASSERT( equal<double>( result1[ 0 ], 5 * ( 1 + 2 ) - 5 * ( 2 + 3 + 4 ) ) );
      TESTER_ASSERT( equal<double>( result1[ 1 ], 0 ) );
      TESTER_ASSERT( equal<double>( result2[ 0 ], 0 ) );
      TESTER_ASSERT( equal<double>( result2[ 1 ], 5 * ( 1 + 2 ) - 5 * ( 2 + 3 + 4 ) ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestHaarFeature);
TESTER_TEST(testBasic);
TESTER_TEST(testHaar);
TESTER_TEST_SUITE_END();
#endif
