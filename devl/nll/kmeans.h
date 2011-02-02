/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_KMEANS_H_
# define NLL_KMEANS_H_

# include "types.h"
# include "generic-operations.h"

#pragma warning( push )
#pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

namespace nll
{
namespace algorithm
{
   /**
    @brief Default allocator. Point must be constructible with a specified size.

    @todo C++ 0x Concept Allocator
    */
   template <class Point>
   struct AllocatorPointConstructible
   {
      static Point allocate( ui32 size )
      {
         return Point( size );
      }

      /**
       We don't use a reference: anyway if Point is not a pointer, it will never be deallocated,
       so the compiler will optimize everything
       */
      static void deallocate( const Point )
      {
         // do nothing: allocated on the stack
      }
   };

   /**
    @ingroup algorithm
    @brief Configuration of the Kmean algorithm.

    Metric must be the actual implementation of a Metric and not a base class.

    Method that must be defined for this class:
    Point:
      -Copyable
      -operator[]
      -if default allocator, it must be constructible
    Metric:
      -distance(Point, Point)
      -copy constructible
      -read
      -write
    Points:
      - operator[]
      - size()

    Method that must be defined in order to make kmeans working:
    For Prototype (Todo C++ 0x), KMeansUtility:
    -null
    -add
    -div
    -size
    typedef point, points
    */
   template <	class TPoint,
               class TMetric = MetricEuclidian<TPoint>,
			      class TPoints = std::vector<TPoint>,
               class AllocatorPoint = AllocatorPointConstructible<TPoint>
            >
   struct BuildKMeansUtility
   {
      typedef TPoint		Point;
	   typedef TPoints	Points;
      typedef TMetric   Metric;
      typedef AllocatorPoint  Allocator;

      BuildKMeansUtility( ui32 pointSize, const Metric& metric ) : _pointSize( pointSize), _metric( metric )
      {
      }

      ui32 pointSize() const
      {
         return _pointSize;
      }

	   inline Point null()
	   {
         Point p = AllocatorPoint::allocate( _pointSize );
         for ( ui32 n = 0; n < _pointSize; ++n )
            p[ n ] = 0;
         return p;
	   }

	   inline void add( Point& p1, const Point& p2)
	   {
         for ( ui32 n = 0; n < _pointSize; ++n )
            p1[ n ] += p2[ n ];
	   }

	   inline void div( Point& p1, double p2)
	   {
		   for ( ui32 n = 0; n < _pointSize; ++n )
            p1[ n ] /= p2;
	   }

      inline double distance( const Point& p1, const Point& p2 )
      {
         return _metric.distance( p1, p2 );
      }

   private:
      Metric   _metric;
      ui32     _pointSize;
   };

   /**
    @ingroup algorithm
    @brief generic implementation of the KMeans algorithm
   
    use BuildKMeansUtility to build a generic utility helper
    */
   template <class Utility>
   class KMeans
   {
   public:
      typedef typename Utility::Point                             Point;
      typedef std::vector<Point>                                  Clusters;
	   typedef typename Utility::Points                            Points;
	   typedef ui32                                                KMeansPoint;
	   typedef std::vector<KMeansPoint>                            KMeansPoints;
	   typedef std::pair<KMeansPoints, Clusters>                   KMeansResult;

   public:
      KMeans( const Utility& utility ) : _utility( utility )
      {}

      /**
       @brief return clusterIds/centroids
              If the centroids are dynamically allocated by Utility::Allocator, it is the responsability
              of the user to deallocate the clusters returned in the results
       */
	   KMeansResult operator()(const typename Utility::Points& points, ui32 nbClusters, ui32 maxIter = 0)
	   {
		   _results.clear();
		   _clusters.clear();
		   if (points.size() < nbClusters || nbClusters == 0)
			   return KMeansResult(_results, _clusters);
		   _results = KMeansPoints(points.size());
         _clusters = Clusters(nbClusters);
		   for (ui32 n = 0; n < points.size(); ++n)
			   _results[n] = static_cast<ui32> (rand() % _clusters.size());

		   bool clusterChange = true;
		   for (ui32 n = 0; (!maxIter || n < maxIter) && clusterChange; ++n)
		   {
			   _calculateCentroids(points);
			   clusterChange = _reassignClusters(points);
		   }

		   return KMeansResult(_results, _clusters);
	   }

   private:
	   // calculate the position of the centroid
	   void	_calculateCentroids(const typename Utility::Points& points)
	   {
		   std::vector<ui32> count(_clusters.size());

		   for (ui32 n = 0; n < static_cast<ui32>(_clusters.size()); ++n)
         {
            // deallocate the previously allocated centroids
            Utility::Allocator::deallocate( _clusters[ n ] );
			   _clusters[n] = _utility.null();
         }

		   for (ui32 n = 0; n < _results.size(); ++n)
		   {
			   const ui32 result = _results[n];
			   if (result == -1)
				   continue;	// the point doesn't belongs to any cluster, is <Metrix::distance> correct?
			   _utility.add(_clusters[result], points[n]);
			   ++count[result];
		   }

		   for (ui32 n = 0; n < _clusters.size(); ++n)
			   if (count[n])	// ensure there are points in the cluster
				   _utility.div(_clusters[n], static_cast<f64>(count[n]));
	   }

	   // assign the nearest cluster for each point
	   bool	_reassignClusters(const typename Utility::Points& points)
	   {
		   bool changed = false;
		   for (ui32 n = 0; n < _results.size(); ++n)
		   {
			   f64   minDist = static_cast<f64>(INT_MAX);
			   i32	minCluster = -1;
			   for (i32 m = 0; m < static_cast<i32> (_clusters.size()); ++m)
			   {
				   f64 dist = _utility.distance(points[n], _clusters[m]);
				   if (minDist > dist)
				   {
					   minDist = dist;
					   minCluster = m;
				   }

			   }
			   if (static_cast<i32> (_results[n]) != minCluster)
			   {
				   changed = true;
				   _results[n] = minCluster;
			   }
		   }
		   return changed;
	   }

   private:
	   Clusters	                     _clusters;
	   KMeansPoints						_results;
      Utility                       _utility;
   };
}
}

#pragma warning( pop )

#endif
