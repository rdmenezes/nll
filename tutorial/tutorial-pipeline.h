#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"

namespace nll
{
namespace tutorial
{
   /**
    @ingroup tutorial
    @brief Pipeline introduction tutorial

    The goals of this tutorial are:
    - to explain why having pipelines?
    - use of some pipeline components

    There are several layer of abstractions in <b>nll</b>. The default implementation (ie <code>algorithm</code>)
    of an algorithm where dependencies are kept minimal, then there is the <code>algorithm-api</code> which is
    an integrated version of the <code>algorithm-impl</code> and the last one, the <code>pipeline</code> layer.
    It tries to define a workflow from a raw stream of data to a processed input, directly usable by the classifier.

    <p>
    The <code>pipeline</code> provides some facilities for being optimized. A pipeline is defined by a sequence
    of operations and a classifier. For each operation, the output of one must be the input of the following.

    <p>
    The <code>pipeline</code> is defined at compile time so that input/output types of each operation is known,
    and better optimized. The pipeline is also strongly typed, so no mistakes can be done in the workflow.
    */
   struct TutorialPipeline
   {
      typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
      typedef nll::algorithm::Classifier<Input>                            Classifier;

      /**
       In this pipeline we will do 2 basic operations:
       - normalization of the database
       - feature extraction
       */
      void runPipeline()
      {
         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "cancer1.dt" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;

         // define the classifier to be used
         typedef nll::algorithm::ClassifierSvm<Input> ClassifierImpl;
         ClassifierImpl c;

         // define the preprocessing unit 1: feature normalization
         typedef nll::preprocessing::PreprocessingRawNormalize<Input>   Preprocessing1;

         // define the preprocessing unit 2: feature selection
         typedef nll::preprocessing::PreprocessingRawSelect<Input>      Preprocessing2;

         // define the pipeline, which is composed of 2 operations and 1 classifier. we have to use the macro
         // TYPELIST_2 that will create this specific pipeline.
         typedef TYPELIST_2(Preprocessing1, Preprocessing2, Classifier) PreprocessingPipeline1;

         // create a feature normalization operation
         Preprocessing1 p1;

         // a <code>PreprocessingOptimizationScheme</code> is created to record the actions to be done
         // for each operation of the pipeline
         nll::preprocessing::PreprocessingOptimizationScheme            optScheme;
         optScheme.add( p1.getId() );  // specify that this operation has to be optimized. The order matters, it will be the 1 optimized operation

         // select all features
         Preprocessing2::Vector v( 9 );
         for ( unsigned  n = 0; n < 9; ++n )
            v[ n ] = true;

         // specify the feature selection algorithm
         Preprocessing2::Options options( 1 );
         options[ 0 ] = Preprocessing2::RELIEFF;
         Preprocessing2 p2( 1, v, options );
         optScheme.add( p2.getId() );

         // create the pipeline
         PreprocessingPipeline1 pipeline1 = nll::preprocessing::make_typelist( p1, p2, reinterpret_cast<Classifier*>( &c ) );

         // optimize the pipeline
         pipeline1.optimize( dat, nll::core::make_buffer1D<double>( 16.7772, 1 ), optScheme );
      }
   };
}
}
