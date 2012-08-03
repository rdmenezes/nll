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

#ifndef NLL_ALGORITHM_HMM_CONTINUOUS_H_
# define NLL_ALGORITHM_HMM_CONTINUOUS_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a hidden markov model

    A hidden markov model is defined by P = (A, B, Pi)
    with A the transition matrix from state T to state T+1
         B the probaility of observing a sample v in state T
         Pi the initial probability p(q)
    <p>
    This class of HMM is dealing with continuous probabilities for the B
    <p>
    Asumptions
     p( qt | qt-1, ..., q0 ) = p( qt | qt-1 )
     p( o | qt, ..., q0 ) = p( o | qt )

     Observation is an array type (operato[], size(), constructor(size))
     Todo: C++ 0x Concept

    */
   template <class Observation, class MarkovChain = MarkovChainFirstOrderDense>
   class HiddenMarkovModelContinuous
   {
   public:
      typedef std::vector<Observation>    Observations;
      typedef size_t                        State;
      typedef std::vector<State>          States;
      typedef core::Buffer1D<double>      Pi;
      typedef core::Matrix<double>        Matrix;

   public:
      /**
       @brief compute a HMM model knowing the hidden variables and observations
       @param observationsList the list of observations. It must be an array of observions.
       @param statesList the states of the correspnding observations. There is a 1 to 1 mapping with the observationsList
              a state is a contiguous list of number starting from 0.
       @param nbOfGaussiansPerState the number of gaussians used to represent the state distribution it is the index of
       @paran gmmNbIterations the number of EM iterations used to learn the state distribution

       StatesList & ObservationsList must define
       Observations &operation[](unsigned) const
       size() const

       @note observations needs to be sorted so they have to be copied. In the case of a lot of observation and with
             memory constraints, it is advised to use reference counted observations.
       @todo C++ 0x add Concept
             ObservationsList & StatesList must define operator[], size(), value_type typedef
             Use specialized wrapper to avoid observations extra copy. Performance probleme in how to build
             the sorted observation list
       */
      template <class ObservationsList, class StatesList>
      void learn( const ObservationsList& observationsList,
                  const StatesList& statesList,
                  const std::vector<unsigned>& nbOfGaussiansPerState,
                  const std::vector<unsigned>& gmmNbIterations )
      {
         size_t nbStates = impl::getNumberOfStates( statesList );
         _nbOfGaussiansPerState = nbOfGaussiansPerState;
         ensure( observationsList.size() == statesList.size(), "the number of obersation list must match the number of state list" );
         ensure( nbStates == nbOfGaussiansPerState.size(), "the number of obersation list must match the number of gaussians defined" );
         ensure( nbStates == gmmNbIterations.size(), "the number of obersation list must match the number of gaussians defined" );

         if ( !observationsList.size() )
            return;

         // First we want to find the density probability function for each state, for that
         // we will use a mixture of gaussians
         typedef std::vector< std::vector< std::pair<size_t, size_t> > > SortedObservations;

         // we want to sort the observations according to their state index
         SortedObservations sorted( nbStates );
         for ( size_t n = 0; n < observationsList.size(); ++n )
         {
            ensure( observationsList[ n ].size() == statesList[ n ].size(), "each observation must belong to only one state" );
            for ( size_t nn = 0; nn < observationsList[ n ].size(); ++nn )
            {
               sorted[ statesList[ n ][ nn ] ].push_back( std::make_pair( n, nn ) );
            }
         }

         _gmms = std::vector<Gmm>( nbStates );
         for ( size_t n = 0; n < nbStates; ++n )
         {
            impl::ObservationsConstAdaptor<ObservationsList> observations( sorted, observationsList, n );
            _gmms[ n ].em( observations, (size_t)observations[ 0 ].size(), nbOfGaussiansPerState[ n ], gmmNbIterations[ n ] );
            //_gmms[ n ].getGaussians()[ 0 ].mean.print( std::cout );
         }

         // use a markov chain to compute the transition matrix and initial state distribution
         MarkovChainFirstOrderDense markovChain;
         markovChain.learn( statesList );
         _pi = markovChain.getInitialStateDistribution();
         _transitions = markovChain.getTransitions();

         // log the parameters of the computed model
         std::stringstream ss;
         ss << "HiddenMarkovModelContinuous.learn" << std::endl
            << "pi:" << std::endl;
         _pi.print( ss );
         ss << "transitions:" << std::endl;
         _transitions.print( ss );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
      }

      /**
       @brief compute the probability of a sequence of observations given the model
       @param observations a sequence of observation. It must conform to an array interface
       */
      template <class Observations>
      double probability( const Observations& obs )
      {
         ensure( obs.size() && _pi.size(), "empty data or model" );
         Matrix emissions( (size_t)_pi.size(), (size_t)obs.size() );
         for ( size_t t = 0; t < obs.size(); ++t )
            for ( size_t s = 0; s < _pi.size(); ++s )
            {
               std::vector<Observation> o( 1 );
               o[ 0 ] = obs[ t ];
               emissions( s, t ) = exp( _gmms[ s ].likelihood( o ) );
            }
         return impl::forward( (size_t)obs.size(), _pi, _transitions, emissions );
      }

      /**
       Compute the list of the most likely hidden state given this sequence of observations

       @param obs a sequence of observation
       @param statesOut return the most probable sequnce of states explaining the observations
       @return the probability p(observations, states|model)
       */
      template <class Observations>
      double computeHiddenState( const Observations& obs, std::vector<State>& statesOut )
      {
         assert( obs.size() && _pi.size() );
         Matrix emissions( (size_t)_pi.size(), (size_t)obs.size() );
         for ( size_t o = 0; o < obs.size(); ++o )
            for ( size_t s = 0; s < _pi.size(); ++s )
            {
               std::vector<Observation> obss( 1 );
               obss[ 0 ] = obs[ o ];
               const double probability = exp( _gmms[ s ].likelihood( obss ) );

               //_gmms[ s ].getGaussians()[ 0 ].mean.print( std::cout );
               //_gmms[ s ].getGaussians()[ 0 ].covariance.print( std::cout );
               emissions( s, o ) = probability;
            }

         size_t endState;
         return impl::viterbi<Matrix, Pi, Matrix>( (size_t)obs.size(), 
                                                    _pi,
                                                    _transitions,
                                                    emissions,
                                                    statesOut,
                                                    endState );
      }

      /**
       @return the computed prior probability for each state
       */
      const Pi& getPi() const
      {
         return _pi;
      }

      /**
       @return the emission probability for each state
       */
      const std::vector<Gmm>& getEmission() const
      {
         return _gmms;
      }

      /**
       @return the transition matrix(i, j) from state i to j
       */
      const core::Matrix<double>& getTransitions() const
      {
         return _transitions;
      }

      /**
       @brief generate a sequence of observations
       @param size the size of the generated sequence
       @param outStates if not null, output the the states that generated this sequence
       @note this can be further optimized as the generators are instanciated for each state.
             For long sequences, they should be precomputed instead.
       */
      std::vector<Observation> generateSequence( size_t size, std::vector<size_t>* outStates = 0 ) const
      {
         ensure( _pi.size(), "learn a model before" );
         ensure( size, "must be >0" );
         std::vector<size_t> sequence( size );
         std::vector<Observation> observations( size );
         sequence[ 0 ] = core::sampling( _pi, 1 )[ 0 ];

         // generate the 1 observation
         // first select a gaussian
         const Gmm::Gaussians& gaussians = _gmms[ sequence[ 0 ] ].getGaussians();
         std::vector<double> g( gaussians.size() );
         for ( size_t nn = 0; nn < gaussians.size(); ++nn )
            g[ nn ] = gaussians[ nn ].weight;
         size_t gaussianId = core::sampling( g, 1 )[ 0 ];

         // generate a sample from this gaussian
         core::NormalMultiVariateDistribution generator( gaussians[ gaussianId ].mean,
                                                         gaussians[ gaussianId ].covariance );
         core::NormalMultiVariateDistribution::VectorT obs = generator.generate();
         observations[ 0 ] = Observation( obs.size() );
         for ( size_t nn = 0; nn < obs.size(); ++nn )
            observations[ 0 ][ nn ] = obs[ nn ];

         // continue generating the sequence following the transition matrix
         for ( size_t n = 1; n < size; ++n )
         {
            // select the new state
            std::vector<double> p( _pi.size() );
            for ( size_t nn = 0; nn < _pi.size(); ++nn )
               p[ nn ] = _transitions( sequence[ n - 1 ], nn );
            sequence[ n ] = core::sampling( p, 1 )[ 0 ];

            // select the gaussian that will generate the sample
            const Gmm::Gaussians& gaussians = _gmms[ sequence[ n ] ].getGaussians();
            std::vector<double> g( gaussians.size() );
            for ( size_t nn = 0; nn < gaussians.size(); ++nn )
               g[ nn ] = gaussians[ nn ].weight;
            size_t gaussianId = core::sampling( g, 1 )[ 0 ];

            // generate a sample from this gaussian
            core::NormalMultiVariateDistribution generator( gaussians[ gaussianId ].mean,
                                                            gaussians[ gaussianId ].covariance );
            core::NormalMultiVariateDistribution::VectorT obs = generator.generate();
            observations[ n ] = Observation( obs.size() );
            for ( size_t nn = 0; nn < obs.size(); ++nn )
               observations[ n ][ nn ] = obs[ nn ];
         }
         if ( outStates )
            *outStates = sequence;
         return observations;
      }

   protected:
      std::vector<unsigned>      _nbOfGaussiansPerState;
      std::vector<Gmm>           _gmms;
      Pi                         _pi;
      Matrix                     _transitions;
   };
}
}

#endif
