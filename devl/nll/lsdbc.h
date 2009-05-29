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
		   PointDist(ui32 pointId, const Metric& m ) : id(pointId), metric( m ){}
		   bool operator<(const PointDist& p) const
		   {
			   return metric((*MeasureDistSet::set_points)[MeasureDistSet::set_index], (*MeasureDistSet::set_points)[id])
				   <  metric((*MeasureDistSet::set_points)[MeasureDistSet::set_index], (*MeasureDistSet::set_points)[p.id]);				
		   }
		   ui32          id;
         const Metric& metric;
	   };
	   typedef std::multiset<PointDist>	PointDistSet;

   public:
	   static const Points*	set_points;
	   static ui32				set_index;

	   PointDistSet			set;
   };

   template <	class Point,
			      class Metric,
			      class Points >
   const Points* MeasureDistSet<Point, Metric, Points>::set_points = 0;

   template <	class Point,
			      class Metric,
			      class Points >
   ui32 MeasureDistSet<Point, Metric, Points>::set_index = 0;


   /**
    @ingroup algorithm
    @brief implement the locally scaled density based algorithm.

     LSDBC : Locally Scaled Density Based Clustering
    "http://www.denizyuret.com/pub/icannga07/LSDBC-icannga07.pdf"

    Point : must define a method "static ui32 size()" that returns the number of components of the Point
                                   "f32 operator[](ui32 indice) const" returns the n-th component of Point

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
		   CPoint(const Point& p, ui32 cluster, f32 eps) : point(p), clusterId(cluster), epsilon(eps){}
		   Point	point;
		   ui32	clusterId;		// if (clusterId == 0) : UNCLASSIFIED
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
	   const CPoints compute(const Points& points, ui32 k, f32 alpha)
	   {
		   // init : calculate neighboring and distance map
		   CPoints	cpoints;
		   if (points.size() < 1)
			   return cpoints;
   		
		   const ui32 size = points[ 0 ].size();

		   typedef KdTree<Point, Metric, 5, Points>			KdTreeInstance;
		   typedef MeasureDistSet<Point, Metric, Points>	MeasureDistSet;
		   typedef std::vector<MeasureDistSet>			      MeasureDistSets;

		   KdTreeInstance kdTree( _metric );
		   kdTree.build(points, size);
		   MeasureDistSets results_kdtree;
		   MeasureDistSet::set_points = &points;
   		
		   // rebuild points
		   for (ui32 n = 0; n < points.size(); ++n)
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
		   typedef std::pair<f32, ui32> Pair;
		   typedef std::vector<Pair> Pairs;
		   Pairs pairs;
		   for (ui32 n = 0; n < points.size(); ++n)
			   pairs.push_back(Pair(cpoints[n].epsilon, n));
		   std::sort(pairs.begin(), pairs.end());

		   // clusterize
		   ui32 clusterId = CLUSTER_UNCLASSIFIED;
		   for (ui32 n = 0; n < cpoints.size(); ++n)
		   {
			   if (cpoints[pairs[n].second].clusterId == CLUSTER_UNCLASSIFIED && 1) // FIXME : what means localMax(p) ?
				   _expandCluster<MeasureDistSet, MeasureDistSets>(pairs[n].second, cpoints, results_kdtree, ++clusterId, alpha, size);
		   }
		   return cpoints;
	   }

   private:
	   template <class MeasureDistSet, class MeasureDistSets>
	   void _expandCluster(ui32 point, CPoints& points, MeasureDistSets& neigbors, ui32 clusterId, f32 alpha, ui32 n)
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
			   ui32 id = cur->id;
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
		   ui32 n = 0;
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
