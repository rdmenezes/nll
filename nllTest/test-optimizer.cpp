#include "stdafx.h"
#include <nll/nll.h>
#include <time.h>

static nll::f64 eval(const nll::core::Buffer1D<nll::f64>& g)
{
   assert(g.size() == 5);
   nll::f64 tmp =	g[0] * sin(g[0]) +
			   1.7 * g[1] * sin(g[0]) -
			   1.5 * g[2] - 0.1 * g[3] * cos(g[3] + g[4] - g[0]) + (0.2 * g[4] * g[4] - g[1]) - 1;
   return nll::core::absolute(tmp);
}

static nll::f64 eval(const std::vector<nll::f64>& g)
{
   assert(g.size() == 5);
   nll::f64 tmp =	g[0] * sin(g[0]) +
			   1.7 * g[1] * sin(g[0]) -
			   1.5 * g[2] - 0.1 * g[3] * cos(g[3] + g[4] - g[0]) + (0.2 * g[4] * g[4] - g[1]) - 1;
   return nll::core::absolute(tmp);
}


class ClassifierTest3 : public nll::algorithm::Classifier<float*>
{
   typedef float* T;
public:
   typedef nll::algorithm::Classifier<float*>   Classifier;

   virtual Classifier* deepCopy() const { return (Classifier*)this; }
   virtual void read( std::istream&  ){}
   virtual void write( std::ostream&  ) const{}
   virtual Classifier::Class test( const T&  ) const{return 0;}
   virtual void learn( const Database& , const nll::core::Buffer1D<nll::f64>& ){}
   ClassifierTest3() : Classifier( createParameters() ){}
   virtual double evaluate( const nll::core::Buffer1D<nll::f64>& params,  const Database& ) const
   {
      return eval(params);
   }
   static const nll::algorithm::ParameterOptimizers createParameters()
   {
      nll::algorithm::ParameterOptimizers params;
      params.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -1000, 1000, 0, 100, 0.0001 ) );
      params.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -1000, 1000, 0, 100, 0.0001 ) );
      params.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -1000, 1000, 0, 100, 0.0001 ) );
      params.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -1000, 1000, 0, 100, 0.0001 ) );
      params.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( -1000, 1000, 0, 100, 0.0001 ) );
      return params;
   }
};

class GridSearchClientTest : public nll::algorithm::OptimizerClient
{
public:
   GridSearchClientTest( nll::algorithm::ParameterOptimizers params, double value, nll::ui32 nbRecursiveLevel, nll::f64 /*_granularityUpdate*/ )
   {
      // initialize internal memory and indexes
      _size = 1;
      _sizes = std::vector<nll::ui32>( params.size() );
      _index = std::vector<nll::ui32>( params.size() );
      for ( nll::ui32 n = 0; n < params.size(); ++n )
      {
         _sizes[ n ] = (nll::ui32)floor( params[ n ].getMax() - params[ n ].getMin() );
         _index[ n ] = _size;
         _size *= _sizes[ n ];
      }
      _index[ 0 ] = 1;
      _buf = new double[ _size ];

      // fill the buffer with random values, put the fitter value at a random position at a random level, the optimizer MUST found the solution
      nll::ui32 level = rand() % nbRecursiveLevel;
      level;

      // set the best position
      nll::core::Buffer1D<nll::f64> valuePos( (nll::ui32)params.size() );
      for ( nll::ui32 n = 0; n < params.size(); ++n )
      {
         valuePos[ n ] = rand() % ( _sizes[ n ] - 2 ) + params[ n ].getMin();
      }

      // fill the buffer
      for ( nll::ui32 n = 0; n < _size; ++n )
         _buf[ n ] = -(double)(rand() % (nll::ui32)value);
      at( valuePos ) = -value;
      _bestPoint = valuePos;
   }
   ~GridSearchClientTest()
   {
      delete [] _buf;
   }
   double& at( const nll::core::Buffer1D<nll::f64>& pos )
   {
      nll::ui32 index = 0;
      for ( nll::ui32 n = 0; n < pos.size(); ++n )
         index += _index[ n ] * (nll::ui32)pos[ n ];
      return _buf[ index ];
   }
   const double& at( const nll::core::Buffer1D<nll::f64>& pos ) const
   {
      nll::ui32 index = 0;
      for ( nll::ui32 n = 0; n < pos.size(); ++n )
         index += _index[ n ] * (nll::ui32)pos[ n ];
      return _buf[ index ];
   }
   virtual double evaluate( const nll::core::Buffer1D<nll::f64>& parameters ) const
   {
      //std::cout << "eval=" << std::endl;
      //parameters.print( std::cout );
      return at( parameters );
   }
   const nll::core::Buffer1D<nll::f64> getBestPoint() const { return _bestPoint; }

private:
   nll::core::Buffer1D<nll::f64> _bestPoint;
   nll::f64                      _value;
   std::vector<nll::ui32>        _sizes;
   std::vector<nll::ui32>        _index;
   nll::ui32                     _size;
   double*                       _buf;
};

class TestNllOptimizer
{
public:
   void testOptimizerGA()
   {
      srand(1);
      ClassifierTest3 c3;
      nll::algorithm::OptimizerGeneticAlgorithm gaOptimizer( 50, 300, 10 );

      ClassifierTest3::Database  dummyDatabase;
      std::vector<double> genes = gaOptimizer.optimize(c3.createOptimizer(dummyDatabase), ClassifierTest3::createParameters());

      double val = eval( genes );
      TESTER_ASSERT( val < 0.0001 );
   }

   void testOptimizerHarmonySearch()
   {
      srand( (nll::ui32)time( 0 ) );
      ClassifierTest3 c3;
      ClassifierTest3::Database  dummyDatabase;
      nll::algorithm::Optimizer* optimizer = new nll::algorithm::OptimizerHarmonySearch( 6, 0.95, 0.6, 0.001, new nll::algorithm::StopConditionIteration( 10000 ) );
      std::vector<double> genes = optimizer->optimize( c3.createOptimizer( dummyDatabase ), ClassifierTest3::createParameters() );
      delete optimizer;
      double val = eval( genes );
      TESTER_ASSERT( val < 0.0001 );
   }

   void testOptimizerHarmonySearchMemory()
   {
      srand( (nll::ui32)time( 0 ) + 1 );
      ClassifierTest3 c3;
      ClassifierTest3::Database  dummyDatabase;
      nll::algorithm::Optimizer* optimizer = new nll::algorithm::OptimizerHarmonySearchMemory( 6, 0.95, 0.6, 0.001, new nll::algorithm::StopConditionIteration( 10000 ), 0.00001, new nll::algorithm::MetricEuclidian<nll::algorithm::OptimizerHarmonySearchMemory::TMetric::value_type>() );
      std::vector<double> genes = optimizer->optimize( c3.createOptimizer( dummyDatabase ), ClassifierTest3::createParameters() );
      delete optimizer;
      double val = eval( genes );
      TESTER_ASSERT( val < 0.0001 );
   }

   void testOptimizerGridSearch()
   {
      srand( 0 );
      nll::algorithm::ParameterOptimizers params;
      params.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( 0, 100, 50, 1, 1 ) );
      params.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( 0, 100, 50, 1, 1 ) );
      params.push_back( new nll::algorithm::ParameterOptimizerGaussianLinear( 0, 100, 50, 1, 1 ) );

      GridSearchClientTest client( params, 400, 1, 0.25 );
      nll::algorithm::Optimizer* optimizer = new nll::algorithm::OptimizerGridSearch();
      std::vector<double> sol = optimizer->optimize( client, params );
      nll::core::Buffer1D<double> solbuf( (nll::ui32)sol.size() );
      for ( nll::ui32 n = 0; n < sol.size(); ++n )
         solbuf[ n ] = sol[ n ];
      /*
      std::cout << "solFound=" << std::endl;
      solbuf.print( std::cout );
      std::cout << "val=" << client.evaluate( solbuf ) << std::endl;
      std::cout << "sol=" << std::endl;
      */
      client.getBestPoint().print( std::cout );
      TESTER_ASSERT( solbuf == client.getBestPoint() );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllOptimizer);
TESTER_TEST(testOptimizerGridSearch);
TESTER_TEST(testOptimizerHarmonySearch);
TESTER_TEST(testOptimizerHarmonySearchMemory);
# ifndef DONT_RUN_SLOW_TEST
TESTER_TEST(testOptimizerGA);
# endif
TESTER_TEST_SUITE_END();
#endif
