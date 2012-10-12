#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   


   

   


 
}
}

class TestOptimizerFunc
{
public:
   struct Function : public algorithm::OptimizerClient
   {
      Function( const std::string& name ) : functionName( name )
      {}

      virtual double evaluateReal( const core::Buffer1D<f64>& parameters ) const = 0;

      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const
      {
         ++_nbEvals;
         return evaluateReal( parameters );
      }

      virtual core::Buffer1D<f64> evaluateGradient( const core::Buffer1D<f64>& parameters ) const
      {
         static const double step = 1e-10;
         algorithm::GradientCalculatorFiniteDifference gradientCalculator( *this, step );
         return gradientCalculator.evaluate( parameters );
      }

      virtual core::Matrix<f64> evaluateHessian( const core::Buffer1D<f64>& parameters ) const
      {
         static const double step = 1e-6;
         algorithm::HessianCalculatorForwardFiniteDifference calculator( *this, step );
         return calculator.evaluate( parameters );
      }

      void resetEvalCount() const
      {
         _nbEvals = 0;
      }

      size_t getNbEvals() const
      {
         return _nbEvals;
      }

      virtual ~Function()
      {}

      core::Buffer1D<double>                       expectedSolution;
      double                                       expectedMinimaValue;
      algorithm::ParameterOptimizers               optimizerParameter;
      std::string                                  functionName;

   private:
      mutable size_t                               _nbEvals;
   };

   /**
    @brief Trivial function
    */
   struct FunctionCircle : public Function
   {
      FunctionCircle() : Function( "FunctionCircle" )
      {
         expectedSolution = core::make_buffer1D<double>( 0, 0 );
         expectedMinimaValue = 0;
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -5, 5, 0, 5, 0.1 ) );
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -5, 5, 0, 5, 0.1 ) );
      }

      virtual double evaluateReal( const core::Buffer1D<f64>& parameters ) const 
      {
         return core::sqr( parameters[ 0 ] ) + core::sqr( parameters[ 1 ] );
      }
   };

   /**
    @see http://en.wikipedia.org/wiki/Rosenbrock_function
    */
   struct FunctionRosenbrock : public Function
   {
      FunctionRosenbrock() : Function( "FunctionRosenbrock" )
      {
         expectedSolution = core::make_buffer1D<double>( 1, 1 );
         expectedMinimaValue = 0;
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -5, 5, 0, 5, 0.1 ) );
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -5, 5, 0, 5, 0.1 ) );
      }

      virtual double evaluateReal( const core::Buffer1D<f64>& parameters ) const 
      {
         return core::sqr( 1 - parameters[ 0 ] ) + 100 * core::sqr( parameters[ 1 ] - core::sqr( parameters[ 0 ] ) );
      }
   };

   /**
    @see http://www.math.ntu.edu.tw/~wwang/cola_lab/test_problems/multiple_opt/multiopt_prob/Ackley/Ackley.htm
    */
   struct FunctionAckley : public Function
   {
      FunctionAckley() : Function( "FunctionAckley" )
      {
         expectedSolution = core::make_buffer1D<double>( 0, 0 );
         expectedMinimaValue = 7.125;
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -30, 30, 0, 5, 0.1 ) );
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -30, 30, 0, 5, 0.1 ) );
      }

      virtual double evaluateReal( const core::Buffer1D<f64>& parameters ) const 
      {
         const double a = 20;
         const double b = 0.2;
         const double c = 2 * core::PI;
         const double d = 5.7;
         const double f = 0.8;
         const double n =  2;

         const double x = parameters[ 0 ];
         const double y = parameters[ 1 ];

         const double val = 1 / f * ( - a * std::exp( -b * std::sqrt( 1 / n * ( x * x + y * y) ) ) -
                                      std::exp( 1 / n * ( std::cos( c * x ) + std::cos( c * y ) ) ) +
                                      a + std::exp( 1.0 ) + d );
         return val;
      }
   };

    /**
    @see http://en.wikipedia.org/wiki/Rastrigin_function
    */
   struct FunctionRastrigin : public Function
   {
      FunctionRastrigin() : Function( "FunctionRastrigin" )
      {
         expectedSolution = core::make_buffer1D<double>( 0, 0 );
         expectedMinimaValue = 0;
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -5.12, 5.12, 0, 5, 0.1 ) );
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -5.12, 5.12, 0, 5, 0.1 ) );
      }

      virtual double evaluateReal( const core::Buffer1D<f64>& parameters ) const 
      {
         const double a = 10;

         double result = a * parameters.size();
         for( size_t i = 0; i < parameters.size(); ++i )
         {
            result += core::sqr( parameters[ i ] ) - a * std::cos( 2 * core::PI * parameters[ i ] );
         }

         return result;
      }
   };

   static std::vector< std::shared_ptr<Function> > generateTestFunctions()
   {
      std::vector< std::shared_ptr<Function> > functions;
      functions.push_back( std::shared_ptr<Function>( new FunctionAckley() ) );
      functions.push_back( std::shared_ptr<Function>( new FunctionCircle() ));
      functions.push_back( std::shared_ptr<Function>( new FunctionRosenbrock() ));
      functions.push_back( std::shared_ptr<Function>( new FunctionRastrigin() ));
      return functions;
   }

   static std::vector< std::shared_ptr<Function> > generateTestFunctions_Newton()
   {
      std::vector< std::shared_ptr<Function> > functions;
      functions.push_back( std::shared_ptr<Function>( new FunctionCircle() ));
      functions.push_back( std::shared_ptr<Function>( new FunctionRosenbrock() ));
      return functions;
   }

   static std::vector< std::shared_ptr<Function> > generateTestFunctions_GradientDescent()
   {
      std::vector< std::shared_ptr<Function> > functions;
      functions.push_back( std::shared_ptr<Function>( new FunctionCircle() ));
      functions.push_back( std::shared_ptr<Function>( new FunctionRosenbrock() ));
      return functions;
   }

   static void test( const Function& function, algorithm::Optimizer& optimizer, size_t nbRestarts = 10, double epsilon = 1e-1 )
   {
      for ( size_t iter = 0; iter < nbRestarts; ++iter )
      {
         function.resetEvalCount();
         const core::Buffer1D<double> result = optimizer.optimize( function, function.optimizerParameter );
         const double v = function.evaluate( result );
         size_t nbEvals = function.getNbEvals();

         std::cout << "Function=" << function.functionName << std::endl 
                   << " iter=" << iter << std::endl
                   << " expectedResults=" << function.expectedSolution << std::endl
                   << " expectedMinima=" << function.expectedMinimaValue << std::endl
                   << " found=" << result << std::endl
                   << " miniMaxFound=" << v << std::endl
                   << " number of function evaluation=" << nbEvals << std::endl;

         bool passed = true;
         for ( size_t n = 0; n < function.expectedSolution.size(); ++n )
         {
            bool val = core::equal<double>( function.expectedSolution[ n ], result[ n ], epsilon );
            passed &= val;

            if ( !val )
               break;
         }

         if ( passed )
         {
            return;
         }

         // not passed! it means we must at least be on a local minima!!
         core::Buffer1D<double> grad = function.evaluateGradient( result );
         for ( size_t n = 0; n < grad.size(); ++n )
         {
            ensure( core::equal<double>( grad[ n ], 0.0, 1e-2 ), "the gradient is not zero => we are not even on a local minima!" );
         }
      }

      TESTER_ASSERT( 0 ); // failed!
   }

   void testGradiendOptimizer()
   {
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions_GradientDescent();

      algorithm::StopConditionStable stop( 500 );
      algorithm::OptimizerGradientDescent optimizer( stop, -0.0013 );

      const size_t size = functions.size();
      for ( size_t n = 0; n < size; ++n )
      {
         for ( size_t nn = 0; nn < 5; ++nn )
         {
            srand( nn );
            test( *functions[ n ], optimizer );
         }
      }
   }

   void testGradiendOptimizer_ackely()
   {
      FunctionAckley function;

      algorithm::StopConditionStable stop( 500 );
      algorithm::OptimizerGradientDescent optimizer( stop, 0.13 );

      for ( size_t n = 0; n < 10; ++n )
      {
         test( function, optimizer, 1 );
      }
   }

   void testGradiendOptimizer_rastrigin()
   {
      FunctionRastrigin function;

      algorithm::StopConditionStable stop( 500 );
      algorithm::OptimizerGradientDescent optimizer( stop, 0.001 );

      for ( size_t n = 0; n < 1; ++n )
      {
         test( function, optimizer, 100 );   // lot of restarts... we need to find the correct valley
      }
   }

   void testPowell()
   {
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions();
      algorithm::OptimizerPowell optimizer;

      const size_t size = functions.size();
      for ( size_t n = 0; n < size; ++n )
      {
         for ( size_t nn = 0; nn < 5; ++nn )
         {
            srand( nn );
            test( *functions[ n ], optimizer );
         }
      }
   }

   void testGA()
   {
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions();
      algorithm::OptimizerGeneticAlgorithm optimizer( 30, 100, 20 );

      const size_t size = functions.size();
      for ( size_t n = 0; n < size; ++n )
      {
         for ( size_t nn = 0; nn < 5; ++nn )
         {
            srand( nn );
            test( *functions[ n ], optimizer );
         }
      }
   }

   void testHarmony()
   {
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions();
      algorithm::StopConditionIteration stop( 800000 );
      algorithm::OptimizerHarmonySearch optimizer( 9, 0.8, 0.1, 5, &stop );

      const size_t size = functions.size();
      for ( size_t n = 0; n < size; ++n )
      {
         for ( size_t nn = 0; nn < 5; ++nn )
         {
            srand( nn );
            test( *functions[ n ], optimizer );
         }
      }
   }

   void testNewtonOptimizer()
   {
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions_Newton();
      algorithm::StopConditionStable stop( 10 );
      algorithm::OptimizerNewton optimizer( stop, 0.9, 0.1, 1 );

      const size_t size = functions.size();
      for ( size_t n = 0; n < size; ++n )
      {
         for ( size_t nn = 0; nn < 20; ++nn )
         {
            srand( nn );
            test( *functions[ n ], optimizer, 1 );
         }
      }
   }

   void testNewtonLineSearchOptimizer()
   {
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions();
      algorithm::StopConditionStable stop( 10 );
      algorithm::OptimizerNewtonLineSearch optimizer( stop, 0.1, 1 );

      const size_t size = functions.size();
      for ( size_t n = 0; n < size; ++n )
      {
         for ( size_t nn = 0; nn < 5; ++nn )
         {
            srand( nn );
            test( *functions[ n ], optimizer, 500 );
         }
      }
   }

   void testGradiendOptimizerLineSearch()
   {
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions();
      algorithm::StopConditionStable stop( 100 );
      algorithm::OptimizerGradientDescentLineSearch optimizer( stop );

      const size_t size = functions.size();
      for ( size_t n = 0; n < size; ++n )
      {
         for ( size_t nn = 0; nn < 5; ++nn )
         {
            srand( nn );
            test( *functions[ n ], optimizer, 100 );  // it is fine to have a high restart rate: some functions have very high number of local minima (e.g., ackley)
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestOptimizerFunc);
TESTER_TEST(testGradiendOptimizer);
TESTER_TEST(testGradiendOptimizerLineSearch);
TESTER_TEST(testPowell);
TESTER_TEST(testGA);
TESTER_TEST(testHarmony);
TESTER_TEST(testGradiendOptimizer_ackely);
TESTER_TEST(testGradiendOptimizer_rastrigin);
TESTER_TEST(testNewtonOptimizer);
TESTER_TEST(testNewtonLineSearchOptimizer);
TESTER_TEST_SUITE_END();
#endif