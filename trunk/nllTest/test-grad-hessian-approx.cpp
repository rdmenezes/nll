#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

class TestFuncGradientHessianApproximation
{
public:
   struct Function1 : public algorithm::OptimizerClient
   {
      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const
      {
         // the test function is x * y^3 + 4x^2
         ensure( parameters.size() == 2, "only 2 params expected" );
         return parameters[ 0 ] * std::pow( parameters[ 1 ], 3.0 ) + 4 * core::sqr( parameters[ 0 ] );
      }

      virtual core::Buffer1D<f64> evaluateGradient( const core::Buffer1D<f64>& parameters ) const
      {
         ensure( parameters.size() == 2, "only 2 params expected" );
         core::Buffer1D<f64> grad( 2 );

         // df/dx = y^3 + 4 * x
         grad[ 0 ] = std::pow( parameters[ 1 ], 3.0 ) + 8 * parameters[ 0 ];

         // df/dy = 3 * x * y^2
         grad[ 1 ] = 3 * parameters[ 0 ] * core::sqr( parameters[ 1 ] );

         return grad;
      }

      virtual core::Matrix<f64> evaluateHessian( const core::Buffer1D<f64>& parameters ) const
      {
         ensure( parameters.size() == 2, "only 2 params expected" );

         // df/dxdx = d/dx( y^3 + 8x ) = 8
         // df/dydy = d/dy( 3 * x * y^2 ) = 6 * x * y
         // df/dxdy = d/dx( 3 * x * y^2 ) = 3 * y^2

         core::Matrix<f64> h( 2, 2 );
         h( 0, 0 ) = 8;
         h( 1, 1 ) = 6 * parameters[ 0 ] * parameters[ 1 ];
         h( 0, 1 ) = 3 * core::sqr( parameters[ 1 ] );
         h( 1, 0 ) = h( 0, 1 );
         return h;
      }
   };

   void testGradient()
   {
      Function1 func;
      algorithm::GradientCalculatorFiniteDifference approximator( func, 1e-8 );

      for ( size_t n = 0; n < 1000; ++n )
      {
         const core::Buffer1D<double> parameters = core::make_buffer1D<double>( core::generateUniformDistribution( -10, 10 ), core::generateUniformDistribution( -10, 10 ) );
         const core::Buffer1D<double> gradientRef = func.evaluateGradient( parameters );
         const core::Buffer1D<double> gradientFound = approximator.evaluate( parameters );

         TESTER_ASSERT( gradientRef.equal( gradientFound, 1e-3 ) );
      }
   };

   void testHessian()
   {
      Function1 func;
      algorithm::HessianCalculatorForwardFiniteDifference approximator( func, 1e-5 );

      for ( size_t n = 0; n < 1000; ++n )
      {
         const core::Buffer1D<double> parameters = core::make_buffer1D<double>( core::generateUniformDistribution( -2, 2 ), core::generateUniformDistribution( -2, 2 ) );
         const core::Matrix<double> ref = func.evaluateHessian( parameters );
         const core::Matrix<double> found = approximator.evaluate( parameters );

         TESTER_ASSERT( ref.equal( found, 1e-3 ) );
      }
   };
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestFuncGradientHessianApproximation);
TESTER_TEST(testGradient);
TESTER_TEST(testHessian);
TESTER_TEST_SUITE_END();
#endif