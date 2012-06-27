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

   static double sourceSinus2( double val )
   {
      return sin( val + 1.5 ) + 1;
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
      srand( 0 );
      for ( double n = 0.11; n < 0.9; n+= 0.02 )
      {
         _testBasic( sourceSinus, sourceJagged, n );
         _testBasic( sourceSinus, sourceGaussian, n );
         _testBasic( sourceSinus, sourceJagged4, n );
         _testBasic( sourceSinus2, sourceJagged, n );
         _testBasic( sourceLogistic, sourceSinus, n );
      }
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

         const ui32 nbPoints = 301;
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

         Matrix unmixingSource;
         unmixingSource.clone( mixingSource );

         Points unmixedSignals( nbPoints );
         Points transformed;
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            transformed.push_back( pci.transform( mixedSignals[ n ] ) );
            unmixedSignals[ n ] = core::make_vector<double>( unmixingSource( 0, 0 ) * mixedSignals[ n ][ 0 ] + unmixingSource( 0, 1 ) * mixedSignals[ n ][ 1 ],
                                                             unmixingSource( 1, 0 ) * mixedSignals[ n ][ 0 ] + unmixingSource( 1, 1 ) * mixedSignals[ n ][ 1 ] );
         }
/*
         std::cout << "indenpendece before=" << core::checkStatisticalIndependence( mixedSignals, 0, 1, pci.getConstrastFunction() ) << std::endl;
         std::cout << "indenpendece after=" << core::checkStatisticalIndependence( transformed, 0, 1, pci.getConstrastFunction() ) << std::endl;
*/
         std::cout << "indenpendece before=" << core::checkStatisticalIndependence( mixedSignals, 0, 1 ) << std::endl;
         std::cout << "indenpendece after=" << core::checkStatisticalIndependence( transformed, 0, 1 ) << std::endl;

         core::Image<ui8> im1 = displaySignal( origSignals, 0, 300, 3 );
         core::writeBmp( im1, NLL_TEST_PATH "data/original0.bmp" );
         core::Image<ui8> im2 = displaySignal( origSignals, 1, 300, 3 );
         core::writeBmp( im2, NLL_TEST_PATH "data/original1.bmp" );

         core::Image<ui8> im1m = displaySignal( mixedSignals, 0, 300, 3 );
         core::writeBmp( im1m, NLL_TEST_PATH "data/mixedSignals0.bmp" );
         core::Image<ui8> im2m = displaySignal( mixedSignals, 1, 300, 3 );
         core::writeBmp( im2m, NLL_TEST_PATH "data/mixedSignals1.bmp" );

         core::Image<ui8> im1t = displaySignal( transformed, 0, 300, 3 );
         core::writeBmp( im1t, NLL_TEST_PATH "data/transformed0.bmp" );
         core::Image<ui8> im2t = displaySignal( transformed, 1, 300, 3 );
         core::writeBmp( im2t, NLL_TEST_PATH "data/transformed1.bmp" );

         core::Image<ui8> im1tt = displaySignal( unmixedSignals, 0, 300, 3 );
         core::writeBmp( im1tt, NLL_TEST_PATH "data/test0.bmp" );
         core::Image<ui8> im2tt = displaySignal( unmixedSignals, 1, 300, 3 );
         core::writeBmp( im2tt, NLL_TEST_PATH "data/test1.bmp" );

         core::exportVectorToMatlabAsRow( mixedSignals, NLL_TEST_PATH "data/mixedSignals0.txt" );
         core::exportVectorToMatlabAsRow( origSignals,  NLL_TEST_PATH "data/origSignals0.txt" );

         /*
         Points matlabRecon = core::readVectorFromMatlabAsColumn<Points>( NLL_TEST_PATH "data/res.txt" );
         core::Image<ui8> im1tm = displaySignal( matlabRecon, 0, 300, 3 );
         core::writeBmp( im1tm, "c:/temp2/matlabRecon0.bmp" );
         core::Image<ui8> im2tm = displaySignal( matlabRecon, 1, 300, 3 );
         core::writeBmp( im2tm, "c:/temp2/matlabRecon1.bmp" );
         */

         // ensure the signals are indenpendent!
         const double indendenceBefore = core::checkStatisticalIndependence( mixedSignals, 0, 1 );
         const double indendenceAfter = core::checkStatisticalIndependence( transformed, 0, 1 );
         TESTER_ASSERT( fabs( indendenceBefore ) >= fabs( indendenceAfter ) );
         TESTER_ASSERT( fabs( indendenceAfter ) < 1e-5 );
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

   // generate random square matrices, check A = u * w * v^t
   void testRandomSVD()
   {
      srand( 0 );
      typedef core::Matrix<double>     Matrix;
      typedef core::Buffer1D<double>   Vector;
      std::cout << "test SVD:";

      for ( int size = 2; size < 300; ++size )
      {
         if ( size % 10 == 0 )
            std::cout << "#";
         Matrix a( size, size, false );
         for ( ui32 n = 0; n < a.size(); ++n )
            a[ n ] = ( (double)( rand() % 100000 ) ) / 100 - 500;

         Matrix v;
         Vector w;
         Matrix aa;
         aa.clone( a );
         core::svdcmp( aa, w, v );

         Matrix eiv( size, size );
         for ( ui32 n = 0; n < (ui32)size; ++n )
            eiv( n, n ) = w[ n ];

         core::transpose( v );
         Matrix aorig = (aa * eiv * v);
         for ( ui32 n = 0; n < aorig.size(); ++n )
            TESTER_ASSERT( fabs( aorig[ n ] - a[ n ] ) < 1e-8 );
      }
   }

   // we are mixing 9 images into 9 new images, we want to find the original images
   // To test, run it and compare the images exported nllTest\data\ica-unmixed-*bmp and ica\*.bmp
   // are correctly recovered. The position of the image will be different and colors might
   // be inversed... but this is expected
   void testImage()
   {
      const ui32 nbImages = 9;
      srand( 10 );
      const std::string path = NLL_TEST_PATH "data/ica/";

      typedef core::Image<ui8>Image;
      std::vector<Image> images;

      // read the images
      const ui32 index[ 9 ] =
      {
         1, 2, 3, 4, 5, 6, 7, 8, 9
      };

      for ( ui32 n = 0; n < nbImages; ++n )
      {
         images.push_back( path + "i" + core::val2str( index[ n ] ) + ".bmp" );
      }

      algorithm::IndependentComponentAnalysis<algorithm::TraitConstrastFunctionG6> pci;

      typedef std::vector<double>   Point;
      typedef std::vector<Point>    Points;
      typedef core::Matrix<double>  Matrix;

      // create the source data
      const ui32 nbPoints = images[ 0 ].size();
      Points points( nbPoints );
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         Point point( nbImages );
         for ( ui32 nn = 0; nn < nbImages; ++nn )
            point[ nn ] = images[ nn ][ n ];
         points[ n ] = point;
      }

      // create mixing matrix
      Matrix mixing( nbImages, nbImages );
      for ( ui32 n = 0; n < nbImages; ++n )
      {
         for ( ui32 nn = 0; nn < nbImages; ++nn )
            mixing( n, nn ) = 1.0/nbImages;
         for ( ui32 nn = 0; nn < 200; ++nn )
         {
            ui32 i1 = rand() % nbImages;
            ui32 i2 = i1;
            while ( i1 == i2 )
               i2 = rand() % nbImages;
            double diff = core::generateUniformDistribution( 0.02, 0.08 );
            if ( mixing( n, i1 ) > diff + 0.08 )
            {
               mixing( n, i1 ) -= diff;
               mixing( n, i2 ) += diff;
            }
         }
      }

      std::cout << "mixing matrix=";
      mixing.print( std::cout );

      // mix the data
      Points pointsMixed( nbPoints );
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         Point p( nbImages );
         for ( ui32 nn = 0; nn < nbImages; ++nn )
         {
            double sum = 0;
            for ( ui32 nnn = 0; nnn < nbImages; ++nnn )
            {
               sum += mixing( nn, nnn ) * images[ nnn ][ n ];
            }
            p[ nn ] = sum;
         }
         pointsMixed[ n ] = p;
      }

      // run ICA
      pci.compute( pointsMixed, nbImages, 0.05 );

      // rebuild images
      //Points pointsMixed( nbPoints );
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         Point p = pci.transform( pointsMixed[ n ] );
         for ( ui32 nn = 0; nn < nbImages; ++nn )
         {
           // if ( n % 100  == 0 )
           // {
           //    std::cout << "p[ nn ]=" << p[ nn ] << std::endl;
           // }
            images[ nn ][ n ] = (ui8)NLL_BOUND( ( p[ nn ] * 10 ) + 128, 0, 255 );
         }
      }

      for ( ui32 nn = 0; nn < nbImages; ++nn )
      {
         core::writeBmp( images[ nn ], NLL_TEST_PATH "data/ica-unmixed-" + core::val2str( nn ) + ".bmp" );
      }

      /*
      // MATLAB check
      core::exportVectorToMatlabAsRow( pointsMixed, "c:/tmp/mixed.txt" );
      Points pp = core::readVectorFromMatlabAsColumn<Points>( "c:/tmp/w.txt" );

      std::cout << "pp size=" << pp.size() << std::endl;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         for ( ui32 nn = 0; nn < nbImages; ++nn )
         {
            images[ nn ][ n ] = (ui8)NLL_BOUND( ( pp[n][ nn ] * 10 ) + 128, 0, 255 );
         }
      }

      for ( ui32 nn = 0; nn < nbImages; ++nn )
      {
         core::writeBmp( images[ nn ], "c:/tmp/matlab-img" + core::val2str( nn ) + ".bmp" );
      }*/
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIndependentComponentAnalysis);
TESTER_TEST(testBasic);
TESTER_TEST(testRandomSVD);
TESTER_TEST(testImage);
TESTER_TEST_SUITE_END();
#endif