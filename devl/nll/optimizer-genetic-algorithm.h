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
            size_t n = rand() % gene.size();
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
            core::Buffer1D<double> buf( static_cast<size_t>( _params.size() ) );
            for ( size_t n = 0; n < _params.size(); ++n )
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
         size_t  numberOfCycles,
         size_t  populationSize = 100,
         size_t  numberOfPeriods = 10,
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
         return core::convert<Gene, std::vector<double> > ( solution[ 0 ], static_cast<size_t>( seed.size() ) );
      }
   private:
      size_t  _numberOfCycles;
      size_t  _populationSize;
      size_t  _numberOfPeriods;
      f32   _mutationRate;
      f32   _selectRate;
   };
}
}

#endif
