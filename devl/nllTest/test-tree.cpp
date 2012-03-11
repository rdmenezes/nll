#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


namespace nll
{
namespace algorithm
{
   /**
    @brief Computes entropy for building trees
    @see www.seas.harvard.edu/courses/cs181/docs/lecture4-notes.pdf
         CS181 Lecture 4 | Committees and Boosting, Avi Pfeer; Revised by David Parkes, Feb 1, 2011
    */
   class WeightedEntropy
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

      struct LevelData : public core::NonCopyable
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
       @param weights a weights attributed to each sample. sum(weights) == 1
       */
      template <class NodeFactory>
      void compute( const Database& dat, const NodeFactory& nodeFactory, const GrowingStrategy& growingStrategy, const core::Buffer1D<float> weights = core::Buffer1D<float>() )
      {
         ensure( weights.size() == dat.size() || weights.size() == 0, "weights must have the same size as database or empty" );

         #ifdef NLL_SECURE
         if ( weights.size() )
         {
            const float sumw = std::accumulate( weights.begin(), weights.end(), 0.0f );
            ensure( core::equal( sumw, 1.0f, 0.01f ), "sum of weights must be 1" );
         }
         #endif

         _compute( dat, weights, nodeFactory, growingStrategy, 0 );
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

      void print( std::ostream& o ) const
      {
         if ( _nodes.size() )
         {
            o << "non-leaf node:";
            _split->print( o );
            o << core::incendl;
            for ( size_t n = 0; n < _nodes.size(); ++n )
            {
               _nodes[ n ].print( o );
               if ( ( n + 1 ) != _nodes.size() )
                  o << core::iendl;
            }
            o << core::decindent;
         } else {
            o << "leaf node,  class=" << _nodeClass;
         }
      }

   private:
      template <class NodeFactory>
      void _compute( const Database& dat, const core::Buffer1D<float>& weights, const NodeFactory& nodeFactory, const GrowingStrategy& growingStrategy, ui32 level )
      {
         LevelData ld( level, dat );
         const bool continueGrowth = growingStrategy.continueGrowth( ld );

         if ( !continueGrowth )
         {
            _nodeClass = growingStrategy.getNodeClass( dat );
            return;
         }

         std::vector<Database> dats;
         _split = std::shared_ptr<NodeSplit>( new NodeFactory::value_type( nodeFactory.create() ) );
         _split->compute( dat, dats );

         _nodes = std::vector<DecisionTree>( dats.size() );
         for ( size_t n = 0; n < dats.size(); ++n )
         {
            _nodes[ n ]._compute( dats[ n ], weights, nodeFactory, growingStrategy, level + 1 );
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
         ensure( data.data.size(), "hugh!? empty!" );

         // first ensure the node is not pure
         Class c = data.data[ 0 ].output;
         for ( ui32 n = 1; n < data.data.size(); ++n )
         {
            if ( c != data.data[ n ].output )
            {
               // the node is impure, check we are below the allowed depth
               return data.depth <= _maxDepth;
            }
         }
         return false;  // node is pure! can't get better now...
      }

      /**
       @brief returns the class of a current node
       */
      virtual Class getNodeClass( const Database& dat ) const
      {
         // get the max class to be able to count the classes
         ui32 max = 0;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            max = std::max<ui32>( dat[ n ].output, max );
         }

         // count the classes
         std::vector<ui32> counts( max + 1 );
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            ++counts[ dat[ n ].output ];
         }

         // return the max count class
         std::vector<ui32>::const_iterator it = std::max_element( counts.begin(), counts.end() );
         return static_cast<Class>( it - counts.begin() );
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

      typedef Problem1::Database Database;
      typedef core::FactoryGeneric<algorithm::TreeNodeSplitDiscrete<Database, algorithm::InformationGain>> NodeFactory;
      dt.compute( dat, NodeFactory(), algorithm::GrowingStrategyFixedDepth<Database>() );

      std::cout << std::endl;

      std::stringstream ss2;
      dt.print( ss2 );

      std::stringstream ss;
      ss << "non-leaf node:feature split=0"  << core::incendl <<
             "non-leaf node:feature split=2" << core::incendl <<
              "leaf node,  class=0"          << core::iendl <<
              "leaf node,  class=1"          << core::decendl <<
             "leaf node,  class=1"           << core::iendl <<
             "non-leaf node:feature split=3" << core::incendl <<
              "leaf node,  class=1"          << core::iendl <<
              "leaf node,  class=0";

      TESTER_ASSERT( ss.str() == ss2.str() );
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
TESTER_TEST(testDecisionTree);
TESTER_TEST_SUITE_END();
#endif