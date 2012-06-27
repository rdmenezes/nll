
#include <nll/nll.h>
#include <tester/register.h>
#include <functional>
#include <complex>

using namespace nll;


struct TestVisualQuasiPeriodicityAnalysis
{
   void testBasic1DRealFFT()
   {
      //
      // compare against known run:
      // http://people.sc.fsu.edu/~jburkardt/c_src/fftw3/fftw3_prb.c
      //
      const double i[]=
      {
         0.218418,
         0.956318,
         0.829509,
         0.561695,
         0.415307,
         0.066119,
         0.257578,
         0.109957,
         0.043829,
         0.633966,
         0.061727,
         0.449539,
         0.401306,
         0.754673,
         0.797287,
         0.001838,
         0.897504,
         0.350752,
         0.094545,
         0.013617,
         0.859097,
         0.840847,
         0.123104,
         0.007512,
         0.260303,
         0.912484,
         0.113664,
         0.351629,
         0.822887,
         0.267132,
         0.692066,
         0.561662,
         0.861216,
         0.453794,
         0.911977,
         0.597917,
         0.188955,
         0.761492,
         0.396988,
         0.185314,
         0.574366,
         0.367027,
         0.617205,
         0.361529,
         0.212930,
         0.714471,
         0.117707,
         0.299329,
         0.825003,
         0.824660,
         0.061862,
         0.710781,
         0.088283,
         0.777994,
         0.745303,
         0.308675,
         0.899373,
         0.763537,
         0.761731,
         0.406970,
         0.938749,
         0.562088,
         0.017820,
         0.501103,
         0.041909,
         0.368851,
         0.271724,
         0.858573,
         0.029037,
         0.017442,
         0.152384,
         0.114319,
         0.353907,
         0.119308,
         0.206653,
         0.212924,
         0.612948,
         0.809519,
         0.587090,
         0.215492,
         0.768056,
         0.723297,
         0.448019,
         0.855176,
         0.945017,
         0.909057,
         0.519726,
         0.030195,
         0.481067,
         0.292313,
         0.902640,
         0.667842,
         0.412278,
         0.156948,
         0.833282,
         0.964404,
         0.740790,
         0.456099,
         0.653561,
         0.406827
      };

      const ui32 sizei = core::getStaticBufferSize( i );

      const double ii[]=
      {
         47.682692,	0,
         0.460101,	0.361409,
         2.261972,	4.348594,
         -1.747824,	4.029391,
         -0.328176,	-4.200315,
         2.816996,	3.477997,
         2.557872,	3.006633,
         1.742836,	0.3078,
         -0.966474,	0.06404,
         1.352084,	-1.948328,
         -0.604495,	-1.275929,
         -2.126921,	-3.871755,
         1.475699,	-0.97569,
         -0.974107,	0.415945,
         -0.67232,	1.167949,
         -0.293818,	-1.886153,
         -3.219591,	-2.653382,
         -1.745777,	-5.347932,
         0.358931,	0.291055,
         1.032895,	-0.519676,
         1.506143,	-1.667676,
         2.077434,	-1.027298,
         -2.191087,	3.317592,
         -1.566222,	2.188927,
         1.060558,	1.045526,
         1.717386,	-4.10537,
         -3.321912,	-0.124217,
         -3.099074,	-0.317698,
         -3.469523,	4.365083,
         -0.191268,	-0.989901,
         0.809721,	-3.732703,
         -0.993215,	1.997418,
         -0.079303,	-2.029524,
         0.861762,	-0.675954,
         -3.27122,	-1.417248,
         2.427421,	1.727539,
         0.351879,	-1.965642,
         -0.294874,	1.439789,
         -1.915906,	-3.679643,
         -2.614548,	-0.444974,
         0.367775,	-1.427269,
         -1.312616,	0.25057,
         -3.503356,	1.765738,
         4.10661,	3.996473,
         -1.461233,	0.664648,
         0.772053,	-3.957355,
         -1.298635,	0.463043,
         -0.35424,	-1.297522,
         -1.508003,	-1.782938,
         1.860838,	0.527204,
         0.452682,	0
      };

      const ui32 sizeii = core::getStaticBufferSize( ii );


      core::Buffer1D<double> input( (double*)i, sizei, false );
      core::Buffer1D<double> output;

      algorithm::Fft1D fft;
      fft.forward( input, input.size(), output );

      TESTER_ASSERT( sizeii == output.size() );
      for ( ui32 n = 0; n < sizeii; ++n )
      {
         TESTER_ASSERT( core::equal<double>( output[ n ], ii[ n ], 1e-5 ) );
      }

      core::Buffer1D<double> input2( (double*)ii, sizeii, false );
      fft.backward( input2, input2.size(), output );
      TESTER_ASSERT( sizei == output.size() );
      for ( ui32 n = 0; n < sizei; ++n )
      {
         TESTER_ASSERT( core::equal<double>( output[ n ] / input.size(), i[ n ], 1e-5 ) );
      }
   }

   void testConvolution1d_a()
   {
      const core::Buffer1D<double> data = core::make_buffer1D<double>( 1, 8, 3, -2, 6 );
      const core::Buffer1D<double> kernel = core::make_buffer1D<double>( 0.1, 0.5, 0.4 );

      const core::Buffer1D<double> convolution = core::convolve( data, kernel );
      convolution.print( std::cout );

      const ui32 half = kernel.size() / 2;
      for ( ui32 n = half; n < data.size() - half; ++n )
      {
         const double val = data[ n - 1 ] * kernel[ 0 ] +
                            data[ n     ] * kernel[ 1 ] +
                            data[ n + 1 ] * kernel[ 2 ];
         TESTER_ASSERT( core::equal<double>(val, convolution[ n ], 1e-6) );
      }

      {
         const double val = ( data[ 0 ] * kernel[ 1 ] + data[ 1 ] * kernel[ 2 ] ) / ( kernel[ 1 ] + kernel[ 2 ] );
         TESTER_ASSERT( core::equal<double>( convolution[ 0 ], val ) );
      }

      {
         const double val = ( data[ 4 ] * kernel[ 1 ] + data[ 3 ] * kernel[ 0 ] ) / ( kernel[ 1 ] + kernel[ 0 ] );
         TESTER_ASSERT( core::equal<double>( convolution[ 4 ], val ) );
      }
   }

   void testConvolution1d_b()
   {
      const core::Buffer1D<double> data = core::make_buffer1D<double>( 1, 8, 3, -2, 6, 2.5 );
      const core::Buffer1D<double> kernel = core::make_buffer1D<double>( 0.1, 0.2, 0.4, 0.13, 0.17 );

      const core::Buffer1D<double> convolution = core::convolve( data, kernel );
      convolution.print( std::cout );

      const ui32 half = kernel.size() / 2;
      for ( ui32 n = half; n < data.size() - half; ++n )
      {
         const double val = data[ n - 2 ] * kernel[ 0 ] +
                            data[ n - 1 ] * kernel[ 1 ] +
                            data[ n + 0 ] * kernel[ 2 ] +
                            data[ n + 1 ] * kernel[ 3 ] +
                            data[ n + 2 ] * kernel[ 4 ];

         TESTER_ASSERT( core::equal<double>(val, convolution[ n ], 1e-6) );
      }

      {
         const double up = ( data[ 0 ] * kernel[ 2 ] + data[ 1 ] * kernel[ 3 ] + data[ 2 ] * kernel[ 4 ] );
         const double down = ( kernel[ 2 ] + kernel[ 3 ] + kernel[ 4 ] );
         const double val = up / down;
         TESTER_ASSERT( core::equal<double>( convolution[ 0 ], val, 1e-6 ) );
      }

      {
         const double up = ( data[ 0 ] * kernel[ 1 ] + data[ 1 ] * kernel[ 2 ] + data[ 2 ] * kernel[ 3 ] + data[ 3 ] * kernel[ 4 ] );
         const double down = ( kernel[ 1 ] + kernel[ 2 ] + kernel[ 3 ] + kernel[ 4 ] );
         const double val = up / down;
         TESTER_ASSERT( core::equal<double>( convolution[ 1 ], val, 1e-6 ) );
      }

      {
         const double up = ( data[ 5 ] * kernel[ 2 ] + data[ 4 ] * kernel[ 1 ] + data[ 3 ] * kernel[ 0 ] );
         const double down = ( kernel[ 2 ] + kernel[ 1 ] + kernel[ 0 ] );
         const double val = up / down;
         TESTER_ASSERT( core::equal<double>( convolution[ 5 ], val, 1e-6 ) );
      }

      {
         const double up = ( data[ 5 ] * kernel[ 3 ] + data[ 4 ] * kernel[ 2 ] + data[ 3 ] * kernel[ 1 ] + data[ 2 ] * kernel[ 0 ] );
         const double down = ( kernel[ 3 ] + kernel[ 2 ] + kernel[ 1 ] + kernel[ 0 ] );
         const double val = up / down;
         TESTER_ASSERT( core::equal<double>( convolution[ 4 ], val, 1e-6 ) );
      }
   }

   void testPeriodogram()
   {
      double i[] =
      {
          0.2779,
         -1.3602,
         -0.6691,
         -0.2785,
          2.6159,
          0.6840,
          1.7780,
         -0.1281,
         -0.3308,
          0.0706,
         -1.0704,
          0.6990,
         -0.7002,
         -1.9162,
          0.3282
      };

      core::Buffer1D<double> smoothingKernel;   // empty
      core::Buffer1D<double> series( i, core::getStaticBufferSize( i ), false );
      algorithm::Periodogram periodogram;
      algorithm::HanningWindow windowFunc;

      core::Buffer1D<double> vals = periodogram.compute( series, smoothingKernel, windowFunc );
      vals.print( std::cout );
   }

   void testPeriodogramPeriodicFunction1()
   {
      srand(0);
      for ( ui32 iter = 0; iter < 100; ++iter )
      {
         const double period =  core::generateUniformDistribution( 0.1, 0.5 );
         const double mean = core::generateUniformDistribution( -0.5, 2.5 );
         const ui32 nbData = core::generateUniformDistributioni( 100, 150 );

         core::Buffer1D<double> data( nbData );
         for ( ui32 n = 0; n < nbData; ++n )
         {
            data[ n ] = std::cos( 2 * core::PI * n * period ) + mean;
         }

         algorithm::Periodogram periodogram;
         algorithm::SquareWindow window;
         const core::Buffer1D<double> p = periodogram.compute( data, core::Buffer1D<double>(), window );

         size_t maxIndex = std::max_element( p.begin(), p.end() ) - p.begin();
         const double periodFound = maxIndex / ( 2.0 * p.size() );
         TESTER_ASSERT( fabs( periodFound - period ) < 0.05 );
      }
   }

   // test the periodogram against a known periodic function
   void testPeriodogramPeriodicFunction2()
   {
      srand(0);
      for ( ui32 iter = 0; iter < 100; ++iter )
      {
         const double period =  core::generateUniformDistribution( 0.1, 0.3 );
         const double period2 =  core::generateUniformDistribution( 0.35, 0.5 );
         const double mean = core::generateUniformDistribution( -0.5, 2.5 );
         const ui32 nbData = core::generateUniformDistributioni( 100, 150 );

         core::Buffer1D<double> data( nbData );
         for ( ui32 n = 0; n < nbData; ++n )
         {
            const double noise = core::generateGaussianDistribution(0, 0.5 );
            data[ n ] = std::cos( 2 * core::PI * n * period ) + mean + std::cos( 2 * core::PI * n * period2 ) + noise;
         }

         algorithm::Periodogram periodogram;
         algorithm::HanningWindow window;
         core::Buffer1D<double> p = periodogram.compute( data, core::make_buffer1D<double>( 0.1, 0.8, 0.1 ), window );

         size_t maxIndex = std::max_element( p.begin(), p.end() ) - p.begin();
         if ( maxIndex >= 1 )
            p[ (ui32)maxIndex - 1 ] = 0;
         p[ (ui32)maxIndex ] = 0;
         if ( maxIndex + 1 < p.size() )
            p[ (ui32)maxIndex + 1 ] = 0;
         size_t maxIndex2 = std::max_element( p.begin(), p.end() ) - p.begin();

         double periodFound = maxIndex / ( 2.0 * p.size() );
         double periodFound2 = maxIndex2 / ( 2.0 * p.size() );
         if ( periodFound2 < periodFound )
         {
            std::swap( periodFound, periodFound2 );
         }

         std::cout << "TRUTH p1=" << period << " p2=" << period2 << std::endl;
         std::cout << "maxIndex=" << maxIndex << " period=" << periodFound << std::endl;
         std::cout << "maxIndex2=" << maxIndex2 << " period2=" << periodFound2 << std::endl;
         TESTER_ASSERT( fabs( periodFound - period ) < 0.05 );
         TESTER_ASSERT( fabs( periodFound2 - period2 ) < 0.05 );
      }
   }

   std::vector<core::Image<float>> createFrames( ui32 nbFrames = 30, ui32 nbPeriods = 3 )
   {
      // create a noisy rectangle increasing/decreasing
      const ui32 sx = 48 / 2;
      const ui32 sy = 32 / 2;
      const float ssx = 40 / 2;
      const float ssy = 26 / 2;
      const float minRatio = 0.6f;
      const float noiseLevel = 30;
      const float shapeVal = 200;


      const ui32 imageCenterx = sx / 2;
      const ui32 imageCentery = sy / 2;
      const ui32 nbFramesPerPeriod = nbFrames / nbPeriods;

      // generate the sequence
      std::vector<core::Image<float>> frames;
      for ( ui32 n = 0; n < nbFrames; ++n )
      {
         const ui32 cycleFrame = n % nbFramesPerPeriod;
         const float ratioFrame = fabs( cycleFrame - nbFramesPerPeriod / 2.0 ) / ( nbFramesPerPeriod / 2 );
         const float rationShape = ( 1 - minRatio ) * ratioFrame + minRatio;

         const ui32 sizeShapex = ssx * rationShape;
         const ui32 sizeShapey = ssy * rationShape;

         core::Image<float> frame( sx, sy, 1 );
         for ( ui32 y = imageCentery - sizeShapey / 2; y < imageCentery + sizeShapey / 2;  ++y )
         {
            for ( ui32 x = imageCenterx - sizeShapex / 2; x < imageCenterx + sizeShapex / 2;  ++x )
            {
               const float val = shapeVal + core::generateUniformDistribution( -noiseLevel, noiseLevel );
               frame.point( x, y )[ 0 ] = val / 255;
            }
         }
         frames.push_back( frame );
         
         core::Image<ui8> save;
         save.import( frame );
         core::extend( save, 3 );
         core::writeBmp( save, "c:/tmp/save-" + core::val2str( n ) + ".bmp" );
         
      }

      return frames;
   }

   void testPeriodicityAnalysis()
   {
      typedef std::vector<core::Image<float>>   Frames;
      typedef algorithm::QuasiPeriodicityAnalysis<Frames> Algorithm;
      for ( ui32 n = 0; n < 100; ++n )
      {
         srand(n);
         //srand(29);
         std::cout << "iter=" << n <<std::endl;
         // setup
         const ui32 nbFrames = core::generateUniformDistributioni( 50, 150 );
         const ui32 nbPeriods = core::generateUniformDistributioni( 3, 10 );
         const std::vector<core::Image<float>> frames = createFrames( nbFrames, nbPeriods );
         const double expectedFrequency = 1 / ( static_cast<double>( nbFrames ) / nbPeriods );

         // do the analysis
         Algorithm periodicity;
         Algorithm::Result result = periodicity.analyse( frames, 0.01 );
         std::cout << "fundamentalFrequency=" << result.fundamentalFrequency << " expected=" << expectedFrequency << std::endl;
         std::cout << "periodicity=" << result.periodicityStrength << std::endl;

         TESTER_ASSERT( fabs( expectedFrequency - result.fundamentalFrequency ) < 0.02 );
         TESTER_ASSERT( result.periodicityStrength > 0.8 );

         //break;
                     /*
            // --- DEBUG
            ((core::Buffer1D<double>)pca.getProjection()).print( std::cout );
            for ( ui32 n = 0; n < nbBasis; ++n )
            {
               const core::Matrix<double>& eiv = pca.getProjection();
               core::Image<ui8> img( sx, sy, 1 );
               for ( ui32 y = 0; y < sy; ++y )
               {
                  for ( ui32 x = 0; x < sx; ++x )
                  {
                     img( x, y, 0 ) = static_cast<ui8>( NLL_BOUND( eiv( n, x + y * sx ) * 127 + 127, 0, 255 ) );
                  }
               }
               core::extend( img, 3 );
               core::writeBmp( img, "c:/tmp/eiv-" + core::val2str( n ) + ".bmp" );
            }
            // --- DEBUG
            */
      }
   }

   void testPcaSparse()
   {
      //
      // here we have more example than dimensions, so it can give different results than PCA
      //
      typedef algorithm::PcaSparseSimple<std::vector<std::vector<double>>> Pca;
      Pca pca;

      std::vector<std::vector<double>> data;
      data.push_back( core::make_vector<double>( 1, 2 ) );
      data.push_back( core::make_vector<double>( 2, 4 ) );
      data.push_back( core::make_vector<double>( 3, 6 ) );
      data.push_back( core::make_vector<double>( -1, 2 ) );

      const bool succeeded = pca.computeByVarianceRetained( data, 1 );
      ensure( succeeded, "error" );
      Pca::Matrix proj = pca.getProjection();

      proj.print( std::cout );

      TESTER_ASSERT( proj.sizex() == 2 );
      TESTER_ASSERT( proj.sizey() == 2 );
      TESTER_ASSERT( core::equal<double>( proj( 0, 0 ), 0.659087, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( proj( 0, 1 ), 0.752067, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( proj( 1, 0 ), 0.752067, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( proj( 1, 1 ), -0.659087, 1e-4 ) );
   }

   void testPcaSparse2()
   {
      //
      // here we have more example than dimensions, so it can give different results than PCA
      //
      typedef algorithm::PcaSparseSimple<std::vector<std::vector<double>>> Pca;
      Pca pca;

      std::vector<std::vector<double>> data;
      data.push_back( core::make_vector<double>( 1, 2 ) );
      data.push_back( core::make_vector<double>( 2, 4 ) );
      data.push_back( core::make_vector<double>( 3, 6 ) );

      const bool succeeded = pca.computeByVarianceRetained( data, 1 );
      ensure( succeeded, "error" );
      Pca::Matrix proj = pca.getProjection();

      proj.print( std::cout );

      TESTER_ASSERT( proj.sizex() == 2 );
      TESTER_ASSERT( proj.sizey() == 1 );
      TESTER_ASSERT( core::equal<double>( proj( 0, 0 ), 0.447214, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( proj( 0, 1 ), 0.894427, 1e-4 ) );
      
      std::vector<double> p = pca.process( core::make_vector<double>( 4, 8 ) );
      TESTER_ASSERT( p.size() == 1 );
      TESTER_ASSERT( core::equal<double>( p[ 0 ], 4.47, 1e-2 ) );

      std::vector<double> pinv = pca.reconstruct( p );
      TESTER_ASSERT( pinv.size() == 2 );
      TESTER_ASSERT( core::equal<double>( pinv[ 0 ], 4, 1e-2 ) );
      TESTER_ASSERT( core::equal<double>( pinv[ 1 ], 8, 1e-2 ) );
   }

   void testPcaSparseReal()
   {
      //
      // Check against PCA: we must obtain exaclty the same results
      //
      typedef algorithm::PcaSparseSimple<std::vector<core::Image<float>>> Pca;
      typedef algorithm::PrincipalComponentAnalysis<std::vector<core::Image<float>>> Pca2;

      std::vector<core::Image<float>> frames = createFrames();

      Pca pca;
      pca.computeByVarianceRetained( frames, 0.9 );

      Pca2 pca2;
      pca2.computeByVarianceRetained( frames, 0.9 );

      Pca::Matrix m = pca.getProjection() - pca2.getProjection();
      m.print( std::cout );
      for ( ui32 n = 0; n < m.size(); ++n )
      {
         ensure( fabs( m[ n ] ) < 1e-6, "It must be equal" );
      }
   }

   void testIsMultipleOf()
   {
      TESTER_ASSERT( core::isMultipleOf<double>( 2, 4.1, 0.2 ) );
      TESTER_ASSERT( core::isMultipleOf<double>( 2, 3.9, 0.2 ) );
      TESTER_ASSERT( core::isMultipleOf<double>( -2, 4.1, 0.2 ) );
      TESTER_ASSERT( core::isMultipleOf<double>( 2, -3.9, 0.2 ) );


      TESTER_ASSERT( core::isMultipleOf<double>( 0.001, 0.0049, 0.0002 ) );
      TESTER_ASSERT( !core::isMultipleOf<double>( 0.001, 0.00521, 0.0002 ) );
      TESTER_ASSERT( !core::isMultipleOf<double>( 0.001, 0.00579, 0.0002 ) );
      TESTER_ASSERT( core::isMultipleOf<double>( 0.001, 0.00582, 0.0002 ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVisualQuasiPeriodicityAnalysis);
 TESTER_TEST(testBasic1DRealFFT);
 TESTER_TEST(testConvolution1d_a);
 TESTER_TEST(testConvolution1d_b);
 TESTER_TEST(testPeriodogram);

 TESTER_TEST(testPeriodogramPeriodicFunction2);
 TESTER_TEST(testPcaSparse);
 TESTER_TEST(testPcaSparse2);

 TESTER_TEST(testPcaSparseReal);
 TESTER_TEST(testPeriodogramPeriodicFunction1);
 
 TESTER_TEST(testPeriodicityAnalysis);
 TESTER_TEST(testIsMultipleOf);
TESTER_TEST_SUITE_END();
#endif
