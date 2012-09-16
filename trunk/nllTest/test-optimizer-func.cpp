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

      virtual core::Buffer1D<f64> evaluateGradient( const core::Buffer1D<f64>& parameters ) const
      {
         static const double step = 1e-3;

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

   struct FunctionRosenbrock : public Function
   {
      FunctionRosenbrock() : Function( "FunctionRosenbrock" )
      {
         expectedSolution = core::make_buffer1D<double>( 1, 1 );
         expectedMinimaValue = 0;
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -10, 10, 0, 5, 0.1 ) );
         optimizerParameter.push_back( new algorithm::ParameterOptimizerGaussianLinear( -10, 10, 0, 5, 0.1 ) );
      }

      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const 
      {
         return core::sqr( 1 - parameters[ 0 ] ) + 100 * core::sqr( parameters[ 1 ] - core::sqr( parameters[ 0 ] ) );
      }
   };

   static std::vector< std::shared_ptr<Function> > generateTestFunctions()
   {
      std::vector< std::shared_ptr<Function> > functions;
      functions.push_back( std::shared_ptr<Function>( new FunctionRosenbrock() ));

      return functions;
   }

   static void test( const Function& function, algorithm::Optimizer& optimizer, size_t nbRestarts = 10, double epsilon = 1e-2 )
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
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions();

      algorithm::StopConditionStable stop( 50 );
      algorithm::OptimizerGradientDescent optimizer( stop );

      test( *functions[ 0 ], optimizer );
   }

   void testPowell()
   {
      const std::vector< std::shared_ptr<Function> > functions = generateTestFunctions();
      algorithm::OptimizerPowell optimizer;

      const size_t size = functions.size();
      for ( size_t n = 0; n < size; ++n )
      {
         test( *functions[ n ], optimizer );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestOptimizerFunc);
TESTER_TEST(testGradiendOptimizer);
TESTER_TEST(testPowell);
TESTER_TEST_SUITE_END();
#endif