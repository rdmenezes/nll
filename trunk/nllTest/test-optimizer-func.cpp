#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   /**
    @brief Pure Newton optimizer

    Fits a Taylor series of degree two to the function to optimize and find the extremum.

    f(x+d) = f(x) + grad f(x) * d + d' * Hessian f( x ) * d / 2
    x_new = x - lambda * (Hf^-1(x)) * grad f(x)

    This algorithm is inverting the Hessian, so this is not a good approach for high dimentional problems. Instead
    we want to factorize pn = Hf^-1(x)grad(x) => Hf(x) * pn = grad(x)

    @note if the hessian can't be inverted, we will do H = H + alpha * I until it can be (i.e., by doing so we almost revert to gradient descent)
    @see http://en.wikipedia.org/wiki/Newton's_method_in_optimization
    */
   class NLL_API OptimizerNewton : public Optimizer
   {
   public:
      using Optimizer::optimize;

   public:
      /**
       @param lambda in ]0..1] to satify the Wolfe condition (http://en.wikipedia.org/wiki/Wolfe_conditions)
       */
      OptimizerNewton( double lambda = 0.9 ) : _lambda( lambda )
      {}

      virtual core::Buffer1D<double> optimize( const OptimizerClient& client, const ParameterOptimizers& parameters, const core::Buffer1D<double>& seed )
      {
      }

   protected:
      double   _lambda;
   };
}
}

class TestOptimizerFunc
{
public:
   struct Function : public algorithm::OptimizerClient
   {
      Function( const std::string& name ) : functionName( name )
      {}

      virtual core::Buffer1D<f64> evaluateGradient( const core::Buffer1D<f64>& parameters ) const
      {
         static const double step = 1e-10;

         core::Buffer1D<f64> gradient( parameters.size() );
         const double val = evaluate( parameters );

         // evaluate gradient by finite difference
         for ( size_t n = 0; n < parameters.size(); ++n )
         {
            core::Buffer1D<f64> p;
            p.clone( parameters );
            p[ n ] += step;
            const double vale = evaluate( p );

            gradient[ n ] = ( vale - val ) / step;
         }

         return gradient;
      }

      virtual ~Function()
      {}

      core::Buffer1D<double>                       expectedSolution;
      double                                       expectedMinimaValue;
      algorithm::ParameterOptimizers               optimizerParameter;
      std::string                                  functionName;
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

      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const 
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

      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const 
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

      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const 
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

      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const 
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
         const core::Buffer1D<double> result = optimizer.optimize( function, function.optimizerParameter );
         const double v = function.evaluate( result );

         std::cout << "Function=" << function.functionName << std::endl 
                   << " iter=" << iter << std::endl
                   << " expectedResults=" << function.expectedSolution << std::endl
                   << " expectedMinima=" << function.expectedMinimaValue << std::endl
                   << " found=" << result << std::endl
                   << " miniMaxFound=" << v << std::endl;

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
      algorithm::OptimizerGeneticAlgorithm optimizer( 50, 50, 10 );

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
      algorithm::StopConditionIteration stop( 200000 );
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
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestOptimizerFunc);
TESTER_TEST(testGradiendOptimizer);
TESTER_TEST(testPowell);
TESTER_TEST(testGA);
TESTER_TEST(testHarmony);
TESTER_TEST(testGradiendOptimizer_ackely);
TESTER_TEST(testGradiendOptimizer_rastrigin);
TESTER_TEST_SUITE_END();
#endif