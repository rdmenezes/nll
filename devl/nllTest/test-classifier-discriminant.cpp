# include <nll/nll.h>
#include "database-builder.h"
#include <tester/register.h>

using namespace nll::core;
using namespace nll::algorithm;

class TestClassifierDiscriminant
{
public:
   void test1()
   {
      srand( 2 );
      typedef Buffer1D<double>   Point;
      typedef ClassifierDiscriminant<Point>  Classifier;

      Classifier classifier;
      Classifier::Database dat = loadDatabaseSpect<Point>();

      classifier.learn( dat, Buffer1D<double>() );
      Classifier::Result r = classifier.test( dat );
      std::cout << "ClassifierDiscriminant learning error=" << r.learningError << " test error=" << r.testingError <<std::endl;
      TESTER_ASSERT( r.learningError < 0.07 );
      TESTER_ASSERT( r.testingError < 0.35 );

      classifier.write( "tmpboosting.bin" );
      Classifier classifier2;
      classifier2.read( "tmpboosting.bin" );
      Classifier::Result r2 = classifier2.test( dat );
      TESTER_ASSERT( r.learningError == r2.learningError );
      TESTER_ASSERT( r.testingError == r2.testingError );
   }

   void test2Pca()
   {
      srand( 2 );
      typedef Buffer1D<double>   Point;
      typedef ClassifierDiscriminant<Point>  Classifier;

      Classifier::Database dat = loadDatabaseSpect<Point>();
      nll::algorithm::FeatureTransformationPca<Point> pca;
      pca.compute( dat, 9 );
      Classifier::Database datProcessed = pca.transform( dat );

      Classifier classifier;
      classifier.learn( datProcessed, Buffer1D<double>() );
      Classifier::Result r = classifier.test( datProcessed );
      std::cout << "ClassifierDiscriminant learning error=" << r.learningError << " test error=" << r.testingError <<std::endl;
      TESTER_ASSERT( r.learningError < 0.17 );
      TESTER_ASSERT( r.testingError < 0.19 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestClassifierDiscriminant);
TESTER_TEST(test1);
TESTER_TEST(test2Pca);
TESTER_TEST_SUITE_END();
#endif