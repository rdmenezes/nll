#ifndef NLL_OPTIMIZER_GENETIC_ALGORITHM_H_
# define NLL_OPTIMIZER_GENETIC_ALGORITHM_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Implementation of a Genetic Algorithm optimizer. A key point for the success of the algorithm is
           how quick the solution can be evaluated and the size of the original population.
    */
   class NLL_API OptimizerGeneticAlgorithm : public Optimizer
   {
      class NLL_API OptimizerGeneticAlgorithmMutator
      {
      public:
         OptimizerGeneticAlgorithmMutator( const ParameterOptimizers& params ) : _params( params )
         {}
         void operator()( core::Buffer1D<double>& gene ) const
         {
            assert( gene.size() == _params.size() );
            ui32 n = rand() % gene.size();
            const ParameterOptimizer& param = _params[ n ];
            double newval = param.modify( gene[ n ] );
            newval = NLL_BOUND( newval, param.getMin(), param.getMax() );
            gene[ n ] = newval;
         }

      private:
         OptimizerGeneticAlgorithmMutator operator=( const OptimizerGeneticAlgorithmMutator& );

      private:
         const ParameterOptimizers&     _params;
      };

      class NLL_API OptimizerGeneticAlgorithmGenerator
      {
      public:
         OptimizerGeneticAlgorithmGenerator( const ParameterOptimizers& params ) : _params( params )
         {}
         core::Buffer1D<double> operator()() const
         {
            assert( _params.size() );
            core::Buffer1D<double> buf( static_cast<ui32>( _params.size() ) );
            for ( ui32 n = 0; n < _params.size(); ++n )
            {
               buf[ n ] = _params[ n ].generate();
            }
            return buf;
         }

      private:
         OptimizerGeneticAlgorithmGenerator operator=( const OptimizerGeneticAlgorithmGenerator& );

      private:
         const ParameterOptimizers&     _params;
      };

      typedef core::Buffer1D<double>   Gene;

   public:
      OptimizerGeneticAlgorithm(
         ui32  numberOfCycles,
         ui32  populationSize = 100,
         ui32  numberOfPeriods = 10,
         f32   mutationRate = 0.3f,
         f32   selectRate = 0.5f
         ) : _numberOfCycles( numberOfCycles ), _populationSize( populationSize ), _numberOfPeriods( numberOfPeriods ), _mutationRate( mutationRate ), _selectRate( selectRate )
      {}
      virtual std::vector<double> optimize( const OptimizerClient& evaluator,
                                            const ParameterOptimizers& params )
      {
         typedef GeneticAlgorithm<  Gene,
                                    OptimizerGeneticAlgorithmGenerator,
                                    OptimizerClient,
                                    GeneticAlgorithmSelectElitism<Gene, OptimizerClient>,
                                    GeneticAlgorithmStopMaxIteration<Gene>,
                                    GeneticAlgorithmRecombinate2Splits<Gene>,
                                    OptimizerGeneticAlgorithmMutator
                                 > GeneticAlgorithm;

         OptimizerGeneticAlgorithmMutator                      mutator( params );
         OptimizerGeneticAlgorithmGenerator                    generator( params );
         GeneticAlgorithmSelectElitism<Gene, OptimizerClient>  selector( evaluator, true );
         GeneticAlgorithmStopMaxIteration<Gene>                stop( _numberOfCycles );
         GeneticAlgorithmRecombinate2Splits<Gene>              recombinator;
         GeneticAlgorithm geneticAlgorithm( generator, evaluator, selector, stop, recombinator, mutator );

         Gene seed = generator();
         std::vector<Gene> solution = geneticAlgorithm.optimize( _populationSize, _mutationRate, _selectRate, _numberOfPeriods, seed );
         assert( solution.size() );
         return core::convert<Gene, std::vector<double> > ( solution[ 0 ], static_cast<ui32>( seed.size() ) );
      }
   private:
      ui32  _numberOfCycles;
      ui32  _populationSize;
      ui32  _numberOfPeriods;
      f32   _mutationRate;
      f32   _selectRate;
   };
}
}

#endif
