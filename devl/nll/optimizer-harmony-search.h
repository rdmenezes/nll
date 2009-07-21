#ifndef OPTIMIZER_HARMONY_SEARCH_H_
# define OPTIMIZER_HARMONY_SEARCH_H_

# include <set>

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
    
           It is supposed to be faster
           and more accurate than GA. Can be used for continuous or finite optimization.
           See http://www.hydroteq.com/ref_0799_AMC.pdf for implementation details and variants.
    */
   class NLL_API OptimizerHarmonySearch : public Optimizer
   {
   private:
      typedef f64                            SolutionValue;
      typedef core::Buffer1D<SolutionValue>  SolutionVector;

   public:
      // work around for gcc and anbiguous swap operator // should be private!
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

   public:
      /**
       @brief constructor: initialize the parameters of the algorithm
       @param hms harmony memory size
       @param hmrc harmony memory reconsideration rate
       @param par pitch adjustement rate
       @param bandwith the scale of the pitch adjustement
       */
      OptimizerHarmonySearch( ui32 hms, f64 hmrc, f64 par, f64 bw, StopCondition* stop ) : _hms( hms ), _hmrc( hmrc ), _par( par ), _bw( bw ), _stop( stop )
      {
         assert( stop );
      }

      virtual std::vector<double> optimize( const OptimizerClient& client, const ParameterOptimizers& parameters )
      {
         _stop->reinit();
         _initializeMemory( client, parameters );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "start OptimizerHarmonySearch..." );
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

         // update memory if needed
         f64 fitness = client.evaluate( sol );
         if ( fitness <= _memory[ _hms - 1 ].fitness )
         {
            _memory[ _hms - 1 ] = SolutionStorage( sol, fitness );
            std::sort( _memory.rbegin(), _memory.rend() );
         }
      }

   private:
      ui32  _hms;       // harmony memory size
      f64   _hmrc;      // harmony memory reconsideration rate
      f64   _par;       // pitch adjustement rate
      f64   _bw;        // bandwith

      StopCondition* _stop;     // stop condition
      HarmonyMemory  _memory;   // store the solutions
   };

   inline void swap( OptimizerHarmonySearch::SolutionStorage& s1, OptimizerHarmonySearch::SolutionStorage& s2 )
   {
      std::swap( s1.fitness, s2.fitness );
      std::swap( s1.solution, s2.solution );
   }
}
}

# pragma warning( pop )

#endif
