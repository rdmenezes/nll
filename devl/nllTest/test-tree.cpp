#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


namespace nll
{
namespace algorithm
{
   

   /**
    @brief Implementation of split points in decision tree
    */
   template <class DatabaseT>
   class SplittingCriteria
   {
   public:
      typedef DatabaseT Database;
      typedef typename Database::Sample::Input::value_type     value_type;

   public:
      virtual ~SplittingCriteria()
      {}

      /**
       @brief Compute the splitting points for the considered feature
       @param dat the database to operate on
       @param featureId the feature considered
       @param outSplits the computed splits
       */
      virtual void computeSplits( const Database& dat, ui32 featureId, std::vector<value_type>& outSplits ) = 0;
   };

   /**
    @brief Implementation of split points in decision tree
    @see http://research.microsoft.com/pubs/65569/splits.pdf
          "Effcient Determination of Dynamic Split Points in a Decision Tree", D. Chickering, C. Meek, R. Rounthwaite
    */
   template <class DatabaseT>
   class SplittingCriteriaGaussianApproximation : SplittingCriteria<DatabaseT>
   {
   public:
      SplittingCriteriaGaussianApproximation( ui32 nbSplits = 10 ) : _nbSplits
      {
      }

      /**
       @brief Compute the splitting points for the considered feature
       @param dat the database to operate on
       @param featureId the feature considered
       @param outSplits the computed splits
       */
      virtual void computeSplits( const Database& dat, ui32 featureId, std::vector<value_type>& outSplits )
      {
         outSplits.clear();

         // here we fit a gaussian on the considered data
         value_type accumMean = 0;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            accumMean += dat[ n ].input[ featureId ];
         }
         accumMean /= dat.size();

         value_type accumStddev = 0;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            accumStddev += core::sqr( dat[ n ].input[ featureId ] - accumMean );
         }
         accumStddev = sqrt( accumStddev / dat.size() );

         // now get all the splits
         outSplits.reserve( _nbSplits );
         for ( ui32 n = 0; n < _nbSplits; ++n )
         {
            const value_type split = accumMean + accumStddev *
               static_cast<value_type>( core::CumulativeGaussianFunction::erfinv_lut( n / ( _nbSplits + 1 ) ) );
            outSplits.push_back( split );
         }
      }

   private:
      ui32  _nbSplits;
   };
}
}

class TestTree
{
public:
   void testErf()
   {
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_lut( 0 ), 0, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_lut( 0.2 ), 0.2227, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_lut( 0.6 ), 0.6039, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_lut( 0.8 ), 0.7421, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_lut( 4 ), 1, 1e-4 ) );

      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_approx1( 0 ), 0, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_approx1( 0.2 ), 0.2227, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_approx1( 0.6 ), 0.6039, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_approx1( 0.8 ), 0.7421, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erf_approx1( 4 ), 1, 1e-3 ) );

      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erfinv_lut( -0.999 ), -2.3268, 1e-4 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erfinv_lut( -0.9565 ), -1.4277, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erfinv_lut( -0.8261 ), -0.9615, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erfinv_lut( -0.6087 ), -0.6062, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erfinv_lut( -0.3913 ), -0.3620, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erfinv_lut( 0.0435 ), 0.0386, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erfinv_lut( 0.4783 ), 0.453, 1e-3 ) );
      TESTER_ASSERT( core::equal<double>( core::CumulativeGaussianFunction::erfinv_lut( 0.9130 ), 1.2103, 1e-3 ) );
   }

   void testSplittingGaussian()
   {
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTree);
TESTER_TEST(testErf);
TESTER_TEST_SUITE_END();
#endif