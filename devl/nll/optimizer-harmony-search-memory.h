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

         inline SolutionVector::value_type& operator[]( ui32 n ){ return solution[ n ]; }

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
      OptimizerHarmonySearchMemory( ui32 hms, f64 hmrc, f64 par, f64 bw, StopCondition* stop, f64 minDistance, TMetric* metric ) : _hms( hms ), _hmrc( hmrc ), _par( par ), _bw( bw ), _minDistance( minDistance ), _metric( metric ), _stop( stop )
      {
         assert( stop );
         assert( metric );
      }

      virtual std::vector<double> optimize( const OptimizerClient& client, const ParameterOptimizers& parameters )
      {
         _stop->reinit();
         _initializeMemory( client, parameters );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "start OptimizerHarmonySearchMemory..." );
         while ( !_stop->stop( _memory[ 0 ].fitness ) )
         {
            _run( client, parameters );

            // log the solution
            std::stringstream sstr;
            sstr << "best=" << _memory[ 0 ].fitness << std::endl;
            sstr << "worst=" << _memory[ _memory.size() - 1].fitness << std::endl;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sstr.str() );
         }

         std::vector<double> best( parameters.size() );
         for ( ui32 n = 0; n < best.size(); ++n )
            best[ n ] = _memory[ 0 ][ n ];
         _evaluatedSolutions.clear();
         return best;
      }

   private:
      /**
       @brief Initialize the memory following a uniform distribution on the range of the parameters
       */
      void _initializeMemory( const OptimizerClient& client, const ParameterOptimizers& parameters );

      void _run( const OptimizerClient& client, const ParameterOptimizers& parameters )
      {
         // generate a new harmony
         SolutionVector sol( static_cast<ui32>( parameters.size() ) );

         for ( ui32 n = 0; n < parameters.size(); ++n )
         {
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
      ui32  _hms;       // harmony memory size
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
