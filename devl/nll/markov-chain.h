/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ALGORITHM_MARKOV_CHAIN_H_
# define NLL_ALGORITHM_MARKOV_CHAIN_H_

namespace nll
{
namespace algorithm
{
   namespace impl
   {
      /**
       @ingroup algorithm
       @brief Compute the number of states in a list of list of observation
       */
      template <class StatesList>
      size_t getNumberOfStates( const StatesList& statesList )
      {
         // empty list
         if ( !statesList.size() || !statesList[ 0 ].size() )
            return 0;

         typedef std::map<size_t, size_t>  StatesMap;
         StatesMap states;
         for ( size_t n = 0; n < statesList.size(); ++n )
            for ( size_t nn = 0; nn < statesList[ n ].size(); ++nn )
               ++states[ statesList[ n ][ nn ] ];

         // check that the state list is a contiguous list of index starting from 0
         int previous = -1;
         for ( StatesMap::const_iterator it = states.begin(); it != states.end(); ++it, ++previous )
         {
            ensure( ( previous + 1 ) ==  static_cast<int>( it->first ), "it must be contiguous and start at 0" );
         }

         // the highest index + 1 is the number of states
         return states.rbegin()->first + 1;
      }
   }

   /**
    @ingroup algorithm
    @brief A first order markov chain.
    
    Internally, a dense representation is used.
    */
   class MarkovChainFirstOrderDense
   {
      typedef core::Matrix<double>     Transitions;
      typedef core::Buffer1D<double>   Vectorf;

   public:
      /**
       @brief Create an empty markov chain
       */
      MarkovChainFirstOrderDense()
      {
         // nothing to do
      }

      /**
       @brief Create a markov chain from a transition matrix and initial state probability
       @param transitions transitions(a, b) defines the transition probability from state a to b. Must define
                          double operator(unsigned, unsigned) const
                          unsigned sizex() const
                          unsigned sizey() const
       @param pi pi[ a ] defines the initial state probability for the sample 0
       */
      template <class Matrix, class Vector>
      MarkovChainFirstOrderDense( const Matrix& transitions, const Vector& pi )
      {
         ensure( transitions.sizex() == transitions.sizey(), "must be a square matrix" );
         ensure( transitions.sizex() == pi.size(), "the number of state must match in the transition matrix and initial state distribution" );
         _transitions = Transitions( transitions.sizex(), transitions.sizey() );
         for ( size_t nx = 0; nx < transitions.sizex(); ++nx )
            for ( size_t ny = 0; ny < transitions.sizey(); ++ny )
               _transitions( nx, ny ) = static_cast<double>( transitions( nx, ny ) );
         _pi = Vectorf( pi.size() );
         for ( size_t n = 0; n < pi.size(); ++n )
            _pi[ n ] = static_cast<double>( pi[ n ] );
      }

      /**
       @return a matrix(a, b) representing the probability transition from state a to state b
       */
      const Transitions& getTransitions() const
      {
         return _transitions;
      }

      /**
       @return the initial state distribution (i.e. how the markov chain is initialized)
       */
      const Vectorf& getInitialStateDistribution() const
      {
         return _pi;
      }

      /**
       @brief Learn a new markov chain from a serie of samples. The states must be contiguous and start at 0

       <code>SamplesList</code> must be a vector of vector of integers. i.e. SamplesList[ 0 ]
       returns the first a list of samples that would be generated by markov chain that we
       want to learn.
       */
      template <class SamplesList>
      void learn( const SamplesList& samplesList )
      {
         const size_t nbStates = impl::getNumberOfStates( samplesList );
         if ( nbStates == 0 )
            return;

         // compute pi
         _pi = Vectorf( nbStates );
         for ( size_t n = 0; n < samplesList.size(); ++n )
         {
            ensure( samplesList[ n ].size(), "samples may not be empty" );
            ++_pi[ samplesList[ n ][ 0 ] ];
         }
         for ( size_t n = 0; n < nbStates; ++n )
            _pi[ n ] /= static_cast<double>( samplesList.size() );

         // compute the transition matrix
         _transitions = Transitions( nbStates, nbStates );
         for ( size_t n = 0; n < samplesList.size(); ++n )
            for ( size_t nn = 0; nn < samplesList[ n ].size() - 1; ++nn )
            {
               size_t s1 = samplesList[ n ][ nn ];
               size_t s2 = samplesList[ n ][ nn + 1 ];
               ++_transitions( s1, s2 );
            }
         for ( size_t s1 = 0; s1 < nbStates; ++s1 )
         {
            double norme = 0;
            for ( size_t s2 = 0; s2 < nbStates; ++s2 )
               norme += _transitions( s1, s2 );
            for ( size_t s2 = 0; s2 < nbStates; ++s2 )
               _transitions( s1, s2 ) /= norme;
         }
      }


      /**
       @brief Generate samples using the defined markov chain
       */
      core::Buffer1D<size_t> generateSequence( size_t size ) const
      {
         const size_t nbStates = _pi.size();
         ensure( nbStates, "init the markov chain first" );
         core::Buffer1D<size_t> sequence( size );

         // find the initial state
         sequence[ 0 ] = core::sampling( _pi, 1 )[ 0 ];

         // create the chain
         for ( size_t n = 1; n < size; ++n )
         {
            std::vector<double> proba( nbStates );
            for ( unsigned nn = 0; nn < nbStates; ++nn )
               proba[ nn ] = _transitions( sequence[ n - 1 ], nn );
            sequence[ n ] = nll::core::sampling( proba, 1 )[ 0 ];
         }
         return sequence;
      }

      /**
       @brief Serialize the markov chain to a stream
       */
      void write( std::ostream& o ) const
      {
         _pi.write( o );
         _transitions.write( o );
      }

      /**
       @brief Unserialize the markov chain to a stream
       */
      void read( std::istream& i )
      {
         _pi.read( i );
         _transitions.read( i );
      }

   protected:
      Vectorf     _pi;
      Transitions _transitions;

   };
}
}

#endif
