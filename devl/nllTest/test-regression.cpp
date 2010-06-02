#include "stdafx.h"
#include <nll/nll.h>

using namespace nll::core;
using namespace nll::algorithm;


class DummyRegression : public Regression< Buffer1D<double>, Buffer1D<double> >
{
public:
   typedef Regression< Buffer1D<double>, Buffer1D<double> >  Base;

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

      DummyRegression() : Regression( buildParameters() )
      {}

      virtual DummyRegression* deepCopy() const
      {
         return 0;
      }

      virtual void read( std::istream& )
      {
      }

      virtual void write( std::ostream& ) const
      {
      }

      virtual Output test( const Point& input ) const
      {
         Buffer1D<double> result( 1 );
         result[ 0 ] = input[ 0 ];
         return result;
      }

      virtual void learn( const Database& , const nll::core::Buffer1D<nll::f64>& )
      {
      }
};

class TestRegression
{
public:
   void testBasic()
   {
      DummyRegression regression;

      DummyRegression::Database dat;
      dat.add( DummyRegression::Database::Sample( make_buffer1D<double>( 1 ), make_buffer1D<double>( 1 ), DummyRegression::Database::Sample::LEARNING ) );

      dat.add( DummyRegression::Database::Sample( make_buffer1D<double>( 1 ), make_buffer1D<double>( 2 ), DummyRegression::Database::Sample::TESTING ) );
      dat.add( DummyRegression::Database::Sample( make_buffer1D<double>( 2 ), make_buffer1D<double>( 3 ), DummyRegression::Database::Sample::TESTING ) );

      dat.add( DummyRegression::Database::Sample( make_buffer1D<double>( 3 ), make_buffer1D<double>( 6 ), DummyRegression::Database::Sample::VALIDATION ) );
      dat.add( DummyRegression::Database::Sample( make_buffer1D<double>( 4 ), make_buffer1D<double>( 8 ), DummyRegression::Database::Sample::VALIDATION ) );
      dat.add( DummyRegression::Database::Sample( make_buffer1D<double>( 5 ), make_buffer1D<double>( 10 ), DummyRegression::Database::Sample::VALIDATION ) );

      DummyRegression::Result r = regression.test( dat );
      TESTER_ASSERT( equal<double>( r.learningError, 0 ) );
      TESTER_ASSERT( equal<double>( r.testingError, 1.0 ) );
      TESTER_ASSERT( equal<double>( r.validationError, 4.0 ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRegression);
TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif
