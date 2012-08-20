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

         size_t     nbSamplesRatio;
         double   errorTraining;
         double   errorTesting;
      };
      typedef std::vector<Result> Results;

      ClassifierDiagnostics( size_t granularity = 20 ) : _granularity( granularity )
      {}

      template <class Classifier, class Database>
      Results evaluate( const Classifier& csource, const core::Buffer1D<double>& params, const Database& dat ) const
      {
         Results results;
         results.reserve( _granularity );

         Database learning = core::filterDatabase( dat, core::make_vector<size_t>( (size_t) Database::Sample::LEARNING ), (size_t) Database::Sample::LEARNING );
         Database testing = core::filterDatabase( dat, core::make_vector<size_t>( (size_t) Database::Sample::TESTING ), (size_t) Database::Sample::TESTING );
         ensure( learning.size() != 0, "no learning!" );
         ensure( testing.size() != 0, "no testing!" );

         const f32 stepTesting = testing.size() / _granularity;
         const f32 stepLearning = learning.size() / _granularity;
         for ( size_t n = 0; n < _granularity; ++n )
         {
            std::shared_ptr<Classifier> classifier( csource.deepCopy() );
            const size_t nbLearningSample = static_cast<size_t>( n * stepLearning * learning.size() );
            const size_t nbTestingSample = static_cast<size_t>( n * stepTesting * testing.size() );
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
      static Database sample( const Database& dat, size_t nbSamples )
      {
         Database resampled;
         const std::vector<float> pbSample( dat.size(), 1.0f / dat.size() );
         const core::Buffer1D<size_t> samples = core::sampling( pbSample, nbSamples );   // TODO: here we are sampling with replacement, better do it without!
         std::for_each( samples.begin(), samples.end(),
            [&]( size_t s )
            { 
               resampled.add( dat[ s ] );
            } );
         return resampled;
      }

   private:
      size_t     _granularity;
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
      for ( size_t n = 0; n < 100; ++n )
      {
         size_t nbCases = (size_t)core::generateUniformDistributioni( 1, 10 );
         size_t nbSamples = (size_t)core::generateUniformDistributioni( 1, nbCases );
         core::Buffer1D<size_t> samples = core::samplingWithoutReplacement( nbCases, nbSamples );
         TESTER_ASSERT( samples.size() == nbSamples );

         std::set<size_t> indexes;
         for ( size_t n = 0; n < samples.size(); ++n )
         {
            std::set<size_t>::const_iterator it = indexes.find( samples[ n ] );
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
 //TESTER_TEST(test);
TESTER_TEST_SUITE_END();
#endif