#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
   /**
    @ingroup tutorial
    @brief Efficient database manipulation using <code>Adapters</code>

    The goals of this tutorial are:
    - to manipulate a database efficiently by avoiding its copy using <code>Adapters</code>
    - to show how easy is to use low level algorithms (generic kmeans) directly on the database
    */
   struct TutorialPartitionDatabase
   {
      typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
      typedef nll::algorithm::Classifier<Input>                            Classifier;

      void partitionDatabase()
      {
         // find the database
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "cancer1.dt" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // work only on the LEARNING database
         // The Adapter is used to adapt the interface of the database to the basic interface of a vector (TODO: switch to concept with C++ 0x)
         // internally, the adapter holds a pointer on the original database and build an additional index to reference
         // the points we are interested in. Different kind of adapters can be used.
         typedef core::DatabaseInputAdapterType<Classifier::Database>   DatabaseLearningAdapter;
         DatabaseLearningAdapter learningDatabase( dat, core::make_vector<ui32>( Classifier::Database::Sample::LEARNING ) );

         // we will partition the learning database using a low level algorithm and the defined adapter.
         // The point type is the one contained by the adapter. We will use the 8 first features, partitionned in
         // 3 clusters using kmeans
         typedef algorithm::BuildKMeansUtility<DatabaseLearningAdapter::value_type, algorithm::MetricEuclidian<DatabaseLearningAdapter::value_type>, DatabaseLearningAdapter>  KmeansConfiguration;
         typedef algorithm::KMeans<KmeansConfiguration> KMeans1;

         // actually compute the partionning
         KmeansConfiguration::Metric metric;
         KmeansConfiguration config( 9, metric );
         KMeans1 kmeans( config );
         KMeans1::KMeansResult result = kmeans( learningDatabase, 3 );

         TESTER_ASSERT( result.first.size() == learningDatabase.size() );
         TESTER_ASSERT( result.second.size() == 3 );

         //
         // TODO add application for vizualisation (shammon's mapping)
         //
      }
   };
}
}