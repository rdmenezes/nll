#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>


struct OptimizerClientResult : public nll::algorithm::OptimizerClient
{
   virtual double getMin() const = 0;
};

/**
 @brief Min = 0 for xi = {0}, i=0...n
 */
struct FunctionJong : public OptimizerClientResult
{
   virtual double evaluate( const nll::core::Buffer1D<nll::f64>& parameters ) const
   {
      double accum = 0;
      for ( unsigned n = 0; n < parameters.size(); ++n )
         accum += nll::core::sqr( parameters[ n ] );
      return accum;
   }

   virtual double getMin() const
   {
      return 0;
   }
};

/**
 @brief Min = 0 for N=3, x=(1, 1, 1)

 http://en.wikipedia.org/wiki/Rosenbrock_function
 */
struct FunctionRosenbrockValley : public OptimizerClientResult
{
   virtual double evaluate( const nll::core::Buffer1D<nll::f64>& parameters ) const
   {
      double accum = 0;
      for ( unsigned n = 0; n < parameters.size() - 1; ++n )
         accum += 100  * nll::core::sqr( parameters[ n + 1 ] - parameters[ n ] * parameters[ n ] ) + 
                  nll::core::sqr( 1 - parameters[ n ] );
      return accum;
   }

   virtual double getMin() const
   {
      return 0;
   }
};

/**
 @brief Min = 0 for xi = 0, i = 0, ..., n

 http://wiki.scilab.org/Rastrigin_function
 */
struct FunctionRastrigin : public OptimizerClientResult
{
   virtual double evaluate( const nll::core::Buffer1D<nll::f64>& parameters ) const
   {
      double accum = 0;
      for ( unsigned n = 0; n < parameters.size(); ++n )
         accum += nll::core::sqr( parameters[ n ] ) - 10 * cos( 2 * nll::core::PI * parameters[ n ] );
      return accum + 10 * parameters.size();
   }

   virtual double getMin() const
   {
      return 0;
   }
};

/**
 @brief f(x)= -418.89829 for xi=420.9687 i=0..n
 */

static int eval = 0;

struct FunctionGriewangk : public OptimizerClientResult
{
   virtual double evaluate( const nll::core::Buffer1D<nll::f64>& parameters ) const
   {
      double accum = 0;
      for ( unsigned n = 0; n < parameters.size(); ++n )
         accum += nll::core::sqr( parameters[ n ] );

      double accum2 = 1;
      for ( unsigned n = 0; n < parameters.size(); ++n )
      {
         double tmp = cos( parameters[ n ] / sqrt( (double)( n + 1 ) ) );
         accum2 *= tmp;
      }
      return accum / 4000 - accum2 + 1;
   }

   virtual double getMin() const
   {
      return 0;
   }
};



/**
 @brief Test the optimizers with different test functions.

 See "Test functions for optimization needs",
 http://www.zsd.ict.pwr.wroc.pl/files/docs/functions.pdf
 */
struct TestOptimizer
{
   static std::vector<OptimizerClientResult*> getFunctions()
   {
      std::vector<OptimizerClientResult*> f;
      f.push_back( new FunctionJong() );
      f.push_back( new FunctionRosenbrockValley() );
      f.push_back( new FunctionRastrigin() );
      f.push_back( new FunctionGriewangk() );
      return f;
   }


   void testPowell()
   {
      const double tol = 1e-2;
      std::vector<OptimizerClientResult*> functions = getFunctions();

      for ( unsigned n = 0; n < static_cast<unsigned>( functions.size() ); ++n )
      {
         typedef nll::core::Buffer1D<double> Point;

         nll::algorithm::ParameterOptimizers parameters;
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 10, 0.5 ) );
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 10, 0.5 ) );
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 10, 0.5 ) );
         
         nll::algorithm::OptimizerPowell optimizer( 200, 1e-3 );
         std::vector<double> res = optimizer.optimize( *functions[ n ], parameters );

         nll::core::Buffer1D<double> buffer( (unsigned)res.size() );
         for ( unsigned nn = 0; nn < res.size(); ++nn )
            buffer[ nn ] = res[ nn ];
         double val = (*functions[ n ]).evaluate( buffer );
         TESTER_ASSERT( val < functions[ n ]->getMin() + tol );

         std::cout << "#";
      }
   }

   void testGA()
   {
      srand( 10 );
      const double tol = 1e-2;
      std::vector<OptimizerClientResult*> functions = getFunctions();

      for ( unsigned n = 0; n < static_cast<unsigned>( functions.size() ); ++n )
      {
         typedef nll::core::Buffer1D<double> Point;

         nll::algorithm::ParameterOptimizers parameters;
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 40, 0.05 ) );
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 40, 0.05 ) );
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 40, 0.05 ) );
         
         // we need a lot of steps as GA are not very good at efficiently optimizing finely a solution
         nll::algorithm::OptimizerGeneticAlgorithm optimizer( 20, 100, 50, 0.9f, 0.5 );
         std::vector<double> res = optimizer.optimize( *functions[ n ], parameters );

         nll::core::Buffer1D<double> buffer( (unsigned)res.size() );
         for ( unsigned nn = 0; nn < res.size(); ++nn )
            buffer[ nn ] = res[ nn ];
         double val = (*functions[ n ]).evaluate( buffer );
         TESTER_ASSERT( val < functions[ n ]->getMin() + tol );

         std::cout << "#";
      }
   }
      
   void testHarmony()
   {
      srand( 0 );
      const double tol = 1e-1;
      std::vector<OptimizerClientResult*> functions = getFunctions();

      for ( unsigned n = 0; n < static_cast<unsigned>( functions.size() ); ++n )
      {
         typedef nll::core::Buffer1D<double> Point;

         nll::algorithm::ParameterOptimizers parameters;
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 10, 0.5 ) );
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 10, 0.5 ) );
         parameters.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -50, 50, 5, 10, 0.5 ) );
         
         nll::algorithm::OptimizerHarmonySearch optimizer( 5, 0.8, 0.5, 0.1, new nll::algorithm::StopConditionIteration( 18000 ) );
         std::vector<double> res = optimizer.optimize( *functions[ n ], parameters );

         nll::core::Buffer1D<double> buffer( (unsigned)res.size() );
         for ( unsigned nn = 0; nn < res.size(); ++nn )
            buffer[ nn ] = res[ nn ];
         double val = (*functions[ n ]).evaluate( buffer );
         TESTER_ASSERT( val < functions[ n ]->getMin() + tol );

         std::cout << "#";
      }
   }
};

TESTER_TEST_SUITE( TestOptimizer );
TESTER_TEST( testPowell );
TESTER_TEST( testGA );
TESTER_TEST( testHarmony );
TESTER_TEST_SUITE_END();
