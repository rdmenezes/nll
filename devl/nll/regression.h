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

      // gcc...
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Output                   Output;

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
      Regression() : Base()
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
         core::shuffleFisherYates( bins );
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
