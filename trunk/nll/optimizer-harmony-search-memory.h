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

#ifndef OPTIMIZER_HARMONY_SEARCH_MEMORY_H_
# define OPTIMIZER_HARMONY_SEARCH_MEMORY_H_

# include <list>

# pragma warning( push )
# pragma warning( disable:4251 ) // conversion from 'const double' to XXX, possible loss of data

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Optimizer using a harmony search algorithm. Minimizes an energy funtion using a stochastic aslgorithm.
           Suitable values could be: (hms = 9, hmrc = 0.8, par = 0.1, _bw depends of the problem)
    
           Same optimizer as OptimizerHarmonySearch, but save all the solutions and avoid to compute again a solution
           if it is too close to a solution previously calculated. Especially designed for optimizing a function that
           is "long" to evaluate (else there is an overhead for evaluating the distance, so don't use this one). 
           Since solutions too close to another can't be evaluated, it is less accurate and it may be necessary to optimize
           further with a fast local optimization algorithm.
    @sa OptimizerHarmonySearch
    */
   class NLL_API OptimizerHarmonySearchMemory : public Optimizer
   {
   private:
      typedef f64                            SolutionValue;
      typedef core::Buffer1D<SolutionValue>  SolutionVector;

   public:
      using Optimizer::optimize;

      // workaround for gcc and ambiguous swap. Should be private!
      struct SolutionStorage
      {
         SolutionStorage() : fitness( -1 )
         {}

         SolutionStorage( SolutionVector& sol, SolutionValue solFitness ) : solution( sol ), fitness( solFitness )
         {}

         inline bool operator<( const SolutionStorage& sol ) const
         {
            return fitness > sol.fitness;
         }

         inline SolutionVector::value_type& operator[]( size_t n ){ return solution[ n ]; }

         SolutionVector    solution;
         SolutionValue     fitness;
      };

   private:
      typedef std::vector<SolutionStorage>      HarmonyMemory;
      typedef std::list<SolutionVector>         EvaluatedSolutionStorage;

   public:
      typedef Metric<SolutionVector>   TMetric;

   public:
      /**
       @brief constructor: initialize the parameters of the algorithm
       @param hms harmony memory size
       @param hmrc harmony memory reconsideration rate
       @param par pitch adjustement rate
       @param bandwith the scale of the pitch adjustement
       @param minDistance the minimal distance for a point from all the evaluated point to be evaluated
       @param the metric used to compare 2 points
       */
      OptimizerHarmonySearchMemory( size_t hms, f64 hmrc, f64 par, f64 bw, StopCondition* stop, f64 minDistance, TMetric* metric ) : _hms( hms ), _hmrc( hmrc ), _par( par ), _bw( bw ), _minDistance( minDistance ), _metric( metric ), _stop( stop )
      {
         assert( stop );
         assert( metric );
      }

      virtual std::vector<double> optimize( const OptimizerClient& client, const ParameterOptimizers& parameters, const core::Buffer1D<double>& seed )
      {
         ensure( seed.size() == parameters.size(), "argument mismatch!" );

         _stop->reinit();
         _initializeMemory( client, parameters );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "start OptimizerHarmonySearchMemory..." );
         size_t cycle = 0;
         while ( !_stop->stop( _memory[ 0 ].fitness ) )
         {
            _run( cycle++, seed, client, parameters );
         }

         // log the solution
         std::stringstream sstr;
         sstr << "best=" << _memory[ 0 ].fitness << std::endl;
         sstr << "worst=" << _memory[ _memory.size() - 1].fitness << std::endl;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sstr.str() );

         std::vector<double> best( parameters.size() );
         for ( size_t n = 0; n < best.size(); ++n )
            best[ n ] = _memory[ 0 ][ n ];
         _evaluatedSolutions.clear();
         return best;
      }

   private:
      /**
       @brief Initialize the memory following a uniform distribution on the range of the parameters
       */
      void _initializeMemory( const OptimizerClient& client, const ParameterOptimizers& parameters );

      void _run( size_t cycle, const core::Buffer1D<double>& seed, const OptimizerClient& client, const ParameterOptimizers& parameters )
      {
         // generate a new harmony
         SolutionVector sol( static_cast<size_t>( parameters.size() ) );

         for ( size_t n = 0; n < parameters.size(); ++n )
         {
            if ( cycle == 0 )
            {
               sol[ n ] = seed[ n ];
            } else {
               if ( core::generateUniformDistribution( 0, 1 ) < _hmrc )
               {
                  // memory consideration
                  sol[ n ] = _memory[ rand() % _hms ][ n ];
                  if ( core::generateUniformDistribution( 0, 1 ) < _par )
                  {
                     // pitch adjustement
                     sol[ n ] += core::generateUniformDistribution( -1, 1 ) * _bw;
                  }
               } else {
                  // random selection
                  sol[ n ] = parameters[ n ].generate();
               }
            }
         }

         // test if the solution is far enough
         bool evaluate = true;
         for ( EvaluatedSolutionStorage::const_iterator it = _evaluatedSolutions.begin(); it != _evaluatedSolutions.end(); ++it )
         {
            if ( _metric->distance( sol, *it ) <= _minDistance )
            {
               evaluate = false;
               break;
            }
         }

         if ( evaluate )
         {
            // update memory if needed
            f64 fitness = client.evaluate( sol );
            if ( fitness <= _memory[ _hms - 1 ].fitness )
            {
               _memory[ _hms - 1 ] = SolutionStorage( sol, fitness );
               std::sort( _memory.rbegin(), _memory.rend() );
            }
            _evaluatedSolutions.push_back( sol );
         }
      }

   private:
      size_t  _hms;       // harmony memory size
      f64   _hmrc;      // harmony memory reconsideration rate
      f64   _par;       // pitch adjustement rate
      f64   _bw;        // bandwith

      f64            _minDistance;
      TMetric*       _metric;
      StopCondition* _stop;     // stop condition
      HarmonyMemory  _memory;   // store the solutions
      EvaluatedSolutionStorage _evaluatedSolutions;
   };

   inline void swap( OptimizerHarmonySearchMemory::SolutionStorage& s1, OptimizerHarmonySearchMemory::SolutionStorage& s2 )
   {
      std::swap( s1.fitness, s2.fitness );
      std::swap( s1.solution, s2.solution );
   }
}
}

# pragma warning( pop )

#endif
