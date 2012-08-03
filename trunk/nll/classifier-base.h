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

#ifndef NLL_CLASSIFIER_BASE_H_
# define NLL_CLASSIFIER_BASE_H_

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression is constant

namespace nll
{
namespace algorithm
{
   /**
    @brief Base class for Classification and regression based algorithms
    */
   template <class TPoint, class TOutput, class TSample = core::ClassificationSample<TPoint, TOutput> >
   class ClassifierBase
   {
   public:
      typedef TPoint                                     Point;
      typedef TOutput                                    Output;
      typedef core::Buffer1D<f64>                        ClassifierParameters;

      typedef TSample                                    Sample;
      typedef core::Database<Sample>                     Database;

      /**
       @brief Define the accuracy of the classifier.
              error rate = -1 if not associated data
      */
      struct Result
      {
         Result() : learningError( -1 ), testingError( -1 ), validationError( -1 ){}
         Result( double tr, double te, double va ) : learningError( tr ), testingError( te ), validationError( va ){}

         double   learningError;
         double   testingError;
         double   validationError;
      };

      /**
       @brief Define a helper function to create a classifier linked with a database on the fly so the classifier
              can be optimized by any optimizer.
       */
      class OptimizerClientClassifier : public OptimizerClient
      {
      public:
         OptimizerClientClassifier( const ClassifierBase* classifier, const Database& database ) : _classifier( classifier ), _database( database ){}
         double evaluate( const ClassifierParameters& parameters ) const
         {
            // we are minimizing the learning error
            return _classifier->evaluate( parameters, _database );
         }

      protected:
         OptimizerClientClassifier& operator=( const OptimizerClientClassifier& );

      protected:
         const ClassifierBase*   _classifier;
         const Database&         _database;
      };

   public:
      ClassifierBase( const ParameterOptimizers& parameters ) : _parametersPrototype( parameters ), _crossValidationBin( 10 )
      {}

      ClassifierBase()  : _crossValidationBin( 10 )
      {}

      virtual ~ClassifierBase()
      {}


      /**
        @brief create a deepcopy of the learner. The returned pointer should be deleted by <code>delete</code>.
               the <code>_crossValidationBin</code> should be copied accross classifiers (else default 10-fold will be used)
        */
      virtual ClassifierBase* deepCopy() const = 0;

      /**
        @brief read the learner's data from a stream
        */
      virtual void read( std::istream& i ) = 0;

      /**
        @brief write the learner's data to a stream
        */
      virtual void write( std::ostream& o ) const = 0;

      /**
        @return the class of a point
        */
      virtual Output test( const Point& p ) const = 0;

      /**
        @brief function used to evaluate classifier performance. It is used for optimizing
               the classifier's parameters as well as in the <code>Typelist</code> internally. The default
               implementation is simply a 10-fold cross validation on the <code>LEARNING|VALIDATION</code>
               samples. In the case that the classifier is not deterministic (meaning given a database, each
               learner build using this database must be the same), else this method should be reimplemented
               using, for example, several cross validations and return the mean value.

        @return return the score of the classifier. The lesser, the better. Scores accross different kind of classifiers
               MUST be compatible to ensure comparison accross classifiers. The error returned is:
               <code>learningError</code>
        */
      virtual double evaluate( const ClassifierParameters& parameters, const Database& dat ) const
      {
         Result r = test( dat, parameters, _crossValidationBin );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "Classifier::evaluate()=" + core::val2str( r.learningError ) );
         return r.learningError;
      }

      /**
       @brief learning method. Only <code>LEARNING</code> samples in the database should be learnt.

        must implement the learning algorithm:
        - <code>LEARNING</code> : learn these examples
        - skip <code>TESTING</code> samples
        - skip <code>VALIDATION</code> samples (used to compute the classifer's model)

        The learning dataset is only used to learn the dataset.
        The validation dataset is used to tune the parameters of the algorithm, so that the testing
            dataset is never used except for testing.
        */
      virtual void learn( const Database& dat, const ClassifierParameters& parameters ) = 0;

      /**
       @brief Test the database and return statistics
       */
      virtual Result test( const Database& dat ) = 0;

      /**
       @brief Learn the <code>LEARNING|VALIDATION</code> sample
       */
      void learnTrainingDatabase( const Database& dat, const ClassifierParameters& parameters )
      {
         Database datl = nll::core::filterDatabase(
            dat,
            nll::core::make_vector<size_t>( Database::Sample::LEARNING,
                                            Database::Sample::VALIDATION ),
            Database::Sample::LEARNING );
         learn( datl, parameters );
      }

      /**
       @brief Learn the <code>LEARNING|VALIDATION|TESTING</code> sample
       */
      void learnAllDatabase( const Database& dat, const ClassifierParameters& parameters )
      {
         Database datl = nll::core::filterDatabase(
            dat,
            nll::core::make_vector<size_t>( Database::Sample::LEARNING,
                                            Database::Sample::VALIDATION,
                                            Database::Sample::TESTING ),
            Database::Sample::LEARNING );
         learn( datl, parameters );
      }

      /**
       @brief create an optimizer for the classifier. It is used to optimize the parameters's model.
       */
      const OptimizerClientClassifier createOptimizer( const Database& dat ) const { return OptimizerClientClassifier( this, dat ); }

      /**
       @brief test the classifier using a kfold-cross validation. On <code>VALIDATION|LEARNING</code> tags, meaning that the
              <code>TESTING</code> samples are discarded.
              
              It is always required to adjust model's parameters. To avoid
              the need of an indenpendant dataset, cross validation can be used. The learning and validation samples
              are used to make a new database. This database will be splitted in <code>kfold</code> bins. These
              bins are independant and have approximatively the same distribution of classes than the source database.

              The learning accuracy can then be used to optimize some of the parameter's model. The final classifier
              should learn the whole <code>VALIDATION|LEARNING</code> database.

              <code>TESTING</code> samples are discarded so that we are sure the testing dataset is really independant.
       @return Result learningError will be set, the validation and testing are set to -1
       */
      Result test( const Database& dat, const ClassifierParameters& learningParameters, size_t kfold ) const
      {
         ensure( kfold >= 3, "useless to do less than a 3-fold cross validation. Current is:" + core::val2str( kfold ) );
         Result rglobal( 0, -1, -1 );

         // filter the database
         Database learningDatabase;
         for ( size_t n = 0; n < dat.size(); ++n )
            if ( dat[ n ].type == Database::Sample::LEARNING ||
                 dat[ n ].type == Database::Sample::VALIDATION )
               learningDatabase.add( dat[ n ] );

         // set the bins
         std::vector<size_t> bins = _setCrossFoldBin( learningDatabase, kfold );

         std::stringstream o;
         o << "testing: crossvalidation, k=" << kfold << std::endl;
         for ( size_t n = 0; n < kfold; ++n )
         {
            _generateDatabase( learningDatabase, bins, n );
            size_t nbLearning;
            size_t nbValidation;
            size_t nbTesting;
            getInfo( learningDatabase, nbLearning, nbTesting, nbValidation );
            ClassifierBase* newc = deepCopy();
            newc->learn( learningDatabase, learningParameters );
            Result res = newc->test( learningDatabase );
            rglobal.learningError += res.testingError * ( nbTesting ); // get the number of errors, not the ratio!
            delete newc;

            o << " cross-validation subset=" << n << " error:" << res.testingError << std::endl;
         }

         rglobal.learningError /= learningDatabase.size();
         o << "cross-validation result:" << std::endl;
         o << "  testing error=" << rglobal.learningError << std::endl;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, o.str() );
         return rglobal;
      }

      /**
       @brief read the classifier from a file
       */
      virtual void read( const std::string& file )
      {
         std::ifstream f( file.c_str(), std::ios_base::binary );
         if ( !f.is_open() )
            return;
         _crossValidationBin = 10;
         read( f );
      }

      /**
       @brief write the classifier to a file.
       */
      virtual void write( const std::string& file ) const
      {
         std::ofstream f( file.c_str(), std::ios_base::binary );
         if ( !f.is_open() )
            return;
         write( f );
      }

      /**
       @brief set the number of bins for the cross validation
       */
      void setCrossValidationBinSize( size_t numberOfBins )
      {
         _crossValidationBin = numberOfBins;
      }

      /**
        @brief Returns information on the database.
        */
      static void getInfo( const Database& dat, size_t& out_nb_learning, size_t& out_nb_testing, size_t& out_nb_validating )
      {
         out_nb_learning   = 0;
         out_nb_testing    = 0;
         out_nb_validating = 0;

         for ( size_t n = 0; n < dat.size(); ++n )
         {
            switch( dat[n].type )
            {
            case Sample::LEARNING:
               ++out_nb_learning;
               break;
            case Sample::TESTING:
               ++out_nb_testing;
               break;
            case Sample::VALIDATION:
               ++out_nb_validating;
               break;
            default:
               ensure(0, "unreachable"); // error
            }
         }
      }

   protected:
      // copy disabled
      ClassifierBase( const ClassifierBase& );
      ClassifierBase& operator=( const ClassifierBase& );

   protected:
      // set a bin ID for each datasets
      virtual std::vector<size_t> _setCrossFoldBin( const Database& dat, size_t nbBins ) const = 0;

      // generate a database for crossvalidation : currentBin == bins[n] => n testing, else n learning example
      void _generateDatabase( Database& dat, const std::vector<size_t>& bins, size_t currentBin ) const
      {
         assert( bins.size() == dat.size() );

         for ( size_t n = 0; n < dat.size(); ++n )
            if ( bins[ n ] == currentBin )
               dat[ n ].type = Database::Sample::TESTING;
            else
               dat[ n ].type = Database::Sample::LEARNING;
      }

   protected:
      const ParameterOptimizers  _parametersPrototype;
      size_t                       _crossValidationBin;
   };
}
}

# pragma warning( pop )

#endif