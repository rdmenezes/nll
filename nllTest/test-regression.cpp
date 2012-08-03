#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;
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

      DummyRegression() : Base( buildParameters() )
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

   void testRegressionMlp()
   {
      typedef RegressionMlp< Buffer1D<double>, Buffer1D<double> > RegressionMlp;
      typedef RegressionMlp::Database Database;

      RegressionMlp mlp;
      Database dat;

      srand( 0 );
      for ( size_t n = 0; n < 500; ++n )
      {
         double val = static_cast<double>( rand() ) / RAND_MAX;
         double output = fabs( cos( val * 4 ) );

         dat.add( Database::Sample( make_buffer1D<double>( val ), make_buffer1D<double>( output ), Database::Sample::LEARNING ) );
      }

      for ( size_t n = 0; n < 100; ++n )
      {
         double val = static_cast<double>( rand() ) / RAND_MAX;
         double output = fabs( cos( val * 4 ) );

         dat.add( Database::Sample( make_buffer1D<double>( val ), make_buffer1D<double>( output ), Database::Sample::TESTING ) );
      }

      mlp.learn( dat, make_buffer1D<double>( 20, 2, 10 ) );
      RegressionMlp::Result r = mlp.test( dat );

      TESTER_ASSERT( r.learningError < 0.09 );
      TESTER_ASSERT( r.testingError < 0.09 );
   }

   void testRegressionSvrNu()
   {
      typedef RegressionSvmNu< Buffer1D<double>, Buffer1D<double> > RegressionSvm;
      typedef RegressionSvm::Database Database;

      RegressionSvm svm;
      Database dat;

      srand( 0 );
      for ( size_t n = 0; n < 500; ++n )
      {
         double val = static_cast<double>( rand() ) / RAND_MAX;
         double output = fabs( cos( val * 4 ) );

         dat.add( Database::Sample( make_buffer1D<double>( val ), make_buffer1D<double>( output ), Database::Sample::LEARNING ) );
      }

      for ( size_t n = 0; n < 100; ++n )
      {
         double val = static_cast<double>( rand() ) / RAND_MAX;
         double output = fabs( cos( val * 4 ) );

         dat.add( Database::Sample( make_buffer1D<double>( val ), make_buffer1D<double>( output ), Database::Sample::TESTING ) );
      }

      svm.learn( dat, make_buffer1D<double>( 10, 100, 0.2 ) );
      RegressionSvm::Result r = svm.test( dat );

      TESTER_ASSERT( r.learningError < 0.09 );
      TESTER_ASSERT( r.testingError < 0.09 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRegression);
TESTER_TEST(testBasic);
TESTER_TEST(testRegressionMlp);
TESTER_TEST(testRegressionSvrNu);
TESTER_TEST_SUITE_END();
#endif
