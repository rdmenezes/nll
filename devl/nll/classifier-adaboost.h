#ifndef NLL_CLASSIFIER_ADABOOST_H_
# define NLL_CLASSIFIER_ADABOOST_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief A factory for creating a spacific classifier
    */
   template <class T, template <typename> class WeakClassifier>
   class FactoryClassifier
   {
   public:
      virtual WeakClassifier<T>* create() const = 0;
   };

   template <class T>
   class FactoryClassifierMlp : public FactoryClassifier<T, ClassifierMlp>
   {
   public:
      virtual ClassifierMlp<T>* create() const
      {
         return new ClassifierMlp<T>();
      }
   };



   template <class T, template <typename> class WeakClassifier>
   class ClassifierAdaboostM1 : public Classifier<T>
   {
      typedef WeakClassifier<T>        BaseWeakLearner;
      typedef Classifier<T>            Base;

   public:
      typedef typename Base::Database  Database;

      struct WeakClassifierTest
      {
         WeakClassifierTest( BaseWeakLearner* c, double a ) : classifier( c ), alpha( a )
         {}

         double            alpha;
         BaseWeakLearner*  classifier;
      };

      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

      typedef FactoryClassifier< T, WeakClassifier >  Factory;

   public:
      static nll::algorithm::ParameterOptimizers buildParameters()
      {
         nll::algorithm::ParameterOptimizers parameters;
         return parameters;
      }

   public:
      ClassifierAdaboostM1( const Factory& factory, ui32 nbWeakLearner, f64 learningSubsetRate = 0.5 ) : Base( buildParameters() ), _factory( factory ), _nbWeakLearner( nbWeakLearner ), _learningSubsetRate( learningSubsetRate )
      {
      }

      ~ClassifierAdaboostM1()
      {
         for ( ui32 n = 0; n < _weakClassifiers.size(); ++n )
            delete _weakClassifiers[ n ].classifier;
      }

      virtual Base* deepCopy() const
      {
         ClassifierAdaboostM1* c = new ClassifierAdaboostM1( _factory, _nbWeakLearner, _learningSubsetRate );
         c->_crossValidationBin = _crossValidationBin;
         c->_learningSubsetRate = _learningSubsetRate;
         for ( ui32 n = 0; n < (ui32)_weakClassifiers.size(); ++n )
         {
            BaseWeakLearner* l = dynamic_cast<BaseWeakLearner*>( _weakClassifiers[ n ].classifier->deepCopy() );
            c->_weakClassifiers.push_back( WeakClassifierTest( l, _weakClassifiers[ n ].alpha ) );
         }
         return c;
      }

      virtual void read( std::istream& o )
      {
         ui32 size = 0;
         core::read<ui32>( size, o );
         for ( ui32 n = 0; n < size; ++n )
         {
            f64 alpha = 0;
            core::read<f64>( alpha, o );
            BaseWeakLearner* weak = _factory.create();
            weak->read( o );
            _weakClassifiers.push_back( WeakClassifierTest( weak, alpha ) );
         }
      }

      virtual void write( std::ostream& o ) const
      {
         ui32 size = static_cast<ui32>( _weakClassifiers.size() );
         core::write<ui32>( size, o );
         for ( ui32 n = 0; n < size; ++n )
         {
            core::write<f64>( _weakClassifiers[ n ].alpha, o );
            _weakClassifiers[ n ].classifier->write( o );
         }
      }

      virtual typename Base::Class test( const T& p ) const
      {
         core::Buffer1D<double> prob( 2 );
         for ( ui32 n = 0; n < _weakClassifiers.size(); ++n )
         {
            Output t = _weakClassifiers[ n ].classifier->test( p );
            prob[ t ] += _weakClassifiers[ n ].alpha;
         }

         return prob[ 0 ] < prob[ 1 ];
      }

      /**
       @brief Learn the database.
       @note Suitable only for 2 class database

       We are expecting as parameters:
       -> the parameter of the weak classifier
       */
      virtual void learn( const Database& dat, const nll::core::Buffer1D<nll::f64>& parameters )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "ClassifierAdaboostM1::learn()" );
         ui32 nbClass = core::getNumberOfClass( dat );
         ensure(  nbClass == 2, "Adaboost M1 is only for binary classification problem" );
         //ensure( parameters.size() == this->_parametersPrototype.size(), "Incorrect parameters." );

         // get the LEARNING sample only
         Database learning = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );

         // train the classifiers
         core::Buffer1D<double>  distribution( learning.size() );
         for ( ui32 n = 0; n < learning.size(); ++n )
            distribution[ n ] = 1.0 / learning.size();

         ui32 learningSubsetSize = static_cast<ui32>( _learningSubsetRate * learning.size() );
         for ( ui32 n = 0; n < _nbWeakLearner; ++n )
         {
            // generate a distribution
            Database subset;

            // TODO: problem if sampling doesn't sample all the classes?
            core::Buffer1D<ui32> samplingIndexes = core::sampling( distribution, learningSubsetSize );
            for ( ui32 nn = 0; nn < learningSubsetSize; ++nn )
            {
               subset.add( learning[ samplingIndexes[ nn ] ] );
            }
            ensure( core::getNumberOfClass( subset ) == 2, "the sampling did not sample all the classes..." );

            // generate a weak classifier and test
            BaseWeakLearner* weak = _factory.create();
            weak->learn( subset, parameters );

            std::vector<Output> res( subset.size() );
            ui32 nbErrors = 0;
            for ( ui32 nn = 0; nn < subset.size(); ++nn )
            {
               res[ nn ] = weak->test( subset[ nn ].input );
               if ( res[ nn ] != subset[ nn ].output )
                  ++nbErrors;
            }
            double eps = static_cast<double>( nbErrors ) / subset.size();

            std::stringstream ss;
            ss << "weak classifier:" << n << " learning error rate=" << eps;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );

            ensure( eps < 0.5, "must be < 0.5" );
            double alpha_t = 0.5 * log( ( 1.0 - eps ) / ( eps + 1e-4 ) ) / log( 10.0 );

            // update the distribution
            for ( ui32 nn = 0; nn < samplingIndexes.size(); ++nn )
            {
               if ( res[ nn ] != subset[ nn ].output )
               {
                  distribution[ samplingIndexes[ nn ] ] *= exp( alpha_t );
               } else {
                  distribution[ samplingIndexes[ nn ] ] *= exp( -alpha_t );
               }
            }

            double sum = 0;
            for ( ui32 nn = 0; nn < distribution.size(); ++nn )
            {
               sum += distribution[ nn ];
            }

            // renormalize the distribution
            ensure( sum > 0, "must be > 0" );
            for ( ui32 nn = 0; nn < distribution.size(); ++nn )
            {
               distribution[ nn ] /= sum;
            }

            _weakClassifiers.push_back( WeakClassifierTest( weak, alpha_t ) );
         }
      }

   private:
         // copy disabled
         ClassifierAdaboostM1& operator=( const ClassifierAdaboostM1& );
         ClassifierAdaboostM1( const ClassifierAdaboostM1& );

   private:
      const Factory&                   _factory;
      ui32                             _nbWeakLearner;
      f64                              _learningSubsetRate;
      std::vector<WeakClassifierTest>  _weakClassifiers;
   };
}
}

#endif
