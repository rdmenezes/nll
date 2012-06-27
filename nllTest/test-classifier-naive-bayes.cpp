# include <nll/nll.h>
#include "database-builder.h"
#include <tester/register.h>

using namespace nll::core;
using namespace nll::algorithm;

class TestNaiveBayesClassifier
{
public:
   void test1()
   {
      srand( 2 );
      typedef Buffer1D<double>   Point;
      typedef ClassifierNaiveBayes<Point>  Classifier;

      Classifier classifier;
      Classifier::Database dat = loadDatabaseSpect<Point>();

      classifier.learn( dat, Buffer1D<double>() );
      Classifier::Result r = classifier.test( dat );
      std::cout << "classifier NaiveBayes learning error=" << r.learningError << " test error=" << r.testingError <<std::endl;
      TESTER_ASSERT( r.learningError < 0.27 );
      TESTER_ASSERT( r.testingError < 0.13 );

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
      typedef ClassifierNaiveBayes<Point>  Classifier;

      Classifier::Database dat = loadDatabaseSpect<Point>();
      nll::algorithm::FeatureTransformationPca<Point> pca;
      pca.compute( dat, 9 );
      Classifier::Database datProcessed = pca.transform( dat );

      Classifier classifier;
      classifier.learn( datProcessed, Buffer1D<double>() );
      Classifier::Result r = classifier.test( datProcessed );
      std::cout << "classifier NaiveBayes learning error=" << r.learningError << " test error=" << r.testingError <<std::endl;
      TESTER_ASSERT( r.learningError < 0.4 );
      TESTER_ASSERT( r.testingError < 0.07 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNaiveBayesClassifier);
TESTER_TEST(test1);
TESTER_TEST(test2Pca);
TESTER_TEST_SUITE_END();
#endif