#include <nll/nll.h>
#include <tester/register.h>
#include "database-builder.h"

# pragma warning( push )
# pragma warning( disable:4996 ) // unsecure function...
# pragma warning( disable:4189 ) // local variable not referenced
# pragma warning( disable:4101 ) // unreferenced

class TestNllClassifierSpect
{
public:
   typedef nll::core::Buffer1D<double>             Point;
   typedef nll::algorithm::ClassifierMlp<Point>    Mlp;
   typedef Mlp::Database                           Database;

   void testNllClassifierSpect()
   {
      Database dat = loadDatabaseSpect<Point>();
     

      srand((nll::ui32)time(0));
      Mlp::BaseClassifier* mlp = new Mlp();

       nll::core::Buffer1D<nll::f64> optimizedParameters = nll::core::make_buffer1D<nll::f64>(5, 0.001, 2);

      nll::algorithm::FeatureSelectionWrapper<Point>* fs = new nll::algorithm::FeatureSelectionBestFirst<Point>();
      nll::core::Buffer1D<bool> result = fs->compute( mlp, optimizedParameters, dat );

      Database newdat = fs->transform( dat );
      mlp->learn(newdat, optimizedParameters);
      const Mlp::Result resultTest = mlp->test(newdat);
      std::cout << "BestClassifierTestingErrorRate=" << resultTest.testingError << std::endl;
      TESTER_ASSERT( resultTest.testingError < 0.185 );
   }

   void testNllClassifierRbf()
   {
      srand( 0 );
      typedef nll::algorithm::ClassifierRbf<Point> Classifier;

      Database dat = loadDatabaseSpect<Point>();

      Classifier classifier;
      classifier.learn( dat, nll::core::make_buffer1D<double>( 40, 0.5, 15 ) );
      Classifier::Result result = classifier.test( dat );

      std::cout << "Rbf T=" << result.testingError << " L=" << result.learningError << std::endl;
      TESTER_ASSERT( result.testingError < 0.24 && result.learningError < 0.10 );
   }

   void testNllClassifierMlp()
   {
      srand( 0 );
      Database dat = loadDatabaseSpect<Point>();
      Mlp classifier;
      classifier.learn( dat, nll::core::make_buffer1D<double>( 20, 0.5, 10 ) );
      Mlp::Result result = classifier.test( dat );

      TESTER_ASSERT( result.testingError < 0.30 && result.learningError < 0.10 );
   }

   void testNllClassifierNaiveb()
   {
      typedef nll::algorithm::ClassifierNaiveBayes<Point> Classifier;

      Database dat = loadDatabaseSpect<Point>();
      Classifier classifier;
      classifier.learn( dat, nll::core::Buffer1D<double>() );
      Classifier::Result result = classifier.test( dat );

      TESTER_ASSERT( result.testingError < 0.13 && result.learningError < 0.27 );
   }

   void testNllPcaClassifierSpect()
   {
      typedef nll::algorithm::ClassifierSvm<Point> Classifier;

      Database dat = loadDatabaseSpect<Point>();
      nll::algorithm::FeatureTransformationPca<Point> pca;
      pca.compute( dat, 9 );
      Database datProcessed = pca.transform( dat );

      Classifier classifier;
      classifier.learn( datProcessed, nll::core::make_buffer1D<double>( 10, 100 ) );
      Classifier::Result result = classifier.test( datProcessed );

      TESTER_ASSERT( result.testingError < 0.097 );
   }

   void testNllQdaClassifierSpect()
   {
      typedef nll::algorithm::ClassifierSvm<Point> Classifier;

      Database dat = loadDatabaseSpect<Point>();
      nll::algorithm::FeatureTransformationDiscriminant<Point> qda;
      qda.compute( dat );
      Database datProcessed = qda.transform( dat );

      Classifier classifier;
      classifier.learn( datProcessed, nll::core::make_buffer1D<double>( 10, 100 ) );
      Classifier::Result result = classifier.test( datProcessed );

      std::cout << "QDA Test error=" << result.testingError << std::endl;
      std::cout << "QDA learning error=" << result.learningError << std::endl;
      TESTER_ASSERT( result.testingError < 0.102 );
   }

   void testNllPCAQdaClassifierSpect()
   {
      typedef nll::algorithm::ClassifierSvm<Point> Classifier;

      Database dat = loadDatabaseSpect<Point>();

      // QDA
      nll::algorithm::FeatureTransformationDiscriminant<Point> qda;
      qda.compute( dat );
      Database datProcessed1 = qda.transform( dat );

      // PCA
      nll::algorithm::FeatureTransformationPca<Point> pca;
      pca.compute( dat, 15 );
      Database datProcessed2 = pca.transform( dat );

      // Combine
      Database datProcessed3 = nll::algorithm::FeatureCombiner::transform( datProcessed1, datProcessed2 );

      // normalize
      nll::algorithm::FeatureTransformationNormalization<Point> normalization;
      normalization.compute( datProcessed3 );
      Database datProcessed = normalization.transform( datProcessed3 );

      // classification
      Classifier classifier;
      classifier.learn( datProcessed, nll::core::make_buffer1D<double>( 10, 100 ) );
      Classifier::Result result = classifier.test( datProcessed );

      std::cout << "QDA+PCA Test error=" << result.testingError << std::endl;
      std::cout << "QDA+PCA learning error=" << result.learningError << std::endl;
      TESTER_ASSERT( result.testingError < 0.102 );
   }

   void testNllClassifierSvmIca()
   {
      srand(time(0));
      typedef nll::algorithm::ClassifierSvm<Point> Classifier;

      Database dat = loadDatabaseSpect<Point>();
      nll::algorithm::FeatureTransformationIca<Point> pca;
      pca.compute( dat,21 );
      Database datProcessed = pca.transform( dat );

      Classifier classifier;
      classifier.learn( datProcessed, nll::core::make_buffer1D<double>( 10, 100 ) );
      Classifier::Result result = classifier.test( datProcessed );

      std::cout << "res=" << result.testingError << std::endl;
      TESTER_ASSERT( result.testingError < 0.1017 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllClassifierSpect);
# ifndef DONT_RUN_VERY_SLOW_TEST
#  ifndef DONT_RUN_SLOW_TEST
TESTER_TEST(testNllClassifierSpect);
#  endif
# endif
TESTER_TEST(testNllClassifierMlp);
TESTER_TEST(testNllClassifierNaiveb);
TESTER_TEST(testNllClassifierRbf);
TESTER_TEST(testNllPcaClassifierSpect);
TESTER_TEST(testNllQdaClassifierSpect);
TESTER_TEST(testNllPCAQdaClassifierSpect);
TESTER_TEST(testNllClassifierSvmIca);
TESTER_TEST_SUITE_END();
#endif

# pragma warning( pop )
