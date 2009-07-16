#include "stdafx.h"

#include <nll/nll.h>
#include <cmath>

typedef std::vector<double>                  Point;
typedef std::vector< std::vector<double> >   Points;

using namespace nll;

Points generate( ui32 nbDim, ui32 size )
{
   Points points;
   for ( ui32 n = 0; n < size; ++n )
   {
      Point p( nbDim );
      for ( ui32 nn = 0; nn < nbDim; ++nn )
         p[ nn ] = core::generateUniformDistribution( -100, 100 );
      points.push_back( p );
   }
   return points;
}

ui32 closerDummy( const Point& p, const Points& points, const algorithm::Metric<Point>& metric )
{
   double minDist = 100000;
   ui32 index = 0;
   for ( ui32 n = 0; n < points.size(); ++n )
   {
      double dist = metric.distance( p, points[ n ] );
      if ( dist < minDist )
      {
         minDist = dist;
         index = n;
      }
   }
   return index;
}

class MetricUser1 : public algorithm::Metric<Point>
{
public:
   virtual f64 distance( const Point& p1, const Point& p2 ) const
   {
      assert( p1.size() == p2.size() );
      f64 dist = 0;
      for ( ui32 n = 0; n < p1.size(); ++n)
         dist +=  ( n * 0.1 + 1  ) * pow( core::absolute( (double)p1[ n ] - p2[ n ] ), (double)n + 1 );
      return dist;
   }
};

/**
 @brief test kdtree using different metrics
 */
class TestKdTree
{
public:
   void testMetric1()
   {
      srand( 0 );
      const ui32 nbDim = 4;
      Points points = generate( nbDim, 10000 );
      typedef MetricUser1 Metric;

      Metric metric;
      typedef algorithm::KdTree< Point, Metric > KdTree;

      KdTree tree( metric );
      tree.build( points, nbDim );

      for ( ui32 n = 0; n < 400; ++n )
      {
         // generate a random point
         Point p = generate( nbDim, 1 )[ 0 ];
         KdTree::NearestNeighborList result = tree.findNearestNeighbor( p, 1 );
         TESTER_ASSERT( result.begin()->id == closerDummy( p, points, metric ) );
      }
   }

   void testMetric2()
   {
      srand( 0 );
      const ui32 nbDim = 4;
      Points points = generate( nbDim, 10000 );
      typedef algorithm::MetricManhattan<Point> Metric;

      Metric metric;
      typedef algorithm::KdTree< Point, Metric > KdTree;

      KdTree tree( metric );
      tree.build( points, nbDim );

      for ( ui32 n = 0; n < 400; ++n )
      {
         // generate a random point
         Point p = generate( nbDim, 1 )[ 0 ];
         KdTree::NearestNeighborList result = tree.findNearestNeighbor( p, 1 );
         TESTER_ASSERT( result.begin()->id == closerDummy( p, points, metric ) );
      }
   }
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKdTree);
TESTER_TEST(testMetric1);
TESTER_TEST(testMetric2);
TESTER_TEST_SUITE_END();
//#endif