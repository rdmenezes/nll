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
       */
      template <class StatesList>
      ui32 getNumberOfStates( const StatesList& statesList )
      {
         // empty list
         if ( !statesList.size() || !statesList[ 0 ].size() )
            return 0;

         typedef std::map<ui32, ui32>  StatesMap;
         StatesMap states;
         for ( ui32 n = 0; n < statesList.size(); ++n )
            for ( ui32 nn = 0; nn < statesList[ n ].size(); ++nn )
               ++states[ statesList[ n ][ nn ] ];

         // check that the state list is a contiguous list of index starting from 0
         int previous = -1;
         for ( StatesMap::const_iterator it = states.begin(); it != states.end(); ++it, ++previous )
         {
            ensure(( previous + 1 ) ==  static_cast<int>( it->first ), "it must be contiguous and start at 0" );
         }

         // the highest index + 1 is the number of states
         return states.rbegin()->first + 1;
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
            return static_cast<ui32>( _map[ _state ].size() );
         }

         const value_type& operator[]( ui32 n ) const
         {
            const std::pair<ui32, ui32> index = _map[ _state ][ n ];
            return _mapValues[ index.first ][ index.second ];
         }

      private:
         // disabled
         ObservationsConstAdaptor& operator=( const ObservationsConstAdaptor& );

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
      template <class EmissionMapper, class Vector, class Mat>
      double viterbi( ui32 nbObservations,
                      const Vector& input_prior,
                      const Mat& input_transitions,
                      const EmissionMapper& input_emissions,
                      std::vector<ui32>& output_path,
                      ui32& output_endState )
      {
         typedef core::Matrix<double>  Matrix;
	      Matrix tt( (ui32)input_prior.size(), (ui32)nbObservations );
         core::Matrix<ui32> vv( (ui32)input_prior.size(), (ui32)nbObservations );
	      ui32 nb_states = (ui32)input_prior.size();

	      for (ui32 n = 0; n < nb_states; ++n)
	      {
		      tt( n, 0 ) = input_prior[ n ] * input_emissions( 0, n );
		      vv( n, 0 ) = 0;
            std::cout << "init s[" << n << "]=" << tt( n, 0 ) << " e=" << input_emissions( 0, n ) << std::endl;
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

			      tt( j, t ) = max * input_emissions( j, t );
			      vv( j, t ) = index;
		      }
	      }

	      int index = -1;
	      double max = -1;
	      for ( ui32 i = 0; i < nb_states; ++i )
		      if ( tt( i, nbObservations - 1 ) > max )
		      {
			      max = tt( i, nbObservations - 1 );
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
         output_path = seq;
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
         ui32 nbStates = implementation::getNumberOfStates( statesList );
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
            implementation::ObservationsConstAdaptor<ObservationsList> observations( sorted, observationsList, n );
            _gmms[ n ].em( observations, (ui32)observations[ 0 ].size(), nbOfGaussiansPerState[ n ], gmmNbIterations[ n ] );
         }

         // compute the initial distribution pi. It is used to initialize the markov chain
         ui32 nbStatesInLearning = 0;
         for ( ui32 n = 0; n < nbStates; ++n )
            nbStatesInLearning += static_cast<ui32>( sorted[ n ].size() );
         _pi = Pi( nbStates );
         for ( ui32 n = 0; n < statesList.size(); ++n )
         {
            ui32 state = statesList[ n ][ 0 ];
            ++_pi[ state ];
         }
         for ( ui32 n = 0; n < nbStates; ++n )
            _pi[ n ] /= static_cast<double>( statesList.size() );

         // compute the transition matrix
         _transitions = Matrix( nbStates, nbStates );
         for ( ui32 n = 0; n < observationsList.size(); ++n )
            for ( ui32 nn = 0; nn < observationsList[ n ].size() - 1; ++nn )
            {
               ui32 s1 = statesList[ n ][ nn ];
               ui32 s2 = statesList[ n ][ nn + 1 ];
               ++_transitions( s1, s2 );
            }

         for ( ui32 s1 = 0; s1 < nbStates; ++s1 )
         {
            double norme = 0;
            for ( ui32 s2 = 0; s2 < nbStates; ++s2 )
               norme += _transitions( s1, s2 );
            for ( ui32 s2 = 0; s2 < nbStates; ++s2 )
               _transitions( s1, s2 ) /= norme;
         }
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
         return implementation::viterbi<Matrix, Pi, Matrix>( (ui32)obs.size(), 
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

using namespace nll::algorithm;

class TestHmmContinuous
{
public:
   // in this test we already know what hmm generated the samples. Just compare we have the same results
   void testHmm1()
   {
      //1245617666
      unsigned seed = 0;//time(0);
      std::cout << "seed=" << seed << std::endl;
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
         {0.5, 2,   0.25}
      };

      // generate observations by state
      std::vector<Observations> observations( nbStates );
      for ( unsigned n = 0; n < nbStates; ++n )
      {
         for ( unsigned nn = 0; nn < 100; ++nn )
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
            const unsigned observationIndex = (unsigned)rand() % observations[ chain[ nn ] ].size();
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

      std::vector<nll::ui32> states;
      hmm.computeHiddenState( dataset[ 0 ], states );

      hmm.getTransitions().print( std::cout );
      for ( unsigned n = 0; n < 4; ++n )
         hmm.getEmission()[ n ].getGaussians()[ 0 ].mean.print( std::cout );
   }
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestHmmContinuous);
TESTER_TEST(testHmm1);
TESTER_TEST_SUITE_END();
//#endif
