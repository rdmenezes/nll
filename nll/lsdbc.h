/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
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

#ifndef NLL_LSDBC_H_
# define NLL_LSDBC_H_

# ifdef NLL_NOT_MULTITHREADED

#  include <vector>
#  include <algorithm>
#  include "kd-tree.h"
#  include "types.h"

#  define CLUSTER_UNCLASSIFIED	0

//
// TODO: non thread safe class!!
//

namespace nll
{
namespace algorithm
{
   template <	class Point,
			      class Metric,
			      class Points = std::vector<Point> 
		      >
   class MeasureDistSet
   {
   public:
	   class PointDist
	   {
	   public:
		   PointDist(size_t pointId, const Metric& m ) : id(pointId), metric( m ){}
		   bool operator<(const PointDist& p) const
		   {
			   return metric((*MeasureDistSet::set_points)[MeasureDistSet::set_index], (*MeasureDistSet::set_points)[id])
				   <  metric((*MeasureDistSet::set_points)[MeasureDistSet::set_index], (*MeasureDistSet::set_points)[p.id]);				
		   }
		   size_t          id;
         const Metric& metric;
	   };
	   typedef std::multiset<PointDist>	PointDistSet;

   public:
	   static const Points*	set_points;
	   static size_t				set_index;

	   PointDistSet			set;
   };

   template <	class Point,
			      class Metric,
			      class Points >
   const Points* MeasureDistSet<Point, Metric, Points>::set_points = 0;

   template <	class Point,
			      class Metric,
			      class Points >
   size_t MeasureDistSet<Point, Metric, Points>::set_index = 0;


   /**
    @ingroup algorithm
    @brief implement the locally scaled density based algorithm.

     LSDBC : Locally Scaled Density Based Clustering
    "http://www.denizyuret.com/pub/icannga07/LSDBC-icannga07.pdf"

    Point : must define a method "static size_t size()" that returns the number of components of the Point
                                   "f32 operator[](size_t indice) const" returns the n-th component of Point

    Metric : must define a "static f32	distance(const Point&, const Point&)" : returns the distance
			     between 2 <Point>
    */
   template <	class Point,
			      class Metric >
   class Lsdbc
   {
   public:
	   struct	CPoint
	   {
		   CPoint(const Point& p, size_t cluster, f32 eps) : point(p), clusterId(cluster), epsilon(eps){}
		   Point	point;
		   size_t	clusterId;		// if (clusterId == 0) : UNCLASSIFIED
		   f32   epsilon;

		   bool operator<(const CPoint& p) const{return epsilon < p.epsilon;}
	   };
	   typedef std::vector<CPoint>	CPoints;

   public:
      Lsdbc( const Metric& m ) : _metric( m ){}

      /**
       @brief compute the clustering
   	
	    Points must define "size()" and "operator[]()"

	    @param	k number of neigbors that is considered for density calculation
	    @param	alpha	the boundary of the current cluster expansion based on its density
      */
	   template <class Points>
	   const CPoints compute(const Points& points, size_t k, f32 alpha)
	   {
		   // init : calculate neighboring and distance map
		   CPoints	cpoints;
		   if (points.size() < 1)
			   return cpoints;
   		
		   const size_t size = points[ 0 ].size();

		   typedef KdTree<Point, Metric, 5, Points>			KdTreeInstance;
		   typedef MeasureDistSet<Point, Metric, Points>	MeasureDistSet;
		   typedef std::vector<MeasureDistSet>			      MeasureDistSets;

		   KdTreeInstance kdTree( _metric );
		   kdTree.build(points, size);
		   MeasureDistSets results_kdtree;
		   MeasureDistSet::set_points = &points;
   		
		   // rebuild points
		   for (size_t n = 0; n < points.size(); ++n)
		   {
			   MeasureDistSet::set_index = n;
			   typename KdTreeInstance::NearestNeighborList list = kdTree.findNearestNeighborList_topDwon(n, k);
			   results_kdtree.push_back(MeasureDistSet());
			   typename MeasureDistSet::PointDistSet& set = results_kdtree.rbegin()->set;

			   f32 dist = 0;
			   for (typename KdTreeInstance::NearestNeighborList::const_iterator it = list.begin(); it != list.end(); ++it)
			   {
				   set.insert(it->id);
				   dist += it->dist;
			   }
			   //cpoints.push_back(CPoint(points[n], 0, dist / static_cast<f32>(k)));
			   cpoints.push_back(CPoint(points[n], 0, list.rbegin()->dist));
		   }

		   //std::cout << "Time knn:" << (f32)(clock() - t) / CLOCKS_PER_SEC << std::endl;

		   // sort points on epsilon value	
		   typedef std::pair<f32, size_t> Pair;
		   typedef std::vector<Pair> Pairs;
		   Pairs pairs;
		   for (size_t n = 0; n < points.size(); ++n)
			   pairs.push_back(Pair(cpoints[n].epsilon, n));
		   std::sort(pairs.begin(), pairs.end());

		   // clusterize
		   size_t clusterId = CLUSTER_UNCLASSIFIED;
		   for (size_t n = 0; n < cpoints.size(); ++n)
		   {
			   if (cpoints[pairs[n].second].clusterId == CLUSTER_UNCLASSIFIED && 1) // FIXME : what means localMax(p) ?
				   _expandCluster<MeasureDistSet, MeasureDistSets>(pairs[n].second, cpoints, results_kdtree, ++clusterId, alpha, size);
		   }
		   return cpoints;
	   }

   private:
	   template <class MeasureDistSet, class MeasureDistSets>
	   void _expandCluster(size_t point, CPoints& points, MeasureDistSets& neigbors, size_t clusterId, f32 alpha, size_t n)
	   {
		   points[point].clusterId = clusterId;
		   MeasureDistSet neigbor = neigbors[point];
		   for (typename MeasureDistSet::PointDistSet::iterator it = neigbor.set.begin(); it != neigbor.set.end();)
		   {
			   typename MeasureDistSet::PointDistSet::iterator cur = it++;
			   if (points[cur->id].clusterId == CLUSTER_UNCLASSIFIED)
				   points[cur->id].clusterId = clusterId;
			   else
				   neigbor.set.erase(cur);
		   }

		   while (neigbor.set.size())
		   {
			   typename MeasureDistSet::PointDistSet::iterator cur = neigbor.set.begin();
			   size_t id = cur->id;
			   if (points[id].epsilon <= points[point].epsilon * pow(2, alpha / static_cast<f32>(n)))
			   {
				   MeasureDistSet neigbor2 = neigbors[id];
				   for (typename MeasureDistSet::PointDistSet::iterator it = neigbor2.set.begin(); it != neigbor2.set.end(); ++it)
					   if (points[it->id].clusterId == CLUSTER_UNCLASSIFIED)
					   {
						   neigbor.set.insert(it->id);
						   points[it->id].clusterId = clusterId;
					   }
			   }
			   neigbor.set.erase(cur);
		   }
	   }
   	
   public:
	   static void print(const CPoints& points, std::ostream& o)
	   {
		   o << "lsdbc:" << std::endl;
		   size_t n = 0;
		   for (typename CPoints::const_iterator it = points.begin(); it != points.end(); ++it, ++n)
			   o << "\tpoint[" << n << "] = " << points[n].clusterId << std::endl;
	   }

   private:
      Metric   _metric;
   };
}
}
# endif

#endif
