#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;



class TestIndependentComponentAnalysis
{
public:
   static double sourceSinus( double val )
   {
      return sin( val + 1.5 );
   }

   static double sourceJagged( double val )
   {
      val = val + 10;
      int vali = int( val );
      return ( val - (double)vali ) - 0.5;
   }
/*
   static double sourceJagged( double val )
   {
      val = val + 10;
      int vali = int( val );
      return -( val - (double)vali ) - 0.5;
   }*/

   // create 2 source one is a sinus function, the other one is a jagged function X
   // mix the signal with W
   // ICA find W^-1, with component order or sign not relevant
   void testBasic()
   {
      for ( ui32 nn = 0; nn < 10; ++nn )
      {
         srand( time( 0 ) );
         algorithm::IndependentComponentAnalysis<> pci;

         typedef std::vector<double>   Point;
         typedef std::vector<Point>    Points;
         typedef core::Matrix<double>  Matrix;

         Matrix mixingSource( 2, 2 );
         mixingSource( 0, 0 ) = 0.6;
         mixingSource( 0, 1 ) = 0.4;
         mixingSource( 1, 0 ) = 0.4;
         mixingSource( 1, 1 ) = 0.6;

         const ui32 nbPoints = 1000;
         Points origSignals( nbPoints );
         Points mixedSignals( nbPoints );
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            const double val = n / 20.0;
            origSignals[ n ] = core::make_vector<double>( sourceSinus( val ), sourceJagged( val ) );
            mixedSignals[ n ] = core::make_vector<double>( mixingSource( 0, 0 ) * origSignals[ n ][ 0 ] + mixingSource( 0, 1 ) * origSignals[ n ][ 1 ],
                                                           mixingSource( 1, 0 ) * origSignals[ n ][ 0 ] + mixingSource( 1, 1 ) * origSignals[ n ][ 1 ] );
         }

         algorithm::IndependentComponentAnalysis<> pic;
         pci.compute( mixedSignals, 2 );

         for ( ui32 n = 0; n < pci.getUnmixingMatrix().size(); ++n )
         {
            for ( ui32 nn = 0; nn < pci.getUnmixingMatrix()[ 0 ].size(); ++nn )
            {
               std::cout << pci.getUnmixingMatrix()[ n ][ nn ] << " ";
            }
            std::cout << std::endl;
         }

         core::inverse( mixingSource );   // find the unmixing matrix
         mixingSource /= sqrt( core::sqr( mixingSource( 0, 0 ) ) +
                               core::sqr( mixingSource( 0, 1 ) ) );

         // we have rotated the data with PCA, so we also must rotate the weight matrix
         Matrix mixingSourceR = pci.getPcaTransform().getProjection() * mixingSource;
         core::transpose( mixingSourceR );

         mixingSource.print( std::cout );
         std::cout << "---" << std::endl;
         mixingSourceR.print( std::cout );
         
         // compare the result: we must not take into account the sign or position. We test only the first component
         // as the second one is orthogonal (no degree of freedom so it is entirely determined by the first component
         // and this is why we need to use a symetric mixing matrix)
         const double min0 = fabs( fabs( mixingSourceR( 0, 0 ) ) - fabs( pci.getUnmixingMatrix()[ 0 ][ 0 ] ) ) +
                             fabs( fabs( mixingSourceR( 0, 1 ) ) - fabs( pci.getUnmixingMatrix()[ 0 ][ 1 ] ) );
         const double min1 = fabs( fabs( mixingSourceR( 0, 0 ) ) - fabs( pci.getUnmixingMatrix()[ 0 ][ 1 ] ) ) +
                             fabs( fabs( mixingSourceR( 0, 1 ) ) - fabs( pci.getUnmixingMatrix()[ 0 ][ 0 ] ) );

         TESTER_ASSERT( fabs( min0 ) < 0.2 || fabs( min1 ) < 0.2 );
         Point p = pci.transform( mixedSignals[ 0 ] );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIndependentComponentAnalysis);
TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif