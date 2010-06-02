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
   template <class TPoint, class Output = ui32, class TSample = core::ClassificationSample<TPoint, Output> >
   class Classifier : public ClassifierBase<TPoint, Output, TSample>
   {
   public:
      typedef ClassifierBase<TPoint, Output, TSample> Base;             // superclass
      typedef Classifier<TPoint, Output, TSample>     BaseClassifier;   // root of the classifiers

      // import the other functions
      using Base::test;

   public:
      /**
       @brief Contructor of the classifier.
       @param parameters defines the parameter's model (min, max, how to modify it...).
              The actual classifier should check the parameters expected for the classifier during
              learning. It shouldn't be exposed to the public.
       */
      Classifier( const ParameterOptimizers& parameters ) : Base( parameters )
      {}

      /**
       By default we do a 10-fold cross validation
       */
      Classifier() : ClassifierBase()
      {}

      /**
       @brief test a database and return some statistics on the database (supposing we have a dataset splitted in
              <code>TEST|LEARNING|VALIDATION</code> sets).
       */
      virtual Result test( const Database& dat )
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
         c->_crossValidationBin = _crossValidationBin;
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
