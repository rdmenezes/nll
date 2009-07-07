#include "stdafx.h"
#include <nll/nll.h>

using namespace nll::algorithm;

class TestHmmContinuous
{
public:
   // in this test we already know what hmm generated the samples. Just compare we have the same results
   // Also test the sequnce generation of the HMM
   void testHmm1()
   {
      //1245617666
      unsigned seed = 1245794065;//time(0);
      //std::cout << "seed=" << seed << std::endl;
      srand( seed ); // set the seed since we need to know the exact paramters found by the algorithm

      typedef std::vector<double>                      Observation;
      typedef HiddenMarkovModelContinuous<Observation> Hmm;
      typedef nll::core::Matrix<double>                Matrix;
      typedef nll::core::Buffer1D<double>              Vector;
      typedef std::vector<Observation>                 Observations;

      // transition matrix
      const unsigned nbStates = 4;
      Matrix transition( nbStates, nbStates );
      transition( 0, 1 ) = 0.6;
      transition( 0, 2 ) = 0.4;
      transition( 1, 0 ) = 1;
      transition( 2, 3 ) = 1;
      transition( 3, 2 ) = 0.5;
      transition( 3, 0 ) = 0.5;

      // initial state probability
      Vector pi( 4 );
      pi = nll::core::make_buffer1D<double>( 0.25, 0.25, 0, 0.5 );

      // define the gaussian distribution (only 1 gaussian is used to generate the distribution)
      struct Gaussian
      {
         double meanx;
         double meany;
         double variance;
      };

      const Gaussian gaussians[] =
      {
         {0,   0,   0.25},
         {0.5, 2,   0.25},
         {2,   0.5, 0.25},
         {1, 0.25,   0.25}
      };

      // generate observations by state
      for ( unsigned nb = 0; nb < 100; ++nb )
      {
         std::vector<Observations> observations( nbStates );
         for ( unsigned n = 0; n < nbStates; ++n )
         {
            for ( unsigned nn = 0; nn < 35; ++nn )
            {
               double posx = nll::core::generateGaussianDistribution( gaussians[ n ].meanx, gaussians[ n ].variance );
               double posy = nll::core::generateGaussianDistribution( gaussians[ n ].meany, gaussians[ n ].variance );
               Observation p = nll::core::make_vector<double>( posx, posy );

               observations[ n ].push_back( p );
            }
         }

         // generate a serie of observations
         const unsigned size = 15;
         const unsigned nbChains = 2000;
         std::vector<Observations> dataset( nbChains );
         std::vector< std::vector<unsigned> > statesList( nbChains );
         for ( unsigned n = 0; n < nbChains; ++n )
         {
            std::vector<unsigned> chain( size );
            chain[ 0 ] = nll::core::sampling( pi, 1 )[ 0 ];
            for ( unsigned nn = 1; nn < size; ++nn )
            {
               std::vector<double> proba( nbStates );
               for ( unsigned nnn = 0; nnn < nbStates; ++nnn )
                  proba[ nnn ] = transition( chain[ nn - 1 ], nnn );
               chain[ nn ] = nll::core::sampling( proba, 1 )[ 0 ];
            }

            Observations obs( size );
            for ( unsigned nn = 0; nn < size; ++nn )
            {
               const unsigned observationIndex = (unsigned)rand() % (unsigned)observations[ chain[ nn ] ].size();
               obs[ nn ] = observations[ chain[ nn ] ][ observationIndex ];
            }
            dataset[ n ] = obs;
            statesList[ n ] = chain;
         }

         // generate the hmm
         Hmm hmm;
         hmm.learn( dataset,
                    statesList,
                    nll::core::make_vector<unsigned>( 1, 1, 1, 1 ),
                    nll::core::make_vector<unsigned>( 5, 5, 5, 5 ) );

         // generate the markov chain
         nll::algorithm::MarkovChainFirstOrderDense markovChain( transition, pi );

         const unsigned sizeSeq = 15;
         nll::core::Buffer1D<nll::ui32> chain = markovChain.generateSequence( sizeSeq );

         std::vector<nll::ui32> chainConv( sizeSeq );
         Observations obs( sizeSeq );
         for ( unsigned n = 0; n < sizeSeq; ++n )
         {
            chainConv[ n ] = chain[ n ];
            const unsigned observationIndex = (unsigned)rand() % (unsigned)observations[ chain[ n ] ].size();
            obs[ n ] = observations[ chain[ n ] ][ observationIndex ];
         }

         // compare
         std::vector<nll::ui32> chainOut;
         hmm.computeHiddenState( obs, chainOut );
         TESTER_ASSERT( chainOut == chainConv );

         hmm.probability( obs );

         // test the sequence generation
         const unsigned nbChainsGeneration = 1000;
         const unsigned chainSize = 6;
         
         std::vector<Observations> lists( nbChainsGeneration );
         std::vector< std::vector<nll::ui32> > statesList2( nbChainsGeneration );
         for ( unsigned n = 0; n < nbChainsGeneration; ++n )
         {
            lists[ n ] = hmm.generateSequence( chainSize, &statesList2[ n ] );
         }

         // recompute a hmm and test it against the model one
         Hmm hmm2;
         hmm2.learn( lists, 
                     statesList2,
                     nll::core::make_vector<unsigned>( 1, 1, 1, 1 ),
                     nll::core::make_vector<unsigned>( 5, 5, 5, 5 ) );

         const double precisionGeneration = 0.05;
         TESTER_ASSERT(  hmm2.getTransitions().equal( hmm.getTransitions(), precisionGeneration ) );
         for ( unsigned n = 0; n < hmm2.getPi().size(); ++n )
            TESTER_ASSERT( nll::core::equal( hmm2.getPi()[ n ], hmm.getPi()[ n ], precisionGeneration ) );
          std::cout << "#";
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestHmmContinuous);
TESTER_TEST(testHmm1);
TESTER_TEST_SUITE_END();
#endif
