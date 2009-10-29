#include "stdafx.h"
#include <nll/nll.h>
#include <iostream>

struct ackley : public nll::algorithm::OptimizerClient
{
   ackley( double a = 20,
           double b = 0.01,
           double c = nll::core::PI / 10 )
   {
      _a = a;
      _b = b;
      _c = c;
   }

   virtual double evaluate( const nll::core::Buffer1D<nll::f64>& parameters ) const
   {
      double ninv = 1/(double)parameters.size();
   
      double sum1 = 0;
      double sum2 = 0;
      for( unsigned i = 0; i < parameters.size(); ++i )
      {
         sum1 += parameters[ i ] * parameters[ i ];
         sum2 += cos( _c * parameters[ i ] );
      }
      double result = -_a * exp( -_b * std::sqrt( ninv * sum1 ) )
            - exp( ninv * sum2 ) + _a + exp(1.0);
      return result;
   }


   double operator()( double v ) const
   {
      return evaluate( nll::core::make_buffer1D<double>( v ) );
   }

   double operator()( const nll::core::Buffer1D<double>& v ) const
   {
      return evaluate( v );
   }

private:
   double _a;
   double _b;
   double _c;
};

struct TestBracketing
{
   void test()
   {
      ackley func1;
      nll::algorithm::BracketingResult res = nll::algorithm::minimumBracketing( 0, 500, func1 );
      TESTER_ASSERT( res.cx < res.bx );
      TESTER_ASSERT( res.bx < res.ax );
      TESTER_ASSERT( res.fa >= res.fb );
      TESTER_ASSERT( res.fc >= res.fb );
   }

   void testBrent()
   {
      ackley func1;
      double x;
      bool error;
      double val = nll::algorithm::brent( -50, -49, 50, func1, x, 0.001, 100, &error );
      TESTER_ASSERT( fabs( val ) < 1e-10 );
      TESTER_ASSERT( fabs( x ) < 1e-10 );
   }

   void testLineMinimization()
   {
      ackley func1;

      nll::core::Buffer1D<double> direction( 1 );
      direction[ 0 ] = 1;
      double val;
      nll::core::Buffer1D<double> p = nll::core::make_buffer1D<double>( -10 );
      bool error = nll::algorithm::lineMinimization( p, direction, val, func1, 1e-4 );
      TESTER_ASSERT( fabs( val ) < 1e-4 );
      TESTER_ASSERT( fabs( p[ 0 ] ) < 1e-3 );
      TESTER_ASSERT( !error );
   }

   void testPowell()
   {
      ackley func1;
      bool error;

      nll::core::Buffer1D<double> direction( 1 );
      direction[ 0 ] = 1;

      std::vector< nll::core::Buffer1D<double> > directions;
      directions.push_back( direction );

      double val;
      nll::core::Buffer1D<double> p = nll::core::make_buffer1D<double>( -10 );
      val = nll::algorithm::powell( p, directions, 1e-4, func1, 200, &error );
      TESTER_ASSERT( fabs( val ) < 1e-4 );
      TESTER_ASSERT( fabs( p[ 0 ] ) < 1e-3 );
      TESTER_ASSERT( !error );
   }

   void testPowell2()
   {
      ackley func1;
      bool error;

      std::vector< nll::core::Buffer1D<double> > directions;
      directions.push_back( nll::core::make_buffer1D<double>( 1, 0 ) );
      directions.push_back( nll::core::make_buffer1D<double>( 0, 1 ) );

      double val;
      nll::core::Buffer1D<double> p = nll::core::make_buffer1D<double>( -20, 10 );
      val = nll::algorithm::powell( p, directions, 1e-4, func1, 20, &error );
      TESTER_ASSERT( fabs( val ) < 1e-4 );
      TESTER_ASSERT( fabs( p[ 0 ] ) < 1e-3 );
      TESTER_ASSERT( fabs( p[ 1 ] ) < 1e-3 );
      TESTER_ASSERT( !error );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestBracketing);
 TESTER_TEST(test);
 TESTER_TEST(testBrent);
 TESTER_TEST(testLineMinimization);
 TESTER_TEST(testPowell);
 TESTER_TEST(testPowell2);
TESTER_TEST_SUITE_END();
#endif
