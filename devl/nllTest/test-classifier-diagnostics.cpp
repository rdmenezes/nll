#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


namespace nll
{
namespace algorithm
{
   /**
    @brief This is a helper class to diagnose classifiers and try to help determine the bias/variance problem
    */
   class ClassifierDiagnostics
   {
   public:
      struct Result
      {
         Result()
         {}

         Result( double nbS, double errTrain, double errTest ) : nbSamplesRatio( nbS ), errorTraining( errTrain ), errorTesting( errTest )
         {}

         ui32     nbSamplesRatio;
         double   errorTraining;
         double   errorTesting;
      };
      typedef std::vector<Result> Results;

      ClassifierDiagnostics( ui32 granularity = 20 ) : _granularity( granularity )
      {}

      template <class Classifier, class Database>
      Results evaluate( const Classifier& csource, const core::Buffer1D<double>& params, const Database& dat ) const
      {
         Results results;
         results.reserve( _granularity );

         Database learning = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );
         Database testing = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::TESTING ), (ui32) Database::Sample::TESTING );
         ensure( learning.size() != 0, "no learning!" );
         ensure( testing.size() != 0, "no testing!" );

         const f32 stepTesting = testing.size() / _granularity;
         const f32 stepLearning = learning.size() / _granularity;
         for ( ui32 n = 0; n < _granularity; ++n )
         {
            std::shared_ptr<Classifier> classifier( csource.deepCopy() );
            const ui32 nbLearningSample = static_cast<ui32>( n * stepLearning * learning.size() );
            const ui32 nbTestingSample = static_cast<ui32>( n * stepTesting * testing.size() );
            Database learningDatSampled = sample( learning, ( nbLearningSample > 1 ) ? nbLearningSample : 1 );
            Database testingDatSampled = sample( testing, ( nbTestingSample > 1 ) ? nbTestingSample : 1 );

            classifier->learn( learningDatSampled, params );
            typename Classifier::Result resultsLearning = classifier->test( learningDatSampled );
            typename Classifier::Result resultsTesting = classifier->test( testingDatSampled );
            results.push_back( Result( 1.0 / _granularity * n, resultsLearning.learningError, resultsTesting.testingError ) );
         }

         return results;
      }

      static core::Image<ui8> plot( const Results& r )
      {
         core::Image<ui8> i;
         return i;
      }

   private:
      template <class Database>
      static Database sample( const Database& dat, ui32 nbSamples )
      {
         Database resampled;
         const std::vector<float> pbSample( dat.size(), 1.0f / dat.size() );
         const core::Buffer1D<ui32> samples = core::sampling( pbSample, nbSamples );   // TODO: here we are sampling with replacement, better do it without!
         std::for_each( samples.begin(), samples.end(),
            [&]( ui32 s )
            { 
               resampled.add( dat[ s ] );
            } );
         return resampled;
      }

   private:
      ui32     _granularity;
   };
}
}

struct TestClassifierDiagnostics
{
   void test()
   {
      typedef std::vector<double> Point;
      typedef nll::algorithm::MetricEuclidian<Point> Metric;
      typedef nll::algorithm::ClassifierNearestNeighbor<Point, Metric >   Classifier;
      typedef Classifier::Database Database;
      
      Metric metric;
      Database dat;
      Classifier c( &metric );
      algorithm::ClassifierDiagnostics diagnostics;
      core::Buffer1D<f64> params = core::make_buffer1D<f64>( 3 );
      diagnostics.evaluate( c, params, dat );
   }

   void testResamplingWithoutReplacement()
   {
      for ( ui32 n = 0; n < 100; ++n )
      {
         ui32 nbCases = (ui32)core::generateUniformDistributioni( 1, 10 );
         ui32 nbSamples = (ui32)core::generateUniformDistributioni( 1, nbCases );
         core::Buffer1D<ui32> samples = core::samplingWithoutReplacement( nbCases, nbSamples );

         std::set<ui32> indexes;
         for ( ui32 n = 0; n < samples.size(); ++n )
         {
            std::set<ui32>::const_iterator it = indexes.find( samples[ n ] );
            TESTER_ASSERT( it == indexes.end() );
            indexes.insert( samples[ n ] );
         }

         samples.print( std::cout );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestClassifierDiagnostics);
 TESTER_TEST(testResamplingWithoutReplacement);
 TESTER_TEST(test);
TESTER_TEST_SUITE_END();
#endif