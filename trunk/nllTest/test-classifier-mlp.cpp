#include "stdafx.h"
#include <nll/nll.h>
#include <time.h>

class TestNllAlgorithmClassifierMlp
{
public:
   void testMlpClassifier()
   {
      srand(1);
      #define TYPE double
      typedef nll::algorithm::ClassifierMlp<nll::core::Buffer1D<TYPE> > CMLP;

      CMLP mlp;
      CMLP::Database dat;
      dat.add(CMLP::Database::Sample(nll::core::make_buffer1D<TYPE>(0.5), 1, CMLP::Database::Sample::LEARNING));
      dat.add(CMLP::Database::Sample(nll::core::make_buffer1D<TYPE>(-0.5), 0, CMLP::Database::Sample::LEARNING));
      dat.add(CMLP::Database::Sample(nll::core::make_buffer1D<TYPE>(-0.5), 0, CMLP::Database::Sample::TESTING));
      nll::core::Buffer1D<double> params(3);
      mlp.learn(dat, params);
      
      #undef TYPE
      #define TYPE float
      typedef nll::algorithm::ClassifierMlp<nll::core::Buffer1D<TYPE> > CMLP2;

      CMLP2 mlp2;
      CMLP2::Database dat2;
      dat2.add(CMLP2::Database::Sample(nll::core::make_buffer1D<TYPE>(0, 0), 0, CMLP2::Database::Sample::LEARNING));
      dat2.add(CMLP2::Database::Sample(nll::core::make_buffer1D<TYPE>(1, 0), 1, CMLP2::Database::Sample::LEARNING));
      dat2.add(CMLP2::Database::Sample(nll::core::make_buffer1D<TYPE>(0, 1), 1, CMLP2::Database::Sample::LEARNING));
      dat2.add(CMLP2::Database::Sample(nll::core::make_buffer1D<TYPE>(1, 1), 0, CMLP2::Database::Sample::LEARNING));

      dat2.add(CMLP2::Database::Sample(nll::core::make_buffer1D<TYPE>(0, 0), 0, CMLP2::Database::Sample::TESTING));
      dat2.add(CMLP2::Database::Sample(nll::core::make_buffer1D<TYPE>(1, 0), 1, CMLP2::Database::Sample::TESTING));
      dat2.add(CMLP2::Database::Sample(nll::core::make_buffer1D<TYPE>(0, 1), 1, CMLP2::Database::Sample::TESTING));
      dat2.add(CMLP2::Database::Sample(nll::core::make_buffer1D<TYPE>(1, 1), 0, CMLP2::Database::Sample::TESTING));
      
      nll::core::Buffer1D<double> params2(3);
      params2[ 0 ] = 3.0f;
      params2[ 1 ] = 0.4f;
      params2[ 2 ] = 0.1f;
      mlp2.learn(dat2, params2);

      
      CMLP2::BaseClassifier* c = &mlp2;
      CMLP2::BaseClassifier::Result res = c->test( dat2 );
      TESTER_ASSERT( res.learningError < 0.01 );
      TESTER_ASSERT( res.testingError < 0.01 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllAlgorithmClassifierMlp);
#ifndef DONT_RUN_SLOW_TEST
 TESTER_TEST(testMlpClassifier);
#endif
TESTER_TEST_SUITE_END();
#endif
