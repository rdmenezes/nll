#include <nll/nll.h>
#include <tester/register.h>
#include <functional>
#include <complex>

using namespace nll;

namespace nll
{
   namespace core
   {
      /**
       @brief Compute a 1D convolution

       Note that the behaviour on the data sides (i.e., +/- convolution.size() / 2) is to do an average of the
       kernel that fits inside the data.

       Complexity in time is o(NM) in processing and o(M+N) in size, with N = data size and M = convolution kernel size

       We are assuming the data has at least the size of the kernel.
       */
      template <class T>
      core::Buffer1D<T> convolve( const core::Buffer1D<T>& data, const core::Buffer1D<T>& convolution )
      {
         #ifdef NLL_SECURE
         {
            const T accum = std::accumulate( convolution.begin(), convolution.end(), static_cast<T>( 0 ) );
            ensure( core::equal<T>( accum, 1, 1e-5 ), "the sum of convolution coef must sum to 1" );
         }
         #endif

         ensure( convolution.size() % 2 == 1, "convolution size must be odd" );
         ensure( convolution.size() > data.size(), "data size must be > kernel size!" ); // if this is not true, data will not be useful anyway

         core::Buffer1D<T> convolved( data.size(), false );

         // compute the "regular" sequence, i.e. in domain [kernelSize/2..dataSize-kernelSize/2]
         const ui32 halfKernelSize = convolution.size() / 2;
         const ui32 lastRegularIndex = data.size() - halfKernelSize;
         for ( ui32 n = halfKernelSize; n < lastRegularIndex; ++n )
         {
            T accum = 0;
            for ( ui32 nn = 0; nn < convolution.size(); ++nn )
            {
               accum += data[ n - halfKernelSize ] * convolution[ nn ];
            }

            convolved[ n ] = accum;
         }

         // compute the normalization factor for part of the kernel fitting in.
         core::Buffer1D<double> maxRegularizationLeft( halfKernelSize, false );
         core::Buffer1D<double> maxRegularizationRight( halfKernelSize, false );

         // assuming half the kernel always fit in the data
         {
            // accum holds the total kernel weight that fits in
            T accum = std::accumulate( convolution.begin() + halfKernelSize + 1, convolution.end(), (T)0.0 ); // we have at least half the filter
            for ( ui32 n = 0; n < halfKernelSize; ++n )
            {
               accum += convolution[ halfKernelSize - n ]; 
               maxRegularizationLeft[ n ] = 1.0 / accum;
            }
         }

         {
            // accum holds the total kernel weight that fits in
            T accum = std::accumulate( convolution.begin(), convolution.begin() + halfKernelSize - 1, (T)0.0 ); // we have at least half the filter
            for ( ui32 n = 0; n < halfKernelSize; ++n )
            {
               accum += convolution[ halfKernelSize + n ]; 
               maxRegularizationRight[ n ] = 1.0 / accum;
            }
         }

         // now take care of the sides, they will be more noisy as we are only using part of the kernel...
         for ( ui32 n = 0; n < halfKernelSize; ++n )
         {
            // left side
            {
               T accum = 0;
               for ( ui32 nn = halfKernelSize - n; nn < convolution.size(); ++nn )
               {
                  accum += convolution[ nn ] * data[ n ];
               }
               convolved[ n ] = accum * maxRegularizationLeft[ n ]; // here we normalize by the weighted kernel fitting in
            }

            // right side
            {
               const ui32 indexRight = data.size() - n - 1; // corresponding index in the data
               T accum = 0;
               for ( int nn = halfKernelSize + n; nn >= 0; --nn )
               {
                  accum += convolution[ nn ] * data[ indexRight ];
               }
               convolved[ indexRight ] = accum * maxRegularizationLeft[ n ]; // here we normalize by the weighted kernel fitting in
            }
         }
         return convolved;
      }
   }

   namespace algorithm
   {
      /**
       @brief Hanning window

       Useful for DFT/FFT purposes, such as in spectral analysis. The DFT/FFT contains an implicit periodic asumption. 
       The non periodic signal will become periodic when applied the window. 
       */
      class HanningWindow : public std::function<double(double)>
      {
      public:
         typedef double value_type;

         value_type operator()( value_type v ) const
         {
            #ifdef NLL_SECURE
            ensure( v >= 0 && v <= 1.0, "wrong domain!" );
            #endif

            return 0.5 * ( 1.0 - std::cos( 2 * core::PI * v ) );
         }
      };

      /**
       @brief Create the periodogram of a 1D time serie
       @see http://www.stat.tamu.edu/~jnewton/stat626/topics/topics/topic4.pdf
            http://www.ltrr.arizona.edu/~dmeko/notes_6.pdf

       f(w) = 1/N * |sum_t=1-N(x(t)*exp(2*pi*i*(t-1)*w))|^2, w in [0..0.5]
       f(w) = f( 1 - w )

       Raw spectrum of a time serie suffers from the spectral bias and (the variance at a given frequency does not decrease as
       the number of samples used in the computation increases) and variance problems.

       The spectral bias problem arises from a sharp truncation of the sequence, and can be reduced by first multiplying the
       finite sequence by a window function which truncates the sequence gradually rather than abruptly.

       The variance problem is reduced but smoothing the periodogram

       Smoothness, stability and resolution of the data must be considered when choosing the smoothing kernel and
       window function. This is problem dependent.
       */
      class Periodogram
      {
      public:
         typedef double                                  value_type;
         typedef std::function<value_type(value_type)>   Function1D;

         /**
          @brief Compute the raw and smoothed periodogram
          @param timeSerie the time serie we are analysing
          @param smoothingKernel the kernel used to smooth the periodogram. Can be empty if no smoothing required
          @param funcWindow the window to use to reduce the sharp truncation effect
          */
         core::Buffer1D<value_type> compute( const core::Buffer1D<value_type>& timeSerie, const core::Buffer1D<value_type>& smoothingKernel, const Function1D& funcWindow )
         {
            // get the mean
            const value_type mean = std::accumulate( timeSerie.begin(), timeSerie.end(), (value_type)0.0 ) / timeSerie.size();

            // multiply by a window to reduce the spectral bias (see http://en.wikipedia.org/wiki/Periodogram)
            core::Buffer1D<value_type> data( timeSerie.size(), false );
            for ( ui32 n = 0; n < data.size(); ++n )
            {
               data[ n ] = ( timeSerie[ n ] - mean ) * funcWindow( static_cast<value_type>( n ) / timeSerie.size() );
            }

            // get the DFT coefficients
            Fft1D fft;
            core::Buffer1D<value_type> fftOutput;
            fft.forward( data, data.size(), fftOutput );
            ensure( fftOutput.size() % 2 == 0, "must be pair! we have real and imaginary parts" );
            ensure( fftOutput.size() / 2 == data.size() / 2, "WRONG!!!" );

            // compute the raw periodogram from the DFT coefficients
            core::Buffer1D<value_type> basicPeriodogram( data.size() / 2 );
            for ( ui32 n = 0; n < basicPeriodogram.size(); ++n )
            {
               ui32 index = n * 2;
               basicPeriodogram[ n ] = ( core::sqr( fftOutput[ index ] ) + core::sqr( fftOutput[ index + 1 ] ) ) / timeSerie.size();
            }

            // smooth the DFT coefficients, we are done!
            if ( smoothingKernel.size() )
            {
               return core::convolve( basicPeriodogram, smoothingKernel );
            } else {
               return basicPeriodogram;
            }
         }
      };

      /**
       @brief Determine the period of a quasi periodic time serie
       @note Direct implementation of "Visual Quasi-Periodicity", 2008 E. Pogalin, A.W.M. Smeulders A.H.C. Thean
       */
      class VisualQuasiPeriodicityAnalysis
      {
      public:
         typedef core::Image<float>    Image;
         typedef std::vector<Image>    Images;
         typedef core::Matrix<double, core::IndexMapperRowMajorFlat2D>   Matrix;

         /**
          @brief Analyze a set of frames to find its degree of periodicity and its period
          @param frames must be greyscale frames. The frames must be cropped and centred on the region of interest
          */
         void analyse( const Images& frames, double frameDuration, double varianceToRetain = 0.75 )
         {
            // sanity checks
            ensure( frameDuration > 0, "incorrect frame time" );
            if ( frames.size() == 0 )
               return;
            const ui32 sx = frames[ 0 ].sizex();
            const ui32 sy = frames[ 0 ].sizey();
            const ui32 nbFrames = static_cast<ui32>( frames.size() );
            for ( ui32 n = 0; n < nbFrames; ++n )
            {
               ensure( sx == frames[ n ].sizex(), "wrong size" );
               ensure( sy == frames[ n ].sizey(), "wrong size" );
               ensure( frames[ n ].getNbComponents() == 1, "only works on greyscale image" );
            }

            // first do a PCA decomposition. The eigen vectors are representing the main "step" of the period
            algorithm::PrincipalComponentAnalysis<Images> pca;
            const bool success = pca.computeByVarianceRetained( frames, varianceToRetain );
            ensure( success, "PCA decomposition failed!" );
            
            // simply project each frame on the retained orthogonal basis, this will give us the reponse of a frame for each basis over time
            const ui32 nbBasis = pca.getNbVectors();
            Matrix response( nbBasis, nbFrames );
            for ( ui32 n = 0; n < frames.size(); ++n )
            {
               core::Buffer1D<float> projection = pca.process( core::Buffer1D<float>( frames[ n ] ) );
               ensure( projection.size() == response.sizey(), "wrong size" );
            }

            // now do spectrum analysis
         }
      };
   }
}

struct TestVisualQuasiPeriodicityAnalysis
{
   void test()
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

   }

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
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVisualQuasiPeriodicityAnalysis);
 TESTER_TEST(test);
 TESTER_TEST(testBasic1DRealFFT);
TESTER_TEST_SUITE_END();
#endif
