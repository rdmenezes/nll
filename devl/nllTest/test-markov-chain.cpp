#include "stdafx.h"
#include <nll/nll.h>

namespace nll
{
namespace algorithm
{
   /**
    @brief A first order markov chain
    */
   class MarkovChainFirstOrder
   {
   public:
      MarkovChainFirstOrder()
      {
      }

      MarkovChainFirstOrder( const core::Matrix<double>& transition )
      {
      }

      ui32 addState()
      {
         return 0;
      }

      void removeState( ui32 state )
      {
      }

      void addTransition( ui32 stateA, ui32 stateB, double probability )
      {
      }

      void removeTransition( ui32 stateA, ui32 stateB )
      {
      }

      std::vector<ui32> generateSequence() const
      {
         std::vector<ui32>();
      }
   };
}
}

class TestMarkovChain
{
public:
   // in this test we already know what hmm generated the samples. Just compare we have the same results
   void testMarkovChainFirstOrder()
   {
   }
};

TESTER_TEST_SUITE(TestMarkovChain);
TESTER_TEST(testMarkovChainFirstOrder);
TESTER_TEST_SUITE_END();