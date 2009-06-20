#include "stdafx.h"
#include <nll/nll.h>

namespace nll
{
namespace algorithm
{
   namespace implementation
   {
      /**
       @brief Compute the number of states in a list of list of observation
              The state list is checked for coherency
       */
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

      /**
       @brief Adaptor from an array of array of observation to a vector type
       */
      template <class ArrayArrayPair>
      class ObservationsConstAdaptor
      {
      public:
         typedef std::vector< std::vector< std::pair<ui32, ui32> > >     Map;
         typedef typename ArrayArrayPair::value_type::value_type         value_type;

      public:
         ObservationsConstAdaptor( const Map& map, const ArrayArrayPair& mapValues, ui32 state ) : _map( map ), _mapValues( mapValues ), _state( state )
         {
            // nothing to do
         }

         ui32 size() const
         {
            return _map[ _state ].size();
         }

         const value_type& operator[]( ui32 n ) const
         {
            const std::pair<ui32, ui32> index = _map[ n ];
            return _mapValues[ index.first ][ index.second ];
         }

      private:
         const ui32              _state;
         const Map&              _map;
         const ArrayArrayPair&   _mapValues;
      };

      /**
       @brief forward algorithm

       Compute the probability p( obersvations | model )
       */
      template <class EmissionMapper, class Vector, class Mat>
      double forward( ui32 nbObservations, const Vector& prior, const Mat& transitions, const EmissionMapper& emission )
      {
         typedef core::Matrix<double> Matrix;

         Matrix tt( prior.size(), 2 );
         for ( ui32 n = 0; n < prior.size(); ++n )
            tt( n, 0 ) = prior[ n ] * emission( 0, n );

         for ( ui32 t = 1; t < nbObservations; ++t )
         {
            for ( ui32 j = 0; j < prior.size(); ++j )
            {
               double sum = 0;
               for ( ui32 i = 0; i < prior.size(); ++i )
                  sum += tt( i, t - 1 ) * transitions( i, j )
               tt( j, t ) = sum * emission( t, j );
            }
         }

         double sum = 0;
         for ( ui32 i = 0; i < prior.size(); ++i )
            sum += tt( i, nbObservations - 1 );
         return sum;
      }

      /**
       @brief viterbi algorithm

       EmissionMapper must define operator(i, j), which returns the probability to see observation j in state i
       Mat must define operator(i, j) must returns p(state_j|state_i)

       @param nbObservations the number of observations in the sequence
       @param input_prior the prior of a state
       @param transition the transition matrix from state to state
       @param input_emissions the emission probability
       @param output_path return the best sequence
       @return p(oberservations, states|model)
       */
      template <class EmissionMapper, class Vector, class Mat, class Emissions>
      typename double viterbi( ui32 nbObservations,
                               const Vector& input_prior,
                               const Mat& input_transitions,
                               const EmissionMapper& input_emissions,
                               std::vector<ui32>& output_path,
                               ui32& output_endState )
      {
         typedef core::Matrix<double>  Matrix;
	      Matrix tt( input_prior.size(), nbObservations );
	      Matrix vv( input_prior.size(), nbObservations );
	      ui32 nb_states = input_prior.size();

	      for (ui32 n = 0; n < nb_states; ++n)
	      {
		      tt( n, 0 ) = input_prior[ n ] * input_emissions( 0, n );
		      vv( n, 0 ) = 0;
	      }

	      for ( ui32 t = 1; t < nbObservations; ++t )
	      {
		      for (ui32 j = 0; j < nb_states; ++j)
		      {
			      int index = -1;
			      double max = -1;
			      for ( ui32 i = 0; i < nb_states; ++i )
				      if ( tt( i, t - 1 ) * input_transitions( i, j ) > max )
				      {
					      index = i;
					      max = tt( i, t - 1 ) * input_transitions( i, j );
				      }
			      ensure( index != -1, "error" );

			      tt( j, t ) = max * input_emissions( t, j );
			      vv( j, t ) = index;
		      }
	      }

	      int index = -1;
	      double max = -1;
	      for ( ui32 i = 0; i < nb_states; ++i )
		      if ( tt( i, nbObservations - 1 ) > max )
		      {
			      max = tt( i, nb_obs - 1 );
			      index = i;
		      }
	      ensure(index != -1, "error");

	      output_endState = index;
	      ui32 cur_state = output_endState;

	      std::vector<ui32> path;
	      path.push_back( cur_state );
	      for ( i32 t = nbObservations - 2; t >= 0; --t )
	      {
		      cur_state = vv( cur_state, t + 1 );
		      path.push_back( cur_state );
	      }

         std::vector<ui32> seq;
	      for ( std::vector<ui32>::reverse_iterator it = path.rbegin(); it != path.rend(); ++it )
		      seq.push_back( *it );
         output_path = std::vector<ui32>( seq.rebegin(), seq.rend() );
         return max;
      }

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

     Observation is an array type
     Todo: C++ 0x Concept

    */
   template <class Observation>
   class HiddenMarkovModelContinuous
   {
   public:
      typedef std::vector<Observation>    Observations;
      typedef ui32                        State;
      typedef std::vector<State>          States;
      typedef std::vector<double>         Pi;
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
         ui32 nbStates = implementation::getNumberOfStates( statesList );
         SortedObservations sorted( nbStates );
         for ( ui32 n = 0; n < observationsList.size() )
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
            ObservationAdaptor observations( statesList, observationsList, n );
            _gmms[ n ].learn( observations, observations[ 0 ].size(), nbOfGaussiansPerState[ n ], gmmNbIterations[ n ] );
         }

         // compute the initial distribution pi. It is used to initialize the markov chain
         ui32 nbStatesInLearning = 0;
         for ( ui32 n = 0; n < statesList.size(); ++n )
            nbStatesInLearning += sorted[ n ].size();
         _pi = Pi( nbStates );
         for ( ui32 n = 0; n < statesList.size(); ++n )
            _pi[ n ] = sorted[ n ].size() / nbStatesInLearning;

         // compute the transition matrix
         _transitions = Matrix( nbStates, nbStates );
         for ( ui32 n = 0; n < observationsList.size(); ++n )
            for ( ui32 nn = 0; nn < observationsList[ n ].size() - 1; ++nn )
            {
               ui32 s1 = observationsList[ n ][ nn ];
               ui32 s2 = observationsList[ n ][ nn + 1 ];
               ++_transitions( s1, s2 );
            }

         for ( ui32 s1 = 0; s1 < nbStates; ++s1 )
            for ( ui32 s2 = 0; s2 < nbStates; ++s2 )
               _transitions( s1, s2 ) /= sorted[ s1 ];
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
       @param probability return the p(observations, states|model)
       */
      template <class Observations>
      void computeHiddenState( const Observations& obs, std::vector<State>& statesOut, double& probability )
      {
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


class TestHmm
{
public:
   void testHmm()
   {
    
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestHmm);
TESTER_TEST(testHmm);
TESTER_TEST_SUITE_END();
#endif
