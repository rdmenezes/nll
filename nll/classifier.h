#ifndef NLL_CLASSIFIER_H_
# define NLL_CLASSIFIER_H_

# include <iostream>
# include <fstream>
# include <set>
# include <map>
# include "types.h"
# include "database.h"

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Base class for all classifiers. ensure ALL classification sample ID start at 0, and are contiguous.
   
    Base class for all the classifiers.
    As template parameter, any class defining the methods size(), operator[], typedef value_type,
    and a constructor(size) need to be defined.

    This base class defines the main operations (validation, testing, learning, database) as well as different optimizers could
    be used to find the best learning parameters.
    */
   template <class TPoint, class Output = ui32>
   class Classifier
   {
   public:
      typedef TPoint                                     Point;
      typedef Output                                     Class;
      typedef core::ClassificationSample<Point, Class>   Sample;
      typedef core::Database<Sample>                     Database;
      typedef Classifier<Point>                          BaseClassifier;
      typedef core::Buffer1D<f64>                        ClassifierParameters;

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
         OptimizerClientClassifier( const Classifier* classifier, const Database& database ) : _classifier( classifier ), _database( database ){}
         double evaluate( const ClassifierParameters& parameters ) const
         {
            return _classifier->evaluate( parameters, _database );
         }

      protected:
         OptimizerClientClassifier& operator=( const OptimizerClientClassifier& );

      protected:
         const Classifier*       _classifier;
         const Database&         _database;
      };

   public:
      /**
        @brief Returns information on the database.
        */
      static void getInfo( const Database& dat, ui32& out_nb_learning, ui32& out_nb_testing, ui32& out_nb_validating )
      {
         out_nb_learning   = 0;
         out_nb_testing    = 0;
         out_nb_validating = 0;

         for ( ui32 n = 0; n < dat.size(); ++n )
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
               assert(0); // error
            }
      }

   public:
      /**
        @brief create a deepcopy of the learner. The returned pointer should be deleted by <code>delete</code>.
        */
      virtual Classifier* deepCopy() const = 0;

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
      virtual Class test( const Point& p ) const = 0;

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
         Result r = test( dat, parameters, 10 );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "Classifier::evaluate()=" + core::val2str( r.learningError ) );
         return r.learningError;
      }


      /**
       @brief learning method. Only <code>LEARNING</code> samples in the database should be learnt.

        must implement the learning algorithm:
        - <code>LEARNING</code> : learn these examples
        - skip <code>TESTING</code> samples
        - skip <code>VALIDATION</code> samples (used to compute the classifer's model)

        The learning dataset is only used for learning the dataset.
        The validation dataset is used to tune the parameters of the algorithm, so that the testing
            dataset is never used except for testing.
        */
      virtual void learn( const Database& dat, const ClassifierParameters& parameters ) = 0;

   protected:
      Classifier( const Classifier& );
      Classifier& operator=( const Classifier& );

   public:
      /**
       @brief Contructor of the classifier.
       @param parameters defines the parameter's model (min, max, how to modify it...).
              The actual classifier should check the parameters expected for the classifier during
              learning. It shouldn't be exposed to the public.
       */
      Classifier( const ParameterOptimizers& parameters ) : _parametersPrototype( parameters )
      {}

      virtual ~Classifier()
      {}

      /**
       @brief read the classifier from a file
       */
      void read( const std::string& file )
      {
         std::ifstream f( file.c_str(), std::ios_base::binary );
         if ( !f.is_open() )
            return;
         read( f );
      }

      /**
       @brief write the classifier to a file.
       */
      void write( const std::string& file ) const
      {
         std::ofstream f( file.c_str(), std::ios_base::binary );
         if ( !f.is_open() )
            return;
         write( f );
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
      Result test( const Database& dat, const ClassifierParameters& learningParameters, ui32 kfold ) const
      {
         ensure( kfold >= 3, "useless to do less than a 3-fold cross validation" );
         Result rglobal( 0, -1, -1 );

         // filter the database
         Database learningDatabase;
         for ( ui32 n = 0; n < dat.size(); ++n )
            if ( dat[ n ].type == Database::Sample::LEARNING ||
                 dat[ n ].type == Database::Sample::VALIDATION )
               learningDatabase.add( dat[ n ] );

         // compute statistics
         ui32 nbClass = core::getNumberOfClass( learningDatabase );
         std::vector<ui32> nbSamplesPerClass( nbClass );
         for ( ui32 n = 0; n < learningDatabase.size(); ++n )
            ++nbSamplesPerClass[ learningDatabase[ n ].output ];

         // set the bins
         std::vector<ui32> bins = _setCrossFoldBin( learningDatabase, kfold, nbSamplesPerClass );

         std::stringstream o;
         o << "testing: crossvalidation, k=" << kfold << std::endl;
         for ( ui32 n = 0; n < kfold; ++n )
         {
            _generateDatabase( learningDatabase, bins, n );
            ui32 nbLearning;
            ui32 nbValidation;
            ui32 nbTesting;
            getInfo( learningDatabase, nbLearning, nbTesting, nbValidation );
            Classifier* newc = deepCopy();
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
       @brief Learn the <code>LEARNING|VALIDATION</code> sample
       */
      void learnTrainingDatabase( const Database& dat, const ClassifierParameters& parameters )
      {
         typename Classifier::Database datl = nll::core::filterDatabase(
            dat,
            nll::core::make_vector<nll::ui32>( Classifier::Database::Sample::LEARNING,
                                               Classifier::Database::Sample::VALIDATION ),
            Classifier::Database::Sample::LEARNING );
         learn( datl, parameters );
      }

      /**
       @brief test a database and return some statistics on the database (supposing we have a dataset splitted in
              <code>TEST|LEARNING|VALIDATION</code> sets).
       */
      Result test( const Database& dat )
      {
         std::map<ui32, ui32> cls;
		   std::map<ui32, ui32> ncls;
		   ui32 nbLearn = 0;
         ui32 nbLearnError = 0;
         ui32 nbValidation = 0;
         ui32 nbValidationError = 0;
		   ui32 nbError = 0;
		   ui32 nbUnknown = 0;
		   ui32 nbTest = 0;
         std::stringstream o;
		   for (ui32 n = 0; n < dat.size(); ++n)
		   {
			   cls[ dat[ n ].output ];	// create an empty entry if 0 error
            if ( dat[ n ].type == Database::Sample::LEARNING )
			   {
				   ++nbLearn;
               Class result = test( dat[n].input );
               if ( result != dat[n].output )
				   {
                  if ( dat[ n ].debug.getBuf() )
					      o << "error learning:" << dat[ n ].debug.getBuf() << " expected:" << dat[ n ].output << " found:" << result << std::endl;
					   ++nbLearnError;
				   }
			   } 
            if ( dat[ n ].type == Database::Sample::VALIDATION )
			   {
				   ++nbValidation;
               Class result = test( dat[n].input );
               if ( result != dat[n].output )
				   {
                  if ( dat[ n ].debug.getBuf() )
					      o << "error validation:" << dat[ n ].debug.getBuf() << " expected:" << dat[ n ].output << " found:" << result << std::endl;
					   ++nbValidationError;
				   }
			   } 
            if ( dat[ n ].type == Database::Sample::TESTING )
            {
				   ++nbTest;
               ++ncls[ dat[ n ].output ];
				   Class result = test( dat[n].input );
				   if ( result != dat[n].output )
				   {
                  if ( dat[ n ].debug.getBuf() )
					      o << "error testing:" << dat[ n ].debug.getBuf() << " expected:" << dat[ n ].output << " found:" << result << std::endl;
					   ++nbError;
					   ++cls[ dat[ n ].output ];
				   }
				   if ( result == -1 )
					   ++nbUnknown;
			   }
		   }

	      o << "error by class (testing):" << std::endl;
	      for (std::map<ui32, ui32>::const_iterator it = cls.begin(); it != cls.end(); ++it)
	      {
		         o << " class:" << it->first << " error:" << it->second << " rate:" << static_cast<double>( it->second ) / nbTest << std::endl;
	      }

	      o << "class by error (testing):" << std::endl;
	      for (std::map<ui32, ui32>::const_iterator it = cls.begin(); it != cls.end(); ++it)
	      {
		      std::map<ui32, ui32>::iterator res = ncls.find(it->first);
		      if (res != ncls.end())
		      {
			      ui32 nbinclass = res->second;
			      o << " class:" << it->first << " error:" << it->second << " nbInClass:" << nbinclass << " rate:" << static_cast<double>(it->second) / nbinclass << std::endl;
		      }
	      }
   		
         o << "classifier performance:" << std::endl;
	      o << " nb learning=" << nbLearn << std::endl;
	      o << " nb testing=" << nbTest << std::endl;
	      o << " testing error rate:" << static_cast<double>(nbError) / nbTest << std::endl;
         o << " learning error rate:" << static_cast<double>(nbLearnError) / nbLearn << std::endl;
	      o << " nb errors:" << nbError << std::endl;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, o.str() );

         return Result( nbLearn ? ( static_cast<double> ( nbLearnError ) / nbLearn) : -1,
                        nbTest ? ( static_cast<double> ( nbError ) / nbTest ) : -1,
                        nbValidation ? ( static_cast<double> ( nbValidationError ) / nbValidation ) : -1 );
      }

   protected:
      // generate nbBins bins out of the database. Each bin is drawing the same distribution as database's class
      std::vector<ui32> _setCrossFoldBin( const Database& dat, ui32 nbBins, const std::vector<ui32>& nbSamplesPerClass ) const
      {
         // create the container and init all the sample to the last bin
         std::vector<ui32> bins( dat.size() );
         for ( ui32 n = 0; n < dat.size(); ++n )
            bins[ n ] = nbBins - 1;

         // init the stat
         std::vector< std::vector<ui32> > stat( nbBins - 1 );
         for ( ui32 n = 0; n < nbBins - 1; ++n )
            stat[ n ] = std::vector<ui32>( nbSamplesPerClass.size() );

         // allocate a bin number for all samples. Only ( nbBins - 1 ) first bins.
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            ui32 c = dat[ n ].output;
            // round to the lowest one
            double threshold = core::round<double>( nbSamplesPerClass[ c ] / nbBins, 0.1 );
            std::vector<ui32> list = core::generateUniqueList( 0, nbBins - 2 );
            for ( ui32 nn = 0; nn < nbBins - 1; ++nn )
            {
               ui32 index = list[ nn ];
               if ( stat[ index ][ c ] <= threshold )
               {
                  bins[ n ] = index;
                  ++stat[ index ][ c ];
                  break;
               }
            }
         }
         return bins;
      }

      // generate a database for crossvalidation : currentBin == bins[n] => n testing, else n learning example
      void _generateDatabase( Database& dat, const std::vector<ui32>& bins, ui32 currentBin ) const
      {
         assert( bins.size() == dat.size() );
         Database newdat;
         for ( ui32 n = 0; n < dat.size(); ++n )
            if ( bins[ n ] == currentBin )
               dat[ n ].type = Database::Sample::TESTING;
            else
               dat[ n ].type = Database::Sample::LEARNING;
      }
      
   protected:
      const ParameterOptimizers _parametersPrototype;
   };
}
}


/*
   //
   // template classifier
   //
   template <class Point>
   class ClassifierTest : public nll::algorithm::Classifier<Point>
   {
   public:
      typedef Classifier<Point>  Base;

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
      ClassifierTest() : Classifier( buildParameters() )
      {}
      virtual Classifier* deepCopy() const
      {
         return 0;
      }

      virtual void read( std::istream& i )
      {
      }

      virtual void write( std::ostream& o ) const
      {
      }

      virtual Class test( const Point& p ) const
      {
         return 0;
      }

      virtual void learn( const Database& dat, const nll::core::Buffer1D<nll::f64>& parameters )
      {
      }
   };
*/

#endif
