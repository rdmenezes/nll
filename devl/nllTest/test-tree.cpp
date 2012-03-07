#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


namespace nll
{
namespace algorithm
{
   class AttributSplitter
   {
   };

   /**
    @brief Computes entropy for building trees
    @see www.cs.cmu.edu/~guestrin/Class/10701-S06/Handouts/recitations/recitation-decision_trees-adaboost-02-09-2006.ppt
         Information Gain,Decision Trees and Boosting, 10-701 ML recitation 9 Feb 2006
    */
   class Entropy
   {
   public:
      /**
       @brief computes the entropy of a vector of integrals
       @param v a set of integers
       */
      template <class Vector>
      double compute( const Vector& v ) const
      {
         typedef typename Vector::value_type value_type;

         STATIC_ASSERT( core::IsIntegral<value_type>::value ); // this implementation only works for integral type

         value_type min = std::numeric_limits<value_type>::max();
         value_type max = std::numeric_limits<value_type>::min();
         for ( ui32 n = 0; n < v.size(); ++n )
         {
            min = std::min( min, v[ n ] );
            max = std::max( max, v[ n ] );
         }

         const ui32 range = static_cast<ui32>( max - min ) + 1;
         std::vector<ui32> counts( range );
         for ( ui32 n = 0; n < v.size(); ++n )
         {
            ui32 c = static_cast<ui32>( v[ n ] - min );
            ++counts[ c ];
         }

         double entropy = 0;
         for ( size_t n = 0; n < counts.size(); ++n )
         {
            if ( counts[ n ] )
            {
               const double p = static_cast<double>( counts[ n ] ) / v.size();
               entropy -= p * core::log2( p );
            }
         }

         return entropy;
      }

      /**
       @brief Computes the conditional entropy H(y|x)
       @note values contained by x and y must be as close to zero as possible! (else arrays with extra padding are created)

       H(Y|X) = sum_i p(X=vi) H(Y|X=vi)
       */
      template <class Vector1, class Vector2>
      double compute( const Vector1& x, const Vector2& y ) const
      {
         typedef typename Vector1::value_type value_type1;
         typedef typename Vector2::value_type value_type2;

         STATIC_ASSERT( core::IsIntegral<value_type1>::value ); // this implementation only works for integral type
         STATIC_ASSERT( core::IsIntegral<value_type2>::value ); // this implementation only works for integral type
         ensure( x.size() == y.size(), "must be the same size" );

         value_type1 max = std::numeric_limits<value_type1>::min();
         for ( ui32 n = 0; n < x.size(); ++n )
         {
            max = std::max( max, x[ n ] );
         }

         std::vector<ui32> counts( max + 1 );
         ui32 yes = 0;
         ui32 no = 0;
         for ( ui32 n = 0; n < x.size(); ++n )
         {
            ui32 i = static_cast<ui32>( x[ n ] );
            ++counts[ i ];
         }

         std::vector< std::vector< value_type2 > > cond( max + 1 );
         for ( size_t n = 0; n < cond.size(); ++n )
         {
            cond[ n ].reserve( counts[ n ] );
         }

         for ( ui32 n = 0; n < x.size(); ++n )
         {
            ui32 i = static_cast<ui32>( x[ n ] );
            cond[ i ].push_back( y[ n ] );
         }

         double entropy = 0;
         for ( size_t n = 0; n < cond.size(); ++n )
         {
            if ( counts[ n ] )
            {
               const double e = compute( cond[ n ] );
               entropy += static_cast<double>( counts[ n ] ) / x.size() * e;
            }
         }

         return entropy;
      }
   };

   /**
    @brief Computes information gain on an integral input/output
    */
   class InformationGain
   {
   public:
      /**
       @brief Compute the information gain IG(y|x) = E(y) - E(y|x)

       It can be seen as the amount of information saved by knowing x when transmiting y
       */
      template <class Vector1, class Vector2>
      double compute( const Vector1& x, const Vector2& y ) const
      {
         Entropy entropy;
         return entropy.compute( y ) - entropy.compute( x, y );
      }
   };

   template <class DatabaseInput, class SplittingCriteria>
   class NodeContinuousSingleSplit
   {
   public:
      typedef typename DatabaseInput::Sample::Input::value_type value_type;

      NodeContinuousSingleSplit() : _direction( 1 ), _thresold( std::numeric_limits<value_type>::min() )
      {}

   private:
      ui32        _featureId;    // the feature we are splitting
      value_type  _thresold;     // the threshold used
      int         _direction;    // the direction pointed 1: value > _thresold ? class = 1 : class = 0
                                 //                       0: value > _thresold ? class = 0 : class = 1
   };

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
          "Efficient Determination of Dynamic Split Points in a Decision Tree", D. Chickering, C. Meek, R. Rounthwaite
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

   /**
    @brief Implementation of split points in decision tree
    @see http://research.microsoft.com/pubs/65569/splits.pdf
          "Effcient Determination of Dynamic Split Points in a Decision Tree", D. Chickering, C. Meek, R. Rounthwaite
    */
   template <class DatabaseT>
   class SplittingCriteriaUniformApproximation : SplittingCriteria<DatabaseT>
   {
   public:
      SplittingCriteriaUniformApproximation( ui32 nbSplits = 10 ) : _nbSplits
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
         value_type max = std::numeric_limits<value_type>::min();
         value_type min = std::numeric_limits<value_type>::max();
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            min = std::min( min, dat[ n ].input[ featureId ] );
            max = std::max( max, dat[ n ].input[ featureId ] );
         }

         // now get all the splits
         outSplits.reserve( _nbSplits );
         for ( ui32 n = 0; n < _nbSplits; ++n )
         {
            const value_type split = min + n * ( max - min ) / ( _nbSplits + 1 );
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

   void testEntropy()
   {
      algorithm::Entropy entropy;

      {
         std::vector<int> v = core::make_vector<int>( 1, 1, 0, 1, 0 );
         const double e = entropy.compute( v );
         TESTER_ASSERT( core::equal<double>( e, 0.971, 0.001 ) );
      }

      {
         std::vector<int> x = core::make_vector<int>( 0, 1, 2, 0, 0, 2, 1, 0 );
         std::vector<int> y = core::make_vector<int>( 1, 0, 1, 0, 0, 1, 0, 1 );
         const double e = entropy.compute( x, y );
         TESTER_ASSERT( core::equal<double>( e, 0.5, 0.001 ) );
      }
   }

   void testTree()
   {
      enum Outlook
      {
         SUNNY,
         OVERCAST,
         RAIN
      };

      enum Temperature
      {
         HOT,
         MILD,
         COOL
      };

      enum Humidity
      {
         HIGH,
         NORMAL
      };

      enum WIND
      {
         WEAK,
         STRONG
      };

      enum PLAY
      {
         NO,
         YES
      };

      std::vector< std::vector<ui32> > days;
      /*
      days.push_back( core::make_vector<ui32>( SUNNY,    HOT,  HIGH,    WEAK ) );      // 1
      days.push_back( core::make_vector<ui32>( SUNNY,    HOT,  HIGH,    STRONG ) );    // 2
      days.push_back( core::make_vector<ui32>( OVERCAST, HOT,  HIGH,    WEAK ) );      // 3
      days.push_back( core::make_vector<ui32>( RAIN,     MILD, HIGH,    WEAK ) );      // 4
      days.push_back( core::make_vector<ui32>( RAIN,     COOL, NORMAL,  WEAK ) );      // 5
      days.push_back( core::make_vector<ui32>( RAIN,     COOL, NORMAL,  STRONG ) );    // 6
      days.push_back( core::make_vector<ui32>( OVERCAST, COOL, NORMAL,  STRONG ) );    // 7
      days.push_back( core::make_vector<ui32>( SUNNY,    MILD, HIGH,    WEAK) );       // 8
      days.push_back( core::make_vector<ui32>( SUNNY,    COOL, NORMAL,  WEAK) );       // 9
      days.push_back( core::make_vector<ui32>( RAIN,     MILD, NORMAL,  WEAK) );       // 10
      days.push_back( core::make_vector<ui32>( SUNNY,    MILD, NORMAL,  STRONG) );     // 11
      days.push_back( core::make_vector<ui32>( OVERCAST, MILD, HIGH,    STRONG) );     // 12
      days.push_back( core::make_vector<ui32>( OVERCAST, HOT,  NORMAL,  WEAK) );       // 13
      days.push_back( core::make_vector<ui32>( RAIN,     MILD, HIGH,    STRONG) );     // 14
      */

      days.push_back( core::make_vector<ui32>( RAIN,     MILD, HIGH,    WEAK ) );      // 4
      days.push_back( core::make_vector<ui32>( RAIN,     COOL, NORMAL,  WEAK ) );      // 5
      days.push_back( core::make_vector<ui32>( RAIN,     COOL, NORMAL,  STRONG ) );    // 6
      days.push_back( core::make_vector<ui32>( RAIN,     MILD, NORMAL,  WEAK) );       // 10
      days.push_back( core::make_vector<ui32>( RAIN,     MILD, HIGH,    STRONG) );     // 14

      std::vector< ui32 > y( 5 );
      y[ 0 ] = 1;
      y[ 1 ] = 1;
      y[ 2 ] = 0;
      y[ 3 ] = 1;
      y[ 4 ] = 0;
      
      /*
      std::vector< ui32 > y( 14 );
      y[ 0 ] = 0;
      y[ 1 ] = 0;
      y[ 2 ] = 1;
      y[ 3 ] = 1;
      y[ 4 ] = 1;
      y[ 5 ] = 0;
      y[ 6 ] = 1;
      y[ 7 ] = 0;
      y[ 8 ] = 1;
      y[ 9 ] = 1;
      y[ 10 ] = 1;
      y[ 11 ] = 1;
      y[ 12 ] = 1;
      y[ 13 ] = 0;
      */


      for ( ui32 n = 0; n < days[ 0 ].size(); ++n )
      {
         std::vector<ui32> x;
         for ( ui32 d = 0; d < days.size(); ++d )
         {
            x.push_back( days[ d ][ n ] );
         }
         algorithm::InformationGain gain;
         const double g = gain.compute( x, y );
         std::cout << "gain[" << n << "]=" << g << std::endl;
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTree);
TESTER_TEST(testEntropy);
/*TESTER_TEST(testErf);*/
TESTER_TEST(testTree)
TESTER_TEST_SUITE_END();
#endif