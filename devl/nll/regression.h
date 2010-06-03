#ifndef NLL_REGRESSION_H_
# define NLL_REGRESSION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Base class for all regression based algorithms.
   
    As template parameter, any class defining the methods size(), operator[], typedef value_type,
    and a constructor(size) need to be defined.

    TOutput must define 
    */
   template <class TPoint, class TOutput, class TSample = core::ClassificationSample<TPoint, TOutput> >
   class Regression : public ClassifierBase<TPoint, TOutput, TSample>
   {
   public:
      typedef ClassifierBase<TPoint, TOutput, TSample> Base;             // superclass
      typedef Regression<TPoint, TOutput, TSample>     BaseRegression;   // root of the regression bases algorithms

      // import the other functions
      using Base::test;

   public:
      /**
       @brief Contructor of the classifier.
       @param parameters defines the parameter's model (min, max, how to modify it...).
              The actual classifier should check the parameters expected for the classifier during
              learning. It shouldn't be exposed to the public.
       */
      Regression( const ParameterOptimizers& parameters ) : Base( parameters )
      {
         // test output capabilities: if the compilation fails, we are missing something needed
         typename Output::value_type t;
         Output out( 5 );
         out[ 0 ];
         out.size();
      }

      /**
       By default we do a 10-fold cross validation
       */
      Regression() : ClassifierBase()
      {}

      /**
       @brief test a database and return some statistics on the database (supposing we have a dataset splitted in
              <code>TEST|LEARNING|VALIDATION</code> sets).
       */
      virtual Result test( const Database& dat )
      {
		   ui32 nbLearn = 0;
         ui32 nbValidation = 0;

         float nbValidationError = 0;
         float nbLearnError = 0;
		   float nbError = 0;

		   ui32 nbTest = 0;
         std::stringstream o;
		   for (ui32 n = 0; n < dat.size(); ++n)
		   {
            if ( dat[ n ].type == Database::Sample::LEARNING )
			   {
				   ++nbLearn;
               Output result = test( dat[n].input );
               ensure( result.size() == dat[ n ].output.size(), "error: size doesn't match" );
               nbLearnError += core::generic_norm2<Output, float>( result, dat[ n ].output, static_cast<ui32>( result.size() ) );
			   } 
            if ( dat[ n ].type == Database::Sample::VALIDATION )
			   {
				   ++nbValidation;
               Output result = test( dat[n].input );
               ensure( result.size() == dat[ n ].output.size(), "error: size doesn't match" );
               nbValidationError += core::generic_norm2<Output, float>( result, dat[ n ].output, static_cast<ui32>( result.size() ) );
			   } 
            if ( dat[ n ].type == Database::Sample::TESTING )
            {
				   ++nbTest;
				   Output result = test( dat[n].input );
               ensure( result.size() == dat[ n ].output.size(), "error: size doesn't match" );
               nbError += core::generic_norm2<Output, float>( result, dat[ n ].output, static_cast<ui32>( result.size() ) );
			   }
		   }


   		
         o << "classifier performance:" << std::endl;
	      o << " nb learning=" << nbLearn << std::endl;
	      o << " nb testing=" << nbTest << std::endl;
	      o << " testing error rate:" << static_cast<double>(nbError) / nbTest << std::endl;
         o << " learning error rate:" << static_cast<double>(nbLearnError) / nbLearn << std::endl;
	      o << " sum of testing errors:" << nbError << std::endl;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, o.str() );

         return Result( nbLearn ? ( static_cast<double> ( nbLearnError ) / nbLearn) : -1,
                        nbTest ? ( static_cast<double> ( nbError ) / nbTest ) : -1,
                        nbValidation ? ( static_cast<double> ( nbValidationError ) / nbValidation ) : -1 );
      }

   protected:
      virtual std::vector<ui32> _setCrossFoldBin( const Database& dat, ui32 nbBins ) const
      {
         std::vector<ui32> bins( dat.size() );
         ui32 reminder = dat.size() % nbBins;

         // create the bit count, floor the counts, then add 1 to the first size % nbBins sample
         for ( ui32 n = 0; n < dat.size(); ++n )
            bins[ n ] = n / dat.size() + ( n < reminder );

         // randomize the list
         randomize( bins, 0.8 );
         return bins;
      }
   };
}
}


/*
   //
   // template classifier
   //
   template <class Point>
   class RegressionTest : public nll::algorithm::Regression<Point, Vector>
   {
   public:
      typedef Regression<Point, Vector>  Base;

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
      RegressionTest () : Regression( buildParameters() )
      {}
      virtual RegressionTest * deepCopy() const
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

      virtual Output test( const Point& p ) const
      {
         return 0;
      }

      virtual void learn( const Database& dat, const nll::core::Buffer1D<nll::f64>& parameters )
      {
      }
   };
*/

#endif
