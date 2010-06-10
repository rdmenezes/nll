#ifndef NLL_TUTORIAL_FEATURE_PCA_H_
# define NLL_TUTORIAL_FEATURE_PCA_H_

#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
   /**
    @ingroup tutorial
    @brief Feature transformation using PCA tutorial

    The goals of this tutorial are to:
    - use a feature transformation algorithm on a database
    - modify the database according to the selected features
    - train and test a classifier on this database
    */
   struct TutorialFeaturePca
   {
      typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
      typedef nll::algorithm::Classifier<Input>                            Classifier;
      typedef nll::algorithm::FeatureTransformationPca<Input>              Pca;

      void featurePca()
      {
         // find the database
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "cancer1.dt" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // create the Pca facility
         // we want to express the database in a base of N best vectors (regarding the reconstruction error)
         // The Pca is setted up, and the new database is processed
         Pca pca;
         pca.compute( dat, 2 );
         Classifier::Database processedDat = pca.transform( dat );

         // define the classifier to be used
         // a SVM classifier is used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         // the classifier is trained with the best parameter found in the <b> classifier optimization tutorial</b>
         c.learnTrainingDatabase( processedDat, nll::core::make_buffer1D<double>( 16.7772, 1 ) );
         Classifier::Result results = c.test( processedDat );

         // no error at all is achieved!
         TESTER_ASSERT( results.testingError <= 0.0 );
      }
   };
}
}

#endif