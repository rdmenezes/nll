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
   template <class Point>
   class FeatureSelectionGeneticAlgorithm : public FeatureSelectionWrapper<Point>
   {
      typedef FeatureSelectionWrapper<Point> Base;
      typedef typename Base::Database        Database;
      typedef typename Base::Classifier      Classifier;
      typedef core::Buffer1D<bool>           Gene;

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
            ui32 geneId = rand() % gene.size();
            gene[ geneId ] = !gene[ geneId ];
         }
      };

      class OperatorGenerate
      {
      public:
         OperatorGenerate( f64 ratioSize, ui32 geneSize ) : _ratioSize( ratioSize ), _geneSize( geneSize ){}
         Gene operator()() const
         {
            ui32 prob = static_cast<ui32>( _ratioSize * 10000 );
            Gene gene( _geneSize );
            for ( ui32 n = 0; n < _geneSize; ++n)
            {
               ui32 r = rand() % 10000;
               if ( r < prob )
                  gene[ n ] = 1;
               else
                  gene[ n ] = 0;
            }
            return gene;
         }

      private:
         f64   _ratioSize;
         ui32  _geneSize;
      };

      class OperatorEvaluate
      {
      public:
         // null feature selection, only used to process the database
         class FeatureSelectionUtility : public FeatureSelectionWrapper<Point>
         {
         public:
            typedef FeatureSelectionWrapper<Point>    Base;
            typedef typename Base::Classifier         Classifier;
            typedef typename Classifier::Database     Database;

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
            Database dat = utility.process( _dat );
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
      FeatureSelectionGeneticAlgorithm( f64 initialFeatureSizeRatio, ui32 populationSize = 100, ui32 nbRounds = 40, ui32 nbPeriods = 5, f64 mutationRate = 0.3, f64 selectionRate = 0.3 ) :
         _initialFeatureSizeRatio( initialFeatureSizeRatio ), _populationSize( populationSize ), _nbRounds( nbRounds ), _nbPeriods( nbPeriods ), _mutationRate( mutationRate ), _selectionRate( selectionRate )
         {}

   protected:
      // parameters : parameters of the learning algorithm
      core::Buffer1D<bool> _compute( const Classifier* classifier, const core::Buffer1D<f64>& parameters, const Database& dat )
      {
         if ( !dat.size() )
            return core::Buffer1D<bool>();
         ui32 geneSize = dat[ 0 ].input.size();
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
         for ( ui32 n = 0; n < geneSize; ++n )
            seed[ n ] = true;
         std::vector<Gene> solution = geneticAlgorithm.optimize( _populationSize, _mutationRate, _selectionRate, _nbPeriods, seed );

         assert( solution.size() );
         return solution[ 0 ];
      }

   private:
      f64      _initialFeatureSizeRatio;
      ui32      _populationSize;
      ui32     _nbRounds;
      ui32     _nbPeriods;
      f64      _mutationRate;
      f64      _selectionRate;
   };
}
}

#endif
