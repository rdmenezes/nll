#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;



class TestIndependentComponentAnalysis
{
public:
   static double sourceSinus( double val )
   {
      return sin( val + 1.5 );
   }

   static double sourceLogistic( double )
   {
      return core::generateLogisticDistribution( 0, 1 );
   }

   static double sourceGaussian( double )
   {
      return core::generateGaussianDistribution( 0, 1 );
   }

   static double sourceJagged( double val )
   {
      val = val;
      int vali = int( val );
      return ( val - (double)vali ) - 0.5;
   }

   static double sourceJagged4( double val )
   {
      val = val;
      int vali = int( val );
      return ( ( val - (double)vali ) - 0.5 ) * 2;
   }

   static double sourceJagged3( double val )
   {
      val = val + 10;
      int vali = int( val );
      return ( val - (double)vali ) - 0.5;
   }

   static double sourceJagged2( double val )
   {
      val = val + 10;
      int vali = int( val );
      return -( val - (double)vali ) - 0.5;
   }

   typedef double (*pfunc)( double );

   void testBasic()
   {
      _testBasic( sourceSinus, sourceJagged, 0.55 );

      srand( time( 0 ) );
      for ( double n = 0.3; n < 0.49; n+= 0.02 )
      {
         //_testBasic( sourceSinus, sourceJagged4, n );
         _testBasic( sourceSinus, sourceJagged, n );
         //_testBasic( sourceSinus, sourceGaussian, n + 0.25 );
      }

      
      _testBasic( sourceSinus, sourceJagged3, 0.3 );
      _testBasic( sourceSinus, sourceJagged, 0.3 );

      _testBasic( sourceSinus, sourceJagged3, 0.4 );
      _testBasic( sourceSinus, sourceJagged, 0.4 );

      _testBasic( sourceSinus, sourceJagged3, 0.6 );
      _testBasic( sourceSinus, sourceJagged, 0.6 );

      _testBasic( sourceSinus, sourceJagged3, 0.7 );
      _testBasic( sourceSinus, sourceJagged, 0.7 );
      
   }

   /**
    @brief Assume point is a set of random variable at t=0...size,
           computes E(f(x))E(f(y)) - E(f(x)(f(y)) (1)
    @note if 2 random variable are independent, they must satisfy (1) == 0 for any function f
    */
   template <class Points, class Function>
   double checkIndependence( const Points& points, ui32 v1, ui32 v2, const Function& f )
   {
      double xa = 0;
      double xb = 0;
      double xc = 0;
      for ( ui32 n = 0; n < points.size(); ++n )
      {
         xa += points[ n ][ v1 ];
         xb += points[ n ][ v2 ];
         xc += points[ n ][ v1 ] * points[ n ][ v2 ];
      }
      return xa / points.size() * xb / points.size() - xc / points.size();
   }

   // create 2 source one is a sinus function, the other one is a jagged function X
   // mix the signal with W
   // ICA find W^-1, with component order or sign not relevant
   void _testBasic( pfunc f1, pfunc f2, double val )
   {
      for ( ui32 nn = 0; nn < 1; ++nn )
      {
         algorithm::IndependentComponentAnalysis<> pci;

         typedef std::vector<double>   Point;
         typedef std::vector<Point>    Points;
         typedef core::Matrix<double>  Matrix;

         Matrix mixingSource( 2, 2 );
         mixingSource( 0, 0 ) = val;
         mixingSource( 0, 1 ) = 1 - val;
         mixingSource( 1, 0 ) = 1 - val;
         mixingSource( 1, 1 ) = val;

         const ui32 nbPoints = 10000;
         Points origSignals( nbPoints );
         Points mixedSignals( nbPoints );

         Matrix cmp1( nbPoints, 1 );
         Matrix cmp2( nbPoints, 1 );
         Matrix cmp3( nbPoints, 1 );
         Matrix cmp4( nbPoints, 1 );
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            const double val = n / 20.0;
            origSignals[ n ] = core::make_vector<double>( f1( val ), f2( val ) );
            mixedSignals[ n ] = core::make_vector<double>( mixingSource( 0, 0 ) * origSignals[ n ][ 0 ] + mixingSource( 0, 1 ) * origSignals[ n ][ 1 ],
                                                           mixingSource( 1, 0 ) * origSignals[ n ][ 0 ] + mixingSource( 1, 1 ) * origSignals[ n ][ 1 ] );

            cmp1[ n ] = origSignals[ n ][ 0 ];
            cmp2[ n ] = origSignals[ n ][ 1 ];
            cmp3[ n ] = mixedSignals[ n ][ 0 ];
            cmp4[ n ] = mixedSignals[ n ][ 1 ];
         }

         std::cout << "correlation orig=" << core::correlation( cmp1, cmp2 ) << std::endl;
         std::cout << "correlation mixed=" << core::correlation( cmp3, cmp4 ) << std::endl;
         std::cout << "correlation orig kurtosis1=" << core::kurtosis( cmp1 ) << std::endl;
         std::cout << "correlation orig kurtosis2=" << core::kurtosis( cmp2 ) << std::endl;
         std::cout << "correlation mixed kurtosis1=" << core::kurtosis( cmp3 ) << std::endl;
         std::cout << "correlation mixed kurtosis2=" << core::kurtosis( cmp4 ) << std::endl;

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

         Matrix unmixingSource;
         unmixingSource.clone( mixingSource );
         //unmixingSource( 1, 0 ) = -unmixingSource( 1, 0 );
         unmixingSource.print( std::cout );

         /*
         mixingSource /= sqrt( core::sqr( mixingSource( 0, 0 ) ) +
                               core::sqr( mixingSource( 0, 1 ) ) );


         // we have rotated the data with PCA, so we also must rotate the weight matrix
         Matrix mixingSourceR = pci.getPcaTransform().getProjection() * mixingSource;
         core::transpose( mixingSourceR );

         mixingSource.print( std::cout );
         //std::cout << "---" << std::endl;
         mixingSourceR.print( std::cout );
         */

         Points unmixedSignals( nbPoints );
         Points transformed;
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            transformed.push_back( pci.transform( mixedSignals[ n ] ) );
            unmixedSignals[ n ] = core::make_vector<double>( unmixingSource( 0, 0 ) * mixedSignals[ n ][ 0 ] + unmixingSource( 0, 1 ) * mixedSignals[ n ][ 1 ],
                                                             unmixingSource( 1, 0 ) * mixedSignals[ n ][ 0 ] + unmixingSource( 1, 1 ) * mixedSignals[ n ][ 1 ] );
         }

         std::cout << "indenpendece before=" << checkIndependence( mixedSignals, 0, 1, pci.getConstrastFunction() ) << std::endl;
         std::cout << "indenpendece after=" << checkIndependence( transformed, 0, 1, pci.getConstrastFunction() ) << std::endl;

         core::Image<ui8> im1 = displaySignal( origSignals, 0, 300, 3 );
         core::writeBmp( im1, "c:/temp2/original0.bmp" );
         core::Image<ui8> im2 = displaySignal( origSignals, 1, 300, 3 );
         core::writeBmp( im2, "c:/temp2/original1.bmp" );

         core::Image<ui8> im1m = displaySignal( mixedSignals, 0, 300, 3 );
         core::writeBmp( im1m, "c:/temp2/mixedSignals0.bmp" );
         core::Image<ui8> im2m = displaySignal( mixedSignals, 1, 300, 3 );
         core::writeBmp( im2m, "c:/temp2/mixedSignals1.bmp" );

         core::Image<ui8> im1t = displaySignal( transformed, 0, 300, 3 );
         core::writeBmp( im1t, "c:/temp2/transformed0.bmp" );
         core::Image<ui8> im2t = displaySignal( transformed, 1, 300, 3 );
         core::writeBmp( im2t, "c:/temp2/transformed1.bmp" );

         core::Image<ui8> im1tt = displaySignal( unmixedSignals, 0, 300, 3 );
         core::writeBmp( im1tt, "c:/temp2/test0.bmp" );
         core::Image<ui8> im2tt = displaySignal( unmixedSignals, 1, 300, 3 );
         core::writeBmp( im2tt, "c:/temp2/test1.bmp" );


         /*
         // compare the result: we must not take into account the sign or position. We test only the first component
         // as the second one is orthogonal (no degree of freedom so it is entirely determined by the first component
         // and this is why we need to use a symetric mixing matrix)
         const double min0 = fabs( fabs( mixingSourceR( 0, 0 ) ) - fabs( pci.getUnmixingMatrix()[ 0 ][ 0 ] ) ) +
                             fabs( fabs( mixingSourceR( 0, 1 ) ) - fabs( pci.getUnmixingMatrix()[ 0 ][ 1 ] ) );
         const double min1 = fabs( fabs( mixingSourceR( 0, 0 ) ) - fabs( pci.getUnmixingMatrix()[ 0 ][ 1 ] ) ) +
                             fabs( fabs( mixingSourceR( 0, 1 ) ) - fabs( pci.getUnmixingMatrix()[ 0 ][ 0 ] ) );
         

         TESTER_ASSERT( fabs( min0 ) < 0.1 || fabs( min1 ) < 0.1 );*/
         Point p = pci.transform( mixedSignals[ 0 ] );
      }
   }

   template <class Points>
   core::Image<ui8> displaySignal( const Points& points, const ui32 component, const ui32 nbPoints, const double maxy )
   {
      core::Image<ui8> i1( nbPoints, 50, 3 );
      const double dy = maxy / i1.sizey();
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         const double y = points[ n ][ component ] / dy + i1.sizey() / 2;
         if ( y >= 0 && y < i1.sizey() )
         {
            i1( n, y, 0 ) = 255;
            i1( n, y, 1 ) = 255;
            i1( n, y, 2 ) = 255;
         }
      }

      return i1;
   }
/*
   void testImage()
   {
      
      typedef std::vector<double>   Point;
      typedef std::vector<Point>    Points;
      typedef core::Matrix<double>  Matrix;

      const std::string pim1 = NLL_TEST_PATH "data/ica/i1.bmp";
      const std::string pim2 = NLL_TEST_PATH "data/ica/i2.bmp";
      const std::string pim3 = NLL_TEST_PATH "data/ica/i3.bmp";

      core::Image<ui8> i1;
      core::readBmp( i1, pim1 );

      core::Image<ui8> i2;
      core::readBmp( i2, pim2 );

      core::Image<ui8> i3;
      core::readBmp( i3, pim3 );

      Points points;
      assert( i1.size() == i2.size() && i1.size() == i3.size() );
      for ( ui32 n = 0; n < i1.size(); n+= 3 )
      {
         points.push_back( core::make_vector<double>( i1[ n ], i2[ n ], i3[ n ] ) );
      }

      algorithm::IndependentComponentAnalysis<> pci;

   }*/

   void testImage()
   {
      srand(time(0));
      typedef std::vector<double>   Point;
      typedef std::vector<Point>    Points;
      typedef core::Matrix<double>  Matrix;

      Points points;
      points.push_back( core::make_vector<double>( 1, 2 ) );
      points.push_back( core::make_vector<double>( 3, 8 ) );
      points.push_back( core::make_vector<double>( -1, 1 ) );
      points.push_back( core::make_vector<double>( 4, 6 ) );

      algorithm::IndependentComponentAnalysis<> pci;
      pci.compute( points, 2 );

   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIndependentComponentAnalysis);
TESTER_TEST(testBasic);
//TESTER_TEST(testImage);
TESTER_TEST_SUITE_END();
#endif