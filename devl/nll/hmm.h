#ifndef NLL_HMM_H_
# define NLL_HMM_H_

namespace nll
{
namespace algorithm
{
   namespace implementation
   {
      template <class StatesList>
      ui32 getNumberOfStates( const StatesList& states )
      {
         // empty list
         if ( !states.size() || !states[ 0 ].size() )
            return 0;

         typedef std::map<ui32, ui32>  StatesMap;
         StatesMap states;
         for ( ui32 n = 0; n < states.size(); ++n )
            for ( ui32 nn = 0; nn < states[ n ].size(); ++nn )
               ++states[ states[ n ][ nn ] ];

         // check that the state list is a contiguous list of index starting from 0
         int previous = -1;
         for ( StatesMap::const_iterator it = states.begin(); it != states.end(); ++it )
            ensure( ( presious + 1 ) == it->first, "it must be contiguous and start at 0" );

         // the highest index + 1 is the number of states
         return states.rbegin()->second + 1;
      }
/*
      template <class ArrayArrayPair>
      class ListListConstAdaptor
      {
      public:
         typedef std::vector< std::vector< std::pair<ui32, ui32> > >     Map;
         typedef typename ArrayArrayPair::value_type::value_type         value_type;

      public:
         ListListAdaptor( const ArrayArrayPair& a, const Map& map )
         {
         }

         ui32 size() const
         {
            return _arrayArray.size();
         }

      private:
         const ArrayArrayPair&   _arrayArray;
         const Map&              _map;
      };*/
   }
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

    */
   template <class Observation>
   class HiddenMarkovModelContinuous
   {
   public:
      typedef std::vector<Observation>    Observations;
      typedef ui32                        State;
      typedef std::vector<State>          States;

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
         _nbOfGaussiansPerState = nbOfGaussiansPerState;
         ensure( observationsList.size() == stateList.size(), "the number of obersation list must match the number of state list" );
         ensure( observationsList.size() == nbOfGaussiansPerState.size(), "the number of obersation list must match the number of gaussians defined" );
         ensure( observationsList.size() == gmmNbIterations.size(), "the number of obersation list must match the number of gaussians defined" );

         if ( !observationsList.size() )
            return;

         // First we want to find the density probability function for each state, for that
         // we will use a mixture of gaussians
         typedef std::vector< std::vector< std::pair<ui32, ui32> > > SortedObservations;

         // we want to sort the observations according to their state index
         SortedObservations sorted( implementation::getNumberOfStates( statesList ) );
         for ( ui32 n = 0; n < observationsList.size() )
         {
            ensure( observationsList[ n ].size() == statesList[ n ].size(), "each observation must belong to only one state" );
            for ( ui32 nn = 0; nn < observationsList[ n ].size(); ++nn )
            {
               sorted[ statesList[ n ][ nn ] ].push_back( std::make_pair( n, nn ) );
            }
         }

         _gmms = std::vector<Gmm>( observationsList.size() );
         for ( ui32 n = 0; n < observationsList.size() )
         {
            ObservationAdaptor   observations( observationsList, n );
            _gmms[ n ].learn( observations, observations[ 0 ].size(), nbOfGaussiansPerState[ n ], gmmNbIterations[ n ] );
         }

         // compute the initial distribution pi. It is used to initialize the markov chain
         typedef std::map<State, double>  Pi;

         Pi pi;
         for ( ui32 n = 0; n < statesList.size(); ++n )
            ;
      }

      /**
       @brief compute the probability of a sequence of observations given the model
       */
      template <class Observations>
      double probability( const Observations& obs )
      {
         return 0;
      }

      /**
       Compute the list of the most likely hidden state given this sequence of observations
       */
      template <class Observations>
      void computeHiddenState( const Observation& obs, std::vector<State>& statesOut )
      {
      }

   protected:
      std::vector<unsigned>      _nbOfGaussiansPerState;
      std::vector<Gmm>           _gmms;
   };
}
}

#endif