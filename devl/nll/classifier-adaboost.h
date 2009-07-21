#ifndef NLL_CLASSIFIER_ADABOOST_H_
# define NLL_CLASSIFIER_ADABOOST_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Adaboost classifier. Base on Adaboost.M1, it is modified so that any classifier could be used
          (ie instead of a base classifier necessiting the weight of the distribution).

    The <code>Classifier</code> is the classifier template (ie <code>ClassifierSvm, ClassifierGmm</code>...).
    This choice has been made because we need to know the exact type of the classifier so that we can save/load
    it from a file, and we also want a strong typing between the <code>T</code> parameter of the <code>ClassifierAdaboost</code>
    and the one from <code>Classifier</code>.
    */
   template <class T, template <typename> class WeakClassifier>
   class ClassifierAdaboost : public Classifier<T>
   {
      typedef WeakClassifier<T>        BaseWeakLearner;
      typedef Classifier<T>            Base;
      typedef typename Base::Database  Database;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

   public:
      static nll::algorithm::ParameterOptimizers buildParameters()
      {
         nll::algorithm::ParameterOptimizers parameters;
         return parameters;
      }

   public:
      /**
       @brief Constructor
       @param source this is the source classifier. Other classifiers will be created by invoking <code>deepCopy()</code>
              A local pointer of the copy is kept by this classifier. It is the responsability of the caller to keep it alive,
              and dealocate it if necesary ( if ownsSource = false, else it is automatically deleted ).
       @param iterations the number of iterations. For each iteration, a new classifier will be generated on a
              dataset sampled according to the internal error distribution
       @param subsetRatio a proportion ( < 1 ) of the original training dataset that will be used to make the training dataset
              for each iteration
       */
      ClassifierAdaboost( const BaseWeakLearner* source, ui32 iterations, double subsetRatio, bool ownsSource = false ) : Base( buildParameters() ),
         _source( source ), _nbIterations( iterations ), _subsetRatio( subsetRatio ), _ownsSource( ownsSource )
      {
         ensure( source, "source must not be null" );
      }

      ~ClassifierAdaboost()
      {
         _destroy();
         if ( _ownsSource )
            delete _source;
      }

      virtual Base* deepCopy() const
      {
         // BaseWeakLearner are strongly linked, that's why we need a reinterpret cast!
         ClassifierAdaboost* c = new ClassifierAdaboost( reinterpret_cast<BaseWeakLearner*>( _source->deepCopy() ), _nbIterations, _subsetRatio, true );
         c->_classifiers = std::vector<BaseWeakLearner*>( _classifiers.size() );
         for ( ui32 n = 0; n < _classifiers.size(); ++n )
            c->_classifiers[ n ] = reinterpret_cast<BaseWeakLearner*>( _classifiers[ n ]->deepCopy() );
         c->_alphas = _alphas;
         c->_crossValidationBin = _crossValidationBin;
         return c;
      }

      /**
       @todo implement
       */
      virtual void read( std::istream& i )
      {
         unreachable( "not fully implemented yet" );
         // TODO read the number of classes
         core::read<ui32>( _nbIterations, i );
         core::read<double>( _subsetRatio, i );

         ui32 size;
         core::read<ui32>( size, i );
         // TODO implement constructor from input stream for all classifiers
      }

      /**
       @todo implement
       */
      virtual void write( std::ostream& o ) const
      {
         ensure( _classifiers.size(), "classifier can't be saved if there is no computed classifier(source is lost)" );

         unreachable( "not fully implemented yet" );
         // TODO write the number of classes

         core::write<ui32>( _nbIterations, o );
         core::write<double>( _subsetRatio, o );

         ui32 s = static_cast<ui32>( _classifiers.size() );
         core::write<ui32>( s, o );
         for ( ui32 n = 0; n < s; ++n )
            _classifiers[ n ]->write( o );
         for ( ui32 n = 0; n < s; ++n )
            core::write<double>( _alphas[ n ], o );
      }

      virtual typename Base::Class test( const T& p ) const
      {
         std::vector<double> prob( _nbClasses );
         for ( ui32 n = 0; n < _classifiers.size(); ++n )
         {
            ui32 c = _classifiers[ n ]->test( p );
            prob[ c ] += _alphas[ n ];
            // TODO modif check ie: log( 1 / ( _alphas[ n ] + std::numeric_limits<double>::epsilon() ) );
         }

         ui32 maxIndex = 0;
         double max = INT_MIN;
         for ( ui32 n = 0; n < _nbClasses; ++n )
            if ( prob[ n ] > max )
            {
               maxIndex = n;
               max = prob[ n ];
            }
         ensure( !core::equal<double>( max, INT_MIN ), "error: no max" );
         return maxIndex;
      }

      virtual void learn( const Database& dat, const nll::core::Buffer1D<nll::f64>& parameters )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "starting adaboost(" + core::val2str( _nbIterations ) + ")" );
         _destroy();
         _alphas.clear();

         // select all learning samples
         _nbClasses = core::getNumberOfClass( dat );
         Database learningDat = core::filterDatabase( dat, core::make_vector<ui32>( Database::Sample::LEARNING ), Database::Sample::LEARNING );
         core::Buffer1D<double> proba( learningDat.size() );
         const ui32 samplingSize = static_cast<ui32>( proba.size() * _subsetRatio );
         for ( ui32 n = 0; n < proba.size(); ++n )
            proba[ n ] = 1.0 / proba.size();
         for ( ui32 n = 0; n < _nbIterations; ++n )
         {
            // generate a distribution, learn the classifier
            core::Buffer1D<ui32> samplingIndexes = core::sampling( proba, samplingSize );

            Database ndat;
            for ( ui32 nn = 0; nn < samplingIndexes.size(); ++nn )
               ndat.add( learningDat[ samplingIndexes[ nn ] ] );
            assert( ndat.size() );

            BaseWeakLearner* c = reinterpret_cast<BaseWeakLearner*>( _source->deepCopy() );   // create a new instance, we don't care if it is the same classifier, we only want to have the same init params  so we can learn the database
            assert( c ); // if not we are in trouble!
            c->learn( ndat, parameters );


            // update the weights
            double err = 0;
            ui32 nbError = 0;
            core::Buffer1D<ui32> cclass( learningDat.size() );
            for ( ui32 nn = 0; nn < learningDat.size(); ++nn )
            {
               cclass[ nn ] = c->test( learningDat[ nn ].input );
               err += ( cclass[ nn ] == learningDat[ nn ].output ) ? 0 : ( ++nbError, proba[ nn ] );
            }
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "weak classifier testing error=" + core::val2str( (double)nbError / learningDat.size() ) );
            ensure( nbError < learningDat.size() / 2, "error, error > 0.5" );
            double alpha = 1 / ( err + 0.0001 );
            
            ensure( alpha >= 0, "error" );
            for ( ui32 nn = 0; nn < learningDat.size(); ++nn )
            {
               double coef = ( cclass[ nn ] == learningDat[ nn ].output ) ? alpha : 1;
               proba[ nn ] = proba[ nn ] * coef;
            }

            // normalize so we have a distribution again
            double sum = 0;
            for ( ui32 nn = 0; nn < proba.size(); ++nn )
               sum += proba[ nn ];
            for ( ui32 nn = 0; nn < proba.size(); ++nn )
               proba[ nn ] /= sum;

            // save the classifier
            _classifiers.push_back( c );
            _alphas.push_back( alpha );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "weak classifier weight=" + core::val2str( alpha ) );
         }
      }

   private:
      void _destroy()
      {
         for ( ui32 n = 0; n < _classifiers.size(); ++n )
            delete _classifiers[ n ];
         _classifiers.clear();
      }

   private:
      ui32                          _nbIterations;
      double                        _subsetRatio;
      std::vector<BaseWeakLearner*> _classifiers;
      const Base*                   _source;
      bool                          _ownsSource;
      std::vector<double>           _alphas;
      ui32                          _nbClasses;
   };
}
}

#endif
