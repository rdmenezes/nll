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

#ifndef NLL_ALGORITHM_HMM_H_
# define NLL_ALGORITHM_HMM_H_

namespace nll
{
namespace algorithm
{
   namespace impl
   {
      /**
       @ingroup algorithm
       @brief Adaptor from an array of array of observation to a vector type
       */
      template <class ArrayArrayPair>
      class ObservationsConstAdaptor
      {
      public:
         typedef std::vector< std::vector< std::pair<size_t, size_t> > >     Map;
         typedef typename ArrayArrayPair::value_type::value_type         value_type;

      public:
         ObservationsConstAdaptor( const Map& map, const ArrayArrayPair& mapValues, size_t state ) : _map( map ), _mapValues( mapValues ), _state( state )
         {
            // nothing to do
         }

         size_t size() const
         {
            return static_cast<size_t>( _map[ _state ].size() );
         }

         const value_type& operator[]( size_t n ) const
         {
            const std::pair<size_t, size_t> index = _map[ _state ][ n ];
            return _mapValues[ index.first ][ index.second ];
         }

      private:
         // disabled
         ObservationsConstAdaptor& operator=( const ObservationsConstAdaptor& );

      private:
         const size_t              _state;
         const Map&              _map;
         const ArrayArrayPair&   _mapValues;
      };

      /**
       @ingroup algorithm
       @brief forward algorithm

       Compute the probability p( obersvations | model )
       */
      template <class EmissionMapper, class Vector, class Mat>
      double forward( size_t nbObservations, const Vector& prior, const Mat& transitions, const EmissionMapper& emission )
      {
         typedef core::Matrix<double> Matrix;

         Matrix tt( prior.size(), nbObservations );
         for ( size_t n = 0; n < prior.size(); ++n )
            tt( n, 0 ) = prior[ n ] * emission( n, 0 );

         for ( size_t t = 1; t < nbObservations; ++t )
         {
            for ( size_t j = 0; j < prior.size(); ++j )
            {
               double sum = 0;
               for ( size_t i = 0; i < prior.size(); ++i )
                  sum += tt( i, t - 1 ) * transitions( i, j );
               tt( j, t ) = sum * emission( j, t );
            }
         }

         double sum = 0;
         for ( size_t i = 0; i < prior.size(); ++i )
            sum += tt( i, nbObservations - 1 );
         return sum;
      }

      /**
       @ingroup algorithm
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
      double viterbi( size_t nbObservations,
                      const Vector& input_prior,
                      const Mat& input_transitions,
                      const EmissionMapper& input_emissions,
                      std::vector<size_t>& output_path,
                      size_t& output_endState )
      {
         typedef core::Matrix<double>  Matrix;
	      Matrix tt( (size_t)input_prior.size(), (size_t)nbObservations );
         core::Matrix<size_t> vv( (size_t)input_prior.size(), (size_t)nbObservations );
	      size_t nb_states = (size_t)input_prior.size();

	      for (size_t n = 0; n < nb_states; ++n)
	      {
		      tt( n, 0 ) = input_prior[ n ] * input_emissions( n, 0 );
		      vv( n, 0 ) = 0;
            //std::cout << "init s[" << n << "]=" << tt( n, 0 ) << " e=" << input_emissions( n, 0 ) << " prior=" << input_prior[ n ] << std::endl;
	      }

	      for ( size_t t = 1; t < nbObservations; ++t )
	      {
		      for (size_t j = 0; j < nb_states; ++j)
		      {
			      int index = -1;
			      double max = -1;
			      for ( size_t i = 0; i < nb_states; ++i )
				      if ( tt( i, t - 1 ) * input_transitions( i, j ) > max )
				      {
					      index = static_cast<int>( i );
					      max = tt( i, t - 1 ) * input_transitions( i, j );
				      }
			      ensure( index != -1, "error" );

			      tt( j, t ) = max * input_emissions( j, t );
			      vv( j, t ) = static_cast<size_t>( index );
		      }
	      }

	      int index = -1;
	      double max = -1;
	      for ( size_t i = 0; i < nb_states; ++i )
		      if ( tt( i, nbObservations - 1 ) > max )
		      {
			      max = tt( i, nbObservations - 1 );
			      index = static_cast<int>( i );
		      }
	      ensure(index != -1, "error");

	      output_endState = static_cast<size_t>( index );
	      size_t cur_state = output_endState;

	      std::vector<size_t> path;
	      path.push_back( cur_state );
	      for ( i32 t = static_cast<i32>( nbObservations ) - 2; t >= 0; --t )
	      {
		      cur_state = vv( cur_state, t + 1 );
		      path.push_back( cur_state );
	      }

         std::vector<size_t> seq;
	      for ( std::vector<size_t>::reverse_iterator it = path.rbegin(); it != path.rend(); ++it )
		      seq.push_back( *it );
         output_path = seq;
         return max;
      }

   }
}
}

#endif
