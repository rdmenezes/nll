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

#ifndef NLL_FEATURE_SELECTION_GENETIC_ALGORITHM_H_
# define NLL_FEATURE_SELECTION_GENETIC_ALGORITHM_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Feature selection using a wrapper approach and a genetic algorithm. The
           GA will select and rate feature subset. These subsets will evolve using
           GA. As for all the wrapper techniques, it is rather slow, thus not suitable
           for a very high dimentional feature set.
    */
   template <class Point, class TClassifier = ClassifierBase<Point, size_t> >
   class FeatureSelectionGeneticAlgorithm : public FeatureSelectionWrapper<Point, TClassifier>
   {
      typedef FeatureSelectionWrapper<Point, TClassifier>   Base;
      typedef typename Base::Database                       Database;
      typedef typename Base::Classifier                     Classifier;
      typedef core::Buffer1D<bool>                          Gene;

   public:
      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   private:
      class OperatorMutate
      {
      public:
         void operator()( Gene& gene ) const
         {
            size_t geneId = rand() % gene.size();
            gene[ geneId ] = !gene[ geneId ];
         }
      };

      class OperatorGenerate
      {
      public:
         OperatorGenerate( f64 ratioSize, size_t geneSize ) : _ratioSize( ratioSize ), _geneSize( geneSize ){}
         Gene operator()() const
         {
            size_t prob = static_cast<size_t>( _ratioSize * 10000 );
            Gene gene( _geneSize );
            for ( size_t n = 0; n < _geneSize; ++n)
            {
               size_t r = rand() % 10000;
               if ( r < prob )
                  gene[ n ] = 1;
               else
                  gene[ n ] = 0;
            }
            return gene;
         }

      private:
         f64   _ratioSize;
         size_t  _geneSize;
      };

      class OperatorEvaluate
      {
      public:
         // null feature selection, only used to process the database
         class FeatureSelectionUtility : public FeatureSelectionWrapper<Point, TClassifier>
         {
         public:
            typedef FeatureSelectionWrapper<Point, TClassifier>   Base;
            typedef typename Base::Classifier                     Classifier;
            typedef typename Classifier::Database                 Database;

         public:
            FeatureSelectionUtility( const core::Buffer1D<bool> buf ) : Base( buf ){}
            virtual core::Buffer1D<bool> _compute( const Classifier* , const core::Buffer1D<f64>& , const Database& ){ return core::Buffer1D<bool>(); }
         };

         OperatorEvaluate( const Database& dat,
            const Classifier* classifier,
            const core::Buffer1D<f64>& parameters ) : _dat( dat ), _classifier( classifier ), _parameters( parameters ){}
         f64 evaluate( const Gene& parameters ) const
         {
            
            FeatureSelectionUtility utility( parameters );
            if ( utility.getNumberOfSelectedFeatures() == 0 )
               return static_cast<f64>( INT_MIN );
            Database dat = utility.transform( _dat );
            return _classifier->evaluate( _parameters, dat );
         }
         f64 operator()( const Gene& parameters ) const
         {
            return evaluate( parameters );
         }

      private:
         OperatorEvaluate& operator=( const OperatorEvaluate& );

      private:
         const Database&            _dat;
         const Classifier*          _classifier;
         const core::Buffer1D<f64>& _parameters;
      };
   public:
      FeatureSelectionGeneticAlgorithm( f64 initialFeatureSizeRatio, size_t populationSize = 100, size_t nbRounds = 40, size_t nbPeriods = 5, f64 mutationRate = 0.3, f64 selectionRate = 0.3 ) :
         _initialFeatureSizeRatio( initialFeatureSizeRatio ), _populationSize( populationSize ), _nbRounds( nbRounds ), _nbPeriods( nbPeriods ), _mutationRate( mutationRate ), _selectionRate( selectionRate )
         {}

   protected:
      // parameters : parameters of the learning algorithm
      core::Buffer1D<bool> _compute( const Classifier* classifier, const core::Buffer1D<f64>& parameters, const Database& dat )
      {
         if ( !dat.size() )
            return core::Buffer1D<bool>();
         size_t geneSize = dat[ 0 ].input.size();
         typedef GeneticAlgorithm<  Gene,
                                    OperatorGenerate,
                                    OperatorEvaluate,
                                    GeneticAlgorithmSelectElitism<Gene, OperatorEvaluate>,
                                    GeneticAlgorithmStopMaxIteration<Gene>,
                                    GeneticAlgorithmRecombinate2Splits<Gene>,
                                    OperatorMutate
                                 > GeneticAlgorithm;

         OperatorEvaluate                                               evaluator( dat, classifier, parameters );
         OperatorMutate                                                 mutator;
         OperatorGenerate                                               generator( _initialFeatureSizeRatio, geneSize );
         GeneticAlgorithmSelectElitism<Gene, OperatorEvaluate>          selector( evaluator, true );
         GeneticAlgorithmStopMaxIteration<Gene>                         stop( _nbRounds );
         GeneticAlgorithmRecombinate2Splits<Gene>                       recombinator;
         GeneticAlgorithm geneticAlgorithm( generator, evaluator, selector, stop, recombinator, mutator );

         Gene seed( geneSize );
         for ( size_t n = 0; n < geneSize; ++n )
            seed[ n ] = true;
         std::vector<Gene> solution = geneticAlgorithm.optimize( _populationSize, _mutationRate, _selectionRate, _nbPeriods, seed );

         assert( solution.size() );
         return solution[ 0 ];
      }

   private:
      f64      _initialFeatureSizeRatio;
      size_t      _populationSize;
      size_t     _nbRounds;
      size_t     _nbPeriods;
      f64      _mutationRate;
      f64      _selectionRate;
   };
}
}

#endif
