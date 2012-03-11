#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


namespace nll
{
namespace algorithm
{
   /**
    @brief Generic decision tree
    */
   template <class Database>
   class DecisionTree
   {
   public:
      typedef typename Database::Sample::Input::value_type  value_type;
      typedef typename Database::Sample::Input              Point;
      typedef typename Database::Sample::Output             Class;
      typedef TreeNodeSplit<Database>                       NodeSplit;

      struct LevelData
      {
         LevelData( ui32 d, const Database&   dd ) : depth( d ), data( dd )
         {}

         ui32              depth;
         const Database&   data;
      };

      /**
       @brief Control the growing strategy of a tree
       */
      class GrowingStrategy
      {
      public:
         /**
          @brief returns true if the tree should continue growing
          */
         virtual bool continueGrowth( const LevelData& data ) const = 0;

         /**
          @brief returns the class of a current node
          */
         virtual Class getNodeClass( const Database& dat ) const = 0;
      };

      /**
       @brief grow a tree out of a database
       @param dat input database
       @param nodeFactory the factory that will create each decision node
       @param growingStrategy the strategy that will control how the tree is growing
       */
      template <class NodeFactory>
      void compute( const Database& dat, const NodeFactory& nodeFactory, const GrowingStrategy& growingStrategy )
      {
         _compute( dat, nodeFactory, growingStrategy, 0 );
      }

      /**
       @brief get the class of this point
       */
      ui32 test( const Point& p ) const
      {
         if ( _nodes.size() == 0 )
         {
            // we are at the leaf...
            return _nodeClass;
         }

         ui32 nodeId = _split->test( p );
         return _nodes[ nodeId ].test( p );
      }

   private:
      template <class NodeFactory>
      void _compute( const Database& dat, const NodeFactory& nodeFactory, const GrowingStrategy& growingStrategy, ui32 level )
      {
         LevelData ld( level, dat );
         const bool continueGrowth = growingStrategy.continueGrowth( dat );

         if ( !continueGrowth )
         {
            _nodeClass = growingStrategy.getNodeClass( dat );
         }

         std::vector<Database> dats;
         _split = std::shared_ptr<NodeSplit>( new NodeSplit( nodeFactory.create() ) );
         _split->compute( dat, dats );

         _nodes = std::vector<DecisionTree>( dats.size() );
         for ( size_t n = 0; n < dats.size(); ++n )
         {
            _nodes->_compute( dats[ n ], nodeFactory, growingStrategy, level + 1 );
         }
      }

   private:
      std::vector<DecisionTree>     _nodes;           // the nodes at n+1 level
      std::shared_ptr<NodeSplit>    _split;           // the decision split
      Class                         _nodeClass;       // if this node is a leaf, return this class
   };

   /**
    @brief Grow a fixed depth tree
    */
   template <class Database>
   class GrowingStrategyFixedDepth : public DecisionTree<Database>::GrowingStrategy
   {
   public:
      typedef typename DecisionTree<Database>::LevelData LevelData;
      typedef typename DecisionTree<Database>::Class     Class;

   public:
      GrowingStrategyFixedDepth( ui32 maxDepth = std::numeric_limits<ui32>::max() ) : _maxDepth( maxDepth )
      {}

      /**
       @brief returns true if the tree should continue growing
       */
      virtual bool continueGrowth( const LevelData& data ) const
      {
         return data.depth <= _maxDepth;
      }

      /**
       @brief returns the class of a current node
       */
      virtual Class getNodeClass( const Database& dat ) const
      {
         ui32 nbClasses = core::getNumberOfClass( dat );
         std::vector<ui32> counts( nbClasses );

         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            ++counts[ dat[ n ].output ];
         }

         std::vector<ui32>::const_iterator it = std::max_element( counts.begin(), counts.end() );
         return static_cast<Class>( it - counts.end() );
      }

   private:
      ui32  _maxDepth;
   };
}
}

class TestTree
{
public:
   class Problem1
   {
   public:
      typedef core::Database<core::ClassificationSample<std::vector<ui32>, ui32>> Database;

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

      std::vector< std::vector<ui32> > create1( std::vector<ui32>& yout ) const
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
         yout = y;

         return days;
      }

      Database createDatabase1() const
      {
         std::vector<ui32> y;
         std::vector< std::vector<ui32> > x = create1( y );
         return createDatabase( x, y );
      }

      Database createDatabase( const std::vector< std::vector<ui32> >& x, std::vector<ui32>& y ) const
      {
         Database dat;
         for ( size_t n = 0; n < x.size(); ++n )
         {
            dat.add( Database::Sample( x[ n ], y[ n ], Database::Sample::LEARNING ) );
         }

         return dat;
      }

      std::vector< std::vector<ui32> > create2( std::vector<ui32>& yout ) const
      {
         std::vector< std::vector<ui32> > days;
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

         yout = y;
         return days;
      }

      Database createDatabase2() const
      {
         std::vector<ui32> y;
         std::vector< std::vector<ui32> > x = create2( y );
         return createDatabase( x, y );
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
         std::vector<ui32> y;
         std::vector< std::vector<ui32> > days = pb1.create1( y );


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
         std::vector<ui32> y;
         std::vector< std::vector<ui32> > days = pb1.create2( y );

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

   void testSplitNodeDiscrete()
   {
      typedef Problem1::Database Database;
      algorithm::TreeNodeSplitDiscrete<Database, algorithm::InformationGain> d;

      Problem1 pb1;
      Database dat = pb1.createDatabase1();

      std::vector<Database> dats;
      d.compute( dat, dats );
      TESTER_ASSERT( d.getFeatureSplit() == 0 );

      size_t size = 0;
      std::for_each( dats.begin(), dats.end(), [&]( const Database& d ){ size += d.size(); } );

      TESTER_ASSERT( dats.size() == 3 );
      TESTER_ASSERT( size == dat.size() );
      for ( size_t n = 0; n < dats.size(); ++n )
      {
         for ( size_t nn = 0; nn < dats[ n ].size(); ++nn )
         {
            TESTER_ASSERT( dats[ n ][ nn ].input[ d.getFeatureSplit() ] == n );
         }
      }
   }

   void testSplitNodeDiscrete2()
   {
      typedef Problem1::Database Database;
      algorithm::TreeNodeSplitDiscrete<Database, algorithm::InformationGain> d;

      Problem1 pb2;
      Database dat = pb2.createDatabase2();

      std::vector<Database> dats;
      d.compute( dat, dats );
      TESTER_ASSERT( d.getFeatureSplit() == 3 );

      size_t size = 0;
      std::for_each( dats.begin(), dats.end(), [&]( const Database& d ){ size += d.size(); } );

      TESTER_ASSERT( dats.size() == 2 );
      TESTER_ASSERT( size == dat.size() );
      for ( size_t n = 0; n < dats.size(); ++n )
      {
         for ( size_t nn = 0; nn < dats[ n ].size(); ++nn )
         {
            TESTER_ASSERT( dats[ n ][ nn ].input[ d.getFeatureSplit() ] == n );
         }
      }
   }

   class Problem2
   {
   public:
      typedef core::Database< core::ClassificationSample< std::vector< double >, ui32 > > Database;

      Database create1() const
      {
         Database d;

         d.add( Database::Sample( core::make_vector<double>( 0.1 ), 0, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 0.5 ), 0, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 0.8 ), 0, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 1.1 ), 1, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 2.1 ), 1, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 5.1 ), 1, Database::Sample::LEARNING )  );

         return d;
      }


      Database create2() const
      {
         Database d;

         d.add( Database::Sample( core::make_vector<double>( 0, 100.1 ), 0, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 0.1, 100.5 ), 0, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 0.1, 100.8 ), 0, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 0.1, 101.1 ), 1, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 0, 102.1 ), 1, Database::Sample::LEARNING )  );
         d.add( Database::Sample( core::make_vector<double>( 0.1, 105.1 ), 1, Database::Sample::LEARNING )  );

         return d;
      }
   };

   void testSplitNodeContinuous()
   {
      Problem2 pb2;
      Problem2::Database dat = pb2.create1();

      typedef algorithm::SplittingCriteriaUniformApproximation<Problem2::Database> SplittingCriteria;
      typedef core::FactoryGeneric<SplittingCriteria> SplittingCriteriaFactory;
      typedef algorithm::TreeNodeSplitContinuousSingle<Problem2::Database, algorithm::InformationGain, SplittingCriteriaFactory> NodeSplit;

      std::vector<Problem2::Database> dats;
      SplittingCriteriaFactory f;
      NodeSplit n1( f );
      n1.compute( dat, dats );

      TESTER_ASSERT( dats.size() == 2 );
      TESTER_ASSERT( n1.getFeatureSplit() == 0 );
      TESTER_ASSERT( core::equal( n1.getSplitThreshold(), 1.0, 0.01 ) );

      for ( size_t n = 0; n < dats.size(); ++n )
      {
         for ( size_t nn = 0; nn < dats[ n ].size(); ++nn )
         {
            TESTER_ASSERT( dats[ n ][ nn ].output == n );
         }
      }
   }

   void testSplitNodeContinuous2()
   {
      Problem2 pb2;
      Problem2::Database dat = pb2.create1();

      typedef algorithm::SplittingCriteriaGaussianApproximation<Problem2::Database> SplittingCriteria;
      typedef core::FactoryGeneric<SplittingCriteria> SplittingCriteriaFactory;
      typedef algorithm::TreeNodeSplitContinuousSingle<Problem2::Database, algorithm::InformationGain, SplittingCriteriaFactory> NodeSplit;

      std::vector<Problem2::Database> dats;
      SplittingCriteriaFactory f;
      NodeSplit n1( f );
      n1.compute( dat, dats );

      TESTER_ASSERT( dats.size() == 2 );
      TESTER_ASSERT( n1.getFeatureSplit() == 0 );

      for ( size_t n = 0; n < dats.size(); ++n )
      {
         for ( size_t nn = 0; nn < dats[ n ].size(); ++nn )
         {
            TESTER_ASSERT( dats[ n ][ nn ].output == n );
         }
      }
   }

   void testSplitNodeContinuous3()
   {
      Problem2 pb2;
      Problem2::Database dat = pb2.create2();

      typedef algorithm::SplittingCriteriaGaussianApproximation<Problem2::Database> SplittingCriteria;
      typedef core::FactoryGeneric<SplittingCriteria> SplittingCriteriaFactory;
      typedef algorithm::TreeNodeSplitContinuousSingle<Problem2::Database, algorithm::InformationGain, SplittingCriteriaFactory> NodeSplit;

      std::vector<Problem2::Database> dats;
      SplittingCriteriaFactory f;
      NodeSplit n1( f );
      n1.compute( dat, dats );

      TESTER_ASSERT( dats.size() == 2 );
      TESTER_ASSERT( n1.getFeatureSplit() == 1 );

      for ( size_t n = 0; n < dats.size(); ++n )
      {
         for ( size_t nn = 0; nn < dats[ n ].size(); ++nn )
         {
            TESTER_ASSERT( dats[ n ][ nn ].output == n );
         }
      }
   }

   void testDecisionTree()
   {
      Problem1 pb1;
      Problem1::Database dat = pb1.createDatabase1();

      algorithm::DecisionTree<Problem1::Database> dt;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTree);
TESTER_TEST(testEntropy);
TESTER_TEST(testErf);
TESTER_TEST(testTree);
TESTER_TEST(testSplitNodeDiscrete);
TESTER_TEST(testSplitNodeDiscrete2);
TESTER_TEST(testSplitNodeContinuous);
TESTER_TEST(testSplitNodeContinuous2);
TESTER_TEST(testSplitNodeContinuous3);
TESTER_TEST_SUITE_END();
#endif