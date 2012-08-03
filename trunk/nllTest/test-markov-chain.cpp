#include <nll/nll.h>
#include <tester/register.h>

class TestMarkovChain
{
public:
   /**
    Generate a sample with a known initial state and transition matrix. Run a markov chain on these
    generated samples, verify we find the expected transition and initial distribution probabilities
    */
   void testMarkovChainFirstOrder()
   {
      srand( 0 );
      typedef nll::core::Matrix<double>                Matrix;
      typedef nll::core::Buffer1D<double>              Vector;

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
      double tolerance = 5e-2;

      for ( unsigned nb = 0; nb < 10; ++nb )
      {
         // generate a serie of observations
         const unsigned size = 15;
         const unsigned nbChains = 5000;
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
            statesList[ n ] = chain;
         }

         // run the tests
         nll::algorithm::MarkovChainFirstOrderDense markovChain;
         markovChain.learn( statesList );

         for ( unsigned n = 0; n < 4; ++n )
         {
            TESTER_ASSERT( nll::core::equal( markovChain.getInitialStateDistribution()[ n ], pi[ n ], tolerance ) );
            for ( unsigned nn = 0; nn < 4; ++nn )
               TESTER_ASSERT( nll::core::equal( markovChain.getTransitions()( n, nn ), transition( n, nn ), tolerance ) );
         }
         std::cout << "#";
      }
   }

   /**
    Given a transition and initial state probabilities. Generate samples using the markov chain and then
    learn another markov chain using the genrated samples. Compare the probabilities found
    */
   void testMarkovChainFirstOrderGenerate()
   {
      srand( 0 );
      typedef nll::core::Matrix<double>                Matrix;
      typedef nll::core::Buffer1D<double>              Vector;

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
      double tolerance = 5e-2;

      // init the first markov chain
      nll::algorithm::MarkovChainFirstOrderDense markovChain( transition, pi );

      // generate samples
      const unsigned nbSamples = 1000;
      const unsigned chainSize = 6;

      for ( unsigned nb = 0; nb < 20; ++nb )
      {
         std::vector< nll::core::Buffer1D<size_t> > samples( nbSamples );
         for ( unsigned n = 0; n < nbSamples; ++n )
            samples[ n ] = markovChain.generateSequence( chainSize );

         // compute another markov chain using the generated samples
         nll::algorithm::MarkovChainFirstOrderDense markovChainStatistics;
         markovChainStatistics.learn( samples );

         // compare with the expected results
         for ( unsigned n = 0; n < 4; ++n )
         {
            TESTER_ASSERT( nll::core::equal( markovChainStatistics.getInitialStateDistribution()[ n ], pi[ n ], tolerance ) );
            for ( unsigned nn = 0; nn < 4; ++nn )
               TESTER_ASSERT( nll::core::equal( markovChainStatistics.getTransitions()( n, nn ), transition( n, nn ), tolerance ) );
         }
         std::cout << "#";
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMarkovChain);
TESTER_TEST(testMarkovChainFirstOrderGenerate);
TESTER_TEST(testMarkovChainFirstOrder);
TESTER_TEST_SUITE_END();
#endif
