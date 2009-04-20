#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
   /**
    @ingroup tutorial
    @brief Feature selection tutorial

    The goals of this tutorial are to:
    - use a feature selection algorithm on a database
    - modify the database according to the selected features
    - train and test a classifier on this database
    */
   struct TutorialFeatureSelection
   {
      typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
      typedef nll::algorithm::Classifier<Input>                            Classifier;

      void featureSelection()
      {
         // find the database
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "cancer1.dt" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // create a feature selection algorithm (Relieff), and select the <code>Nth</code> best features
         typedef nll::algorithm::FeatureSelectionFilterRelieff<Input>   FeatureSelectionAlgorithm;
         FeatureSelectionAlgorithm  featureSelection( 3 );
         const nll::core::Buffer1D<bool> selected = featureSelection.compute( dat );

         // suppress the features that have not been selected by the feature selection algorithm
         Classifier::Database filteredDat = nll::core::filterDatabase( dat, selected );

         // define the classifier to be used
         // a SVM classifier is used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         // the classifier is trained with the best parameter found in the <b> classifier optimization tutorial</b>
         c.learnTrainingDatabase( filteredDat, nll::core::make_buffer1D<double>( 16.7772, 1 ) );
         Classifier::Result results = c.test( filteredDat );

         // we achieved exactly the same results with only 3 features!
         TESTER_ASSERT( results.testingError < 0.03 );
      }
   };
}
}
