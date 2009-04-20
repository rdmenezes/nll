#include "stdafx.h"
#include <nll/nll.h>

class TestNllClassifierNearestNeighbor
{
public:
   void testNllClassifierNearestNeighbor()
   {    
      typedef nll::core::Buffer1D<double> Point1;
      typedef nll::algorithm::ClassifierNearestNeighbor<Point1, nll::algorithm::MetricEuclidian<Point1> >   NN;

      NN::BaseClassifier* baseClassifier;
      nll::algorithm::MetricEuclidian<Point1> metric;

      NN nearest( &metric );
      baseClassifier = &nearest;
      baseClassifier->write("data/nn.dat");
      NN newNearest("data/nn.dat", &metric);
      TESTER_ASSERT( newNearest.getMetric() );

      NN::Database dat;
      dat.add( NN::Database::Sample( nll::core::make_buffer1D<double>( 0, 0, 0 ),  0, NN::Database::Sample::LEARNING ) );
      dat.add( NN::Database::Sample( nll::core::make_buffer1D<double>( 10, 0, 0 ), 1, NN::Database::Sample::LEARNING ) );
      dat.add( NN::Database::Sample( nll::core::make_buffer1D<double>( 0, 10, 0 ), 2, NN::Database::Sample::LEARNING ) );
      dat.add( NN::Database::Sample( nll::core::make_buffer1D<double>( 0, 0, 10 ), 3, NN::Database::Sample::LEARNING ) );

      dat.add( NN::Database::Sample( nll::core::make_buffer1D<double>( 1, -1, 1 ),  0, NN::Database::Sample::TESTING ) );
      dat.add( NN::Database::Sample( nll::core::make_buffer1D<double>( 12, 2, 0 ), 1, NN::Database::Sample::TESTING ) );
      dat.add( NN::Database::Sample( nll::core::make_buffer1D<double>( 1, 10, 3 ), 2, NN::Database::Sample::TESTING ) );
      dat.add( NN::Database::Sample( nll::core::make_buffer1D<double>( 1, 1, 9 ), 3, NN::Database::Sample::TESTING ) );

      NN nearestLearning( &metric );
      nearestLearning.learn( dat, nll::core::make_buffer1D<double>( 1 ) );

      NN::Result resultLearning = nearestLearning.test( dat );
     
      nearestLearning.write( "data/nn.dat" );
      TESTER_ASSERT( resultLearning.learningError <= 0 && resultLearning.testingError <= 0 );

      NN nearestLoading( "data/nn.dat", &metric );
      resultLearning = nearestLearning.test( dat );
      TESTER_ASSERT( resultLearning.learningError <= 0 && resultLearning.testingError <= 0 );
   
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllClassifierNearestNeighbor);
TESTER_TEST(testNllClassifierNearestNeighbor);
TESTER_TEST_SUITE_END();
#endif
