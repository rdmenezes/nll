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

     Observation is an array type
     Todo: C++ 0x Concept

    */
   template <class Observation, class MarkovChain = MarkovChainFirstOrderDense>
   class HiddenMarkovModelContinuous
   {
   public:
      typedef std::vector<Observation>    Observations;
      typedef ui32                        State;
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
         ui32 nbStates = impl::getNumberOfStates( statesList );
         _nbOfGaussiansPerState = nbOfGaussiansPerState;
         ensure( observationsList.size() == statesList.size(), "the number of obersation list must match the number of state list" );
         ensure( nbStates == nbOfGaussiansPerState.size(), "the number of obersation list must match the number of gaussians defined" );
         ensure( nbStates == gmmNbIterations.size(), "the number of obersation list must match the number of gaussians defined" );

         if ( !observationsList.size() )
            return;

         // First we want to find the density probability function for each state, for that
         // we will use a mixture of gaussians
         typedef std::vector< std::vector< std::pair<ui32, ui32> > > SortedObservations;

         // we want to sort the observations according to their state index
         SortedObservations sorted( nbStates );
         for ( ui32 n = 0; n < observationsList.size(); ++n )
         {
            ensure( observationsList[ n ].size() == statesList[ n ].size(), "each observation must belong to only one state" );
            for ( ui32 nn = 0; nn < observationsList[ n ].size(); ++nn )
            {
               sorted[ statesList[ n ][ nn ] ].push_back( std::make_pair( n, nn ) );
            }
         }

         _gmms = std::vector<Gmm>( nbStates );
         for ( ui32 n = 0; n < nbStates; ++n )
         {
            impl::ObservationsConstAdaptor<ObservationsList> observations( sorted, observationsList, n );
            _gmms[ n ].em( observations, (ui32)observations[ 0 ].size(), nbOfGaussiansPerState[ n ], gmmNbIterations[ n ] );
            //_gmms[ n ].getGaussians()[ 0 ].mean.print( std::cout );
         }

         // use a markov chain to compute the transition matrix and initial state distribution
         MarkovChainFirstOrderDense markovChain;
         markovChain.learn( statesList );
         _pi = markovChain.getInitialStateDistribution();
         _transitions = markovChain.getTransitions();
      }

      /**
       @brief compute the probability of a sequence of observations given the model
       @param observations a sequence of observation. It must conform to an array interface
       */
      template <class Observations>
      double probability( const Observations& obs )
      {
         return 0;
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
         Matrix emissions( (ui32)_pi.size(), (ui32)obs.size() );
         for ( ui32 o = 0; o < obs.size(); ++o )
            for ( ui32 s = 0; s < _pi.size(); ++s )
            {
               std::vector<Observation> obss( 1 );
               obss[ 0 ] = obs[ o ];
               const double probability = exp( _gmms[ s ].likelihood( obss ) );
               emissions( s, o ) = probability;
            }

         ui32 endState;
         return impl::viterbi<Matrix, Pi, Matrix>( (ui32)obs.size(), 
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

      std::vector<Observation> generateSequence() const
      {
         return std::vector<Observation>();
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
