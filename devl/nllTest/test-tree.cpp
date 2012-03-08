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

   /**
    @brief Generic factory with no parameters
    */
   template <class Metric>
   class FactoryGeneric
   {
   public:
      typedef Metric value_type;

      static Metric create()
      {
         return Metric;
      }
   };


   /**
    @brief Defines how a node should be splitted
    */
   template <class Database>
   class TreeNodeSplit
   {
   public:
      typedef typename Database::Sample::Input::value_type  value_type;
      typedef typename Database::Sample::Input              Point;

   public:
      virtual void compute( const Database& dat, std::vector<Database>& split_out ) = 0;
      virtual ui32 test( const Point& p ) const = 0;

      virtual ~TreeNodeSplit()
      {}
   };

   /**
    @brief Create discrete node decision split
    */
   template <class Database, class Metric>
   class TreeNodeSplitDiscrete : public TreeNodeSplit<Database>
   {
      typedef std::map<ui32, ui32>  AttributMapper;

   public:
      virtual void compute( const Database& dat, std::vector<Database>& split_out )
      {
         if ( dat.size() == 0 )
            return;
         const ui32 nbFeatures = dat[ 0 ].input.size();

         std::vector<ui32> y( dat.size() );
         std::vector<ui32> x( dat.size() );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            y[ n ] = dat[ n ].output;
         }

         // find the best feature
         double bestSplitMetric = std::numeric_limits<double>::min();
         for ( ui32 feature = 0; feature < nbFeatures; ++feature )
         {
            Metric metric;

            for ( ui32 n = 0; n < dat.size(); ++n )
            {
               x[ n ] = static_cast<ui32>( dat[ n ].input[ feature ] );
            }

            const double splitMetric = metric.compute( x, y );
            if ( splitMetric > bestSplitMetric )
            {
               bestSplitMetric = splitMetric;
               _featureId = feature;
            }
         }

         // here we are generating a mapper feature value -> bin id
         ui32 id = 0;
         AttributMapper attributMapper;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const ui32 featureValue = static_cast<ui32>( dat[ n ].input[ _featureId ] );
            AttributMapper::const_iterator it = attributMapper.find( dat[ n ].input[ featureValue ] );
            if ( it == attributMapper.end() )
            {
               attributMapper[ featureValue ] = id++;
            }
         }

         _attributMapper = attributMapper;

         // finally route the samples according to the split rule
         split_out = std::vector<Database>( _attributMapper.size() );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const ui32 branch = test( dat[ n ].input );
            split_out[ branch ].add( dat[ n ] );
         }
      }

      virtual ui32 test( const Point& p ) const
      {
         const ui32 featureValue = static_cast<ui32>( dat[ n ].input[ _featureId ] );
         AttributMapper::const_iterator it = _attributMapper.find( dat[ n ].input[ featureValue ] );
         ensure( it != _attributMapper.end(), "this feature value was never encountered during training! Can't classify" );
         return it->second;
      }

   private:
      ui32                 _featureId;          // the feature we are splitting
      AttributMapper       _attributMapper;     // map a feature ID to a bin
   };

   /**
    @brief Decision node for continuous attributs. A single split will be produced

    @param ContinuousSplittingCriteria the splitting criteria to be used for the continuous attributs
    @param Metric the metric to be used to select the best split, the higher, the better
    */
   template <class Database, class Metric, class SplittingCriteria>
   class TreeNodeSplitContinuousSingle : public TreeNodeSplit<Database>
   {
   public:
      typedef TreeNodeSplit<Database>                       Base;

      TreeNodeSplitContinuousSingle() : _thresold( std::numeric_limits<value_type>::min() ), _featureId( (ui32)-1 )
      {}

      /**
       @brief Compute the best split
       @param dat the database to use
       @param 
       */
      void compute( const Database& dat, std::vector<Database>& split_out )
      {
         if ( dat.size() == 0 )
            return;
         const ui32 nbFeatures = dat[ 0 ].input.size();

         std::vector<ui32> y( dat.size() );
         std::vector<ui32> x( dat.size() );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            y[ n ] = dat[ n ].output;
         }

         // select the best feature
         double bestSplitMetric = std::numeric_limits<double>::min();
         for ( ui32 feature = 0; feature < nbFeatures; ++feature )
         {
            Metric metric;
            SplittingCriteria splitter;

            std::vector<value_type> splits;
            splitter.computeSplits( dat, feature, splits );

            // then select the best split
            for ( size_t split = 0; split < splits.size(); ++split )
            {
               for ( ui32 n = 0; n < dat.size(); ++n )
               {
                  x[ n ] = dat[ n ].input[ feature ] >= splits[ split ];
               }

               // check the split quality
               const double splitMetric = metric.compute( x, y );
               if ( splitMetric > bestSplitMetric )
               {
                  bestSplitMetric = splitMetric;
                  _featureId = feature;
                  _thresold = splits[ split ];
               }
            }
         }

         // finally route the samples according to the split
         split_out = std::vector<Database>( 2 );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const ui32 branch = test( dat[ n ].input );
            split_out[ branch ].add( dat[ n ] );
         }
      }

      /**
       @brief return the branch to be taken
       */
      ui32 test( const Point& p ) const
      {
         return dat[ n ].input[ _featureId ] >= _thresold;
      }

   private:
      ui32                 _featureId;    // the feature we are splitting
      value_type           _thresold;     // the threshold used
   };

   
}
}

class TestTree
{
public:
   class Problem1
   {
   public:
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

      std::vector< std::vector<ui32> > create1() const
      {
         std::vector< std::vector<ui32> > days;
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

         return days;
      }

      std::vector< std::vector<ui32> > create2() const
      {
         std::vector< std::vector<ui32> > days;
         days.push_back( core::make_vector<ui32>( RAIN,     MILD, HIGH,    WEAK ) );      // 4
         days.push_back( core::make_vector<ui32>( RAIN,     COOL, NORMAL,  WEAK ) );      // 5
         days.push_back( core::make_vector<ui32>( RAIN,     COOL, NORMAL,  STRONG ) );    // 6
         days.push_back( core::make_vector<ui32>( RAIN,     MILD, NORMAL,  WEAK) );       // 10
         days.push_back( core::make_vector<ui32>( RAIN,     MILD, HIGH,    STRONG) );     // 14

         return days;
      }
   };

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
      Problem1 pb1;

      {
         std::vector< std::vector<ui32> > days = pb1.create1();

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

         std::vector<double> ref = core::make_vector<double>( 0.24675, 0.0292226, 0.151836, 0.048127 );
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
            TESTER_ASSERT( core::equal( g, ref[ n ], 1e-4 ) );
         }
      }

      {
         std::vector< std::vector<ui32> > days = pb1.create2();

         std::vector< ui32 > y( 5 );
         y[ 0 ] = 1;
         y[ 1 ] = 1;
         y[ 2 ] = 0;
         y[ 3 ] = 1;
         y[ 4 ] = 0;

         std::vector<double> ref = core::make_vector<double>( 0, 0.0199731, 0.0199731, 0.970951 );
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
            TESTER_ASSERT( core::equal( g, ref[ n ], 1e-4 ) );
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTree);
TESTER_TEST(testEntropy);
TESTER_TEST(testErf);
TESTER_TEST(testTree)
TESTER_TEST_SUITE_END();
#endif