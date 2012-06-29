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

   /**
    @brief Factory creating a new instance of a MLP classifier
    */
   template <class T>
   class FactoryClassifierMlp : public FactoryClassifier<T, ClassifierMlp>
   {
   public:
      virtual ClassifierMlp<T>* create() const
      {
         return new ClassifierMlp<T>();
      }
   };


   /**
    @brief basic Adaboost implementation for binary classification only

    This version of the algorithm is using resampling of the training data instead of the original version
    (i.e., a specific weak learner using the Dt distribution must be used to weight the learner)

    @note see http://www.site.uottawa.ca/~stan/csi5387/boost-tut-ppr.pdf for implementation details
          see http://www.face-rec.org/algorithms/Boosting-Ensemble/decision-theoretic_generalization.pdf for full proof of the algorihm
    @param WeakClassifier it must be an instance of <code>Classifier</code>
    */
   template <class T, template <typename> class WeakClassifier>
   class ClassifierAdaboost : public Classifier<T>
   {
      typedef WeakClassifier<T>        BaseWeakLearner;
      typedef Classifier<T>            Base;

      struct WeakClassifierTest
      {
         WeakClassifierTest( BaseWeakLearner* c, double a ) : classifier( c ), alpha( a )
         {}

         double            alpha;
         BaseWeakLearner*  classifier;
      };

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

      // for gcc...
      typedef typename Base::Point                    Point;
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Class                    Class;

      typedef FactoryClassifier< T, WeakClassifier >  Factory;

   public:
      static nll::algorithm::ParameterOptimizers buildParameters()
      {
         nll::algorithm::ParameterOptimizers parameters;
         return parameters;
      }

   public:
      /**
       @param factory the factory to create a new weak classifier, note that only a reference is taken and must be kept alive
       @param nbWeakLearner the number of weak learner that will be created for the strong classifier
       @param learningSubsetRate the size of learning database for each weak classifier
       */
      ClassifierAdaboost( const Factory& factory, size_t nbWeakLearner, f64 learningSubsetRate = 0.3 ) : Base( buildParameters() ), _factory( factory ), _nbWeakLearner( nbWeakLearner ), _learningSubsetRate( learningSubsetRate )
      {
      }

      ~ClassifierAdaboost()
      {
         for ( size_t n = 0; n < _weakClassifiers.size(); ++n )
            delete _weakClassifiers[ n ].classifier;
      }

      virtual ClassifierAdaboost* deepCopy() const
      {
         ClassifierAdaboost* c = new ClassifierAdaboost( _factory, _nbWeakLearner, _learningSubsetRate );
         c->_crossValidationBin = this->_crossValidationBin;
         c->_learningSubsetRate = this->_learningSubsetRate;
         for ( size_t n = 0; n < (size_t)_weakClassifiers.size(); ++n )
         {
            BaseWeakLearner* l = dynamic_cast<BaseWeakLearner*>( _weakClassifiers[ n ].classifier->deepCopy() );
            c->_weakClassifiers.push_back( WeakClassifierTest( l, _weakClassifiers[ n ].alpha ) );
         }
         return c;
      }

      virtual void read( std::istream& o )
      {
         size_t size = 0;
         core::read<size_t>( size, o );
         for ( size_t n = 0; n < size; ++n )
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
         size_t size = static_cast<size_t>( _weakClassifiers.size() );
         core::write<size_t>( size, o );
         for ( size_t n = 0; n < size; ++n )
         {
            core::write<f64>( _weakClassifiers[ n ].alpha, o );
            _weakClassifiers[ n ].classifier->write( o );
         }
      }

      virtual Class test( const T& p ) const
      {
         core::Buffer1D<double> prob( 2 );
         for ( size_t n = 0; n < _weakClassifiers.size(); ++n )
         {
            Class t = _weakClassifiers[ n ].classifier->test( p );
            ensure( t < 2, "Adaboost handles only binary decision problems" );
            prob[ t ] += _weakClassifiers[ n ].alpha;
         }

         if ( prob[ 0 ] < prob[ 1 ] )
         {
            return 1;
         } else {
            return 0;
         }
      }

      /**
       @brief Learn the database.
       @note Suitable only for 2 class database

       We are expecting as parameters:
       -> the parameter of the weak classifier
       */
      virtual void learn( const Database& dat, const nll::core::Buffer1D<nll::f64>& parameters )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "ClassifierAdaboost::learn()" );
         size_t nbClass = core::getNumberOfClass( dat );
         ensure(  nbClass == 2, "basic Adaboost is only for binary classification problem" );

         // get the LEARNING sample only
         Database learning = core::filterDatabase( dat, core::make_vector<size_t>( (size_t) Database::Sample::LEARNING ), (size_t) Database::Sample::LEARNING );

         // train the classifiers
         core::Buffer1D<double>  distribution( learning.size() );
         for ( size_t n = 0; n < learning.size(); ++n )
            distribution[ n ] = 1.0 / learning.size();

         size_t learningSubsetSize = static_cast<size_t>( _learningSubsetRate * learning.size() );
         for ( size_t n = 0; n < _nbWeakLearner; ++n )
         {
            // generate a distribution
            Database subset;

            size_t nbClassesSampled = 0;
            core::Buffer1D<size_t> samplingIndexes;
            while ( nbClassesSampled != 2 ) // handle the case where the sampling doesn't have the two classes
            {
               samplingIndexes = core::sampling( distribution, learningSubsetSize );
               for ( size_t nn = 0; nn < learningSubsetSize; ++nn )
               {
                  subset.add( learning[ samplingIndexes[ nn ] ] );
               }
               nbClassesSampled = core::getNumberOfClass( subset );
            }

            // generate a weak classifier and test
            BaseWeakLearner* weak = _factory.create();
            weak->learn( subset, parameters );

            std::vector<Class> res( subset.size() );
            double eps = 0;
            for ( size_t nn = 0; nn < subset.size(); ++nn )
            {
               res[ nn ] = weak->test( subset[ nn ].input );
               if ( res[ nn ] != subset[ nn ].output )
               {
                  eps += distribution[ samplingIndexes[ nn ] ];
               }
            }

            std::stringstream ss;
            ss << "weak classifier:" << n << " learning error rate=" << eps;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );

            if ( eps < 0.5 )
            {
               double alpha_t = 0.5 * core::log2( ( 1.0 - eps ) / ( eps + 1e-4 ) );

               // update the distribution
               for ( size_t nn = 0; nn < samplingIndexes.size(); ++nn )
               {
                  if ( res[ nn ] != subset[ nn ].output )
                  {
                     distribution[ samplingIndexes[ nn ] ] *= exp( alpha_t );
                  } else {
                     distribution[ samplingIndexes[ nn ] ] *= exp( -alpha_t );
                  }
               }

               // renormalize the distribution
               const double sum = std::accumulate( distribution.begin(), distribution.end(), 0.0 );
               ensure( sum > 0, "must be > 0" );
               for ( size_t nn = 0; nn < distribution.size(); ++nn )
               {
                  distribution[ nn ] /= sum;
               }

               _weakClassifiers.push_back( WeakClassifierTest( weak, alpha_t ) );
            } else {
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "weak classifier's error is too large, reduced to=" + core::val2str( n ) );
               continue;
            }
         }
      }

   private:
         // copy disabled
         ClassifierAdaboost& operator=( const ClassifierAdaboost& );
         ClassifierAdaboost( const ClassifierAdaboost& );

   private:
      const Factory&                   _factory;
      size_t                             _nbWeakLearner;
      f64                              _learningSubsetRate;
      std::vector<WeakClassifierTest>  _weakClassifiers;
   };
}
}

#endif
