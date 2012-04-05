#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
   namespace algorithm
   {
      /**
       @brief The period of a quasi periodic 2D time series
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

            // now do FFT analysis
            //http://people.sc.fsu.edu/~jburkardt/c_src/fftw3/fftw3_prb.c
         }
      };
   }
}

struct TestVisualQuasiPeriodicityAnalysis
{
   void test()
   {
      
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
         TESTER_ASSERT( core::equal<double>( output[ n ], i[ n ], 1e-5 ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVisualQuasiPeriodicityAnalysis);
 TESTER_TEST(test);
 TESTER_TEST(testBasic1DRealFFT);
TESTER_TEST_SUITE_END();
#endif
