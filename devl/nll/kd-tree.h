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

#ifndef NLL_KD_TREE_H_
# define NLL_KD_TREE_H_

# include <map>
# include <set>
# include "types.h"
# include "type-traits.h"
# include "generic-operations.h"

# pragma warning( push )
# pragma warning( disable:4267 ) // conversion possible loss of data
# pragma warning( disable:4245 ) // conversion signed/unsigned mismatch
# pragma warning( disable:4244 ) // conversion possible loss of data

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief generic implementation of a kd-tree.

    Implementation of "K-d trees for semidynamic point sets,
    Jon Louis Bentley	AT&T Bell Laboratories, Murray Hill, NJ"

    "http://portal.acm.org/citation.cfm?doid=98524.98564"
    Point need to provide:
      "f32 operator[](size_t) const"

    Distance needs to provide
      double distance( const Point& p1, const Point& p2 ) const;
      Distance( const Distance& d )

      The distance must be of the form sum_i( coef_i * pow( x_i, coef2_i ) ), with coef_i >= 1
   
   	build o(n log n)
      NearestNeighbor o(log n) 
      no insert/del
   
   	cutoff : number of points in leaf

      @note the points address must <b>never</b> change! (they are indexed in the tree,
            but data are still needed and so must never change between building and retrieving).
    */
   template <	class	   Point,
               class    Distance = MetricEuclidian<Point>,
			      int		cutoff = 5,
			      class	   Points = std::vector<Point> >
   class KdTree
   {
   private:
	   struct			_KdTree
	   {
      public:
		   _KdTree() : bucket(false), cut_dim(-1), cut_val(-1), l(0), h(0), lopt(-1), hipt(-1){}
		   bool		bucket;
		   size_t		cut_dim;
		   f32		cut_val;
		   _KdTree*	l;
		   _KdTree*	h;
		   size_t		lopt;
         size_t     hipt;

         _KdTree* clone() const
         {
            _KdTree* root = new _KdTree();
            _clone( this, root );
            return root;
         }

      private:
         void _copyValue( const _KdTree* cpy )
         {
            if ( cpy )
            {
               bucket = cpy->bucket;
               cut_dim = cpy->cut_dim;
               cut_val = cpy->cut_val;
               lopt = cpy->lopt;
               hipt = cpy->hipt;
            }
         }
         void _clone( const _KdTree* root, _KdTree*& rootClone ) const
         {
            if ( root->l )
            {
               rootClone->l = new _KdTree();
               rootClone->l->_copyValue( root->l );
               _clone( root->l, rootClone->l );
            } else {
               rootClone->l = 0;
            }

            if ( root->h )
            {
               rootClone->h = new _KdTree();
               rootClone->h->_copyValue( root->h );
               _clone( root->h, rootClone->h );
            } else {
               rootClone->h = 0;
            }
         }

         _KdTree& operator=( const _KdTree& );
         _KdTree( const _KdTree& );
	   };

   public:
	   struct			NearestNeighbor
	   {
		   NearestNeighbor(size_t pointId, f32 pointDist) : id(pointId), dist(pointDist){}
		   size_t		id;
		   f32		dist;
		   bool operator<(const NearestNeighbor& n) const {return dist < n.dist;}
	   };
	   typedef	std::multiset<NearestNeighbor> NearestNeighborList;
	   void print(std::ostream& o, const NearestNeighborList& nns)
	   {
		   for (typename NearestNeighborList::const_iterator i = nns.begin(); i != nns.end(); ++i)
			   o << "(" << i->id << ", " << i->dist << ") ";
	   }

   private:
      struct   _Query
      {
         NearestNeighborList nearest;
         f32                 minDistFound;
      };

   public:
      /**
       @brief default Distance so we can put it in Vectors. Use setDistance() if it needs additional parameters
       */
	   KdTree( const Distance& dist = Distance() ) : _root(0), _distance( dist ){}

	   ~KdTree(){ _delete(_root); }

      /**
       @brief set the distance to be used by the kd-Tree
       */
      void setDistance( const Distance& d )
      {
         _distance = d;
      }

      /**
       @brief copy constructor
       */
      KdTree( const KdTree& cpy )
      {
         copy( cpy );
      }

      KdTree& operator=( const KdTree& cpy )
      {
         copy( cpy );
         return *this;
      }

      void copy( const KdTree& cpy )
      {
         _perm = cpy._perm;
         _invPerm = cpy._invPerm;
         _tmpMin = cpy._tmpMin;
         _tmpMax = cpy._tmpMax;
         _points = cpy._points;
         _pointSize = cpy._pointSize;
         if ( cpy._root )
            _root = cpy._root->clone();
         else
            _root = 0;
         _distance = cpy._distance;
      }

	   /**
       @brief Buil the tree, O(n log n). <code>points</code> MUST remain valid and unchanged until
              the tree is used. If not the result is invalid.
       @param pointSize the dimension of a point
	    */
	   void build(const Points& points, size_t pointSize )
	   {
		   if (!points.size())
			   return;
         _pointSize = pointSize;
		   _points = &points;
		   size_t size = points.size();
         _perm = std::vector<size_t>( size );
         _tmpMin = std::vector<f32>( pointSize );
         _tmpMax = std::vector<f32>( pointSize );
		   for (size_t n = 0; n < size; ++n)
			   _perm[ n ] = n;
		   _root = _build( 0, size - 1 );
		   _invPerm = std::vector<size_t>( points.size() );
		   for (size_t n = 0; n < points.size(); ++n)
			   _invPerm[ _perm[ n ] ] = n;
	   }

      /**
       @brief Find the k nearest neighbours. Complexity O(k log n)
       @param point the point to find the k neighbours from
	    */
	   NearestNeighborList findNearestNeighbor(const Point& point, size_t k) const
	   {
         ensure( k, "k must be > 1" );
         _Query query;
		   _findNearestNeighbor(query, _root, point, k);
		   return query.nearest;
	   }

      /**
       @brief display the tree
	    */
	   void display() const
	   {
		   _display(_root);
	   }
      
   private:
	   void _delete(_KdTree* p)
	   {
		   if (p)
		   {
			   _delete(p->l);
			   _delete(p->h);
			   delete p;
		   }
	   }

	   void _display(const _KdTree* p) const
	   {
		   if (p)
		   {
			   std::cout << "Node:" << p << std::endl;
			   std::cout << "  bucket  :" << p->bucket << std::endl;
			   if (p->bucket)
			   {
				   std::cout << "  pt:";
				   for (size_t n = p->lopt; n <= p->hipt; ++n)
					   std::cout << _perm[n] << ";";
				   std::cout << std::endl;
			   } else {
				   std::cout << "  cutdim  :" << p->cut_dim << std::endl;
				   std::cout << "  cutval  :" << p->cut_val << std::endl;
				   std::cout << "  l       :" << p->l << std::endl;
				   std::cout << "  h       :" << p->h << std::endl;
			   }
			   _display(p->l);
			   _display(p->h);
		   }
	   }

      void _findNearestNeighbor(_Query& query, const _KdTree* p, const Point& point, size_t k) const
	   {
		   if (p->bucket)
		   {
			   for (size_t n = p->lopt; n <= p->hipt; ++n)
			   {
				   f32 dist = _distance.distance((*_points)[_perm[n]], point );
				   if (query.nearest.size() < k)
				   {
					   query.nearest.insert(NearestNeighbor(_perm[n], dist));
					   query.minDistFound = query.nearest.rbegin()->dist;
				   }
				   else 
				   {
					   f32 minDistFound = query.nearest.rbegin()->dist;
					   if (dist < minDistFound)
					   {
						   query.nearest.erase(--query.nearest.end());
						   query.nearest.insert(NearestNeighbor(_perm[n], dist));
						   query.minDistFound = query.nearest.rbegin()->dist;
					   }
				   }
			   }
		   } else {
			   f32 val = p->cut_val;
			   f32 target_val = point[ p->cut_dim ];
			   if (target_val <= val)
			   {
				   _findNearestNeighbor(query, p->l, point, k);
				   if (query.nearest.size() < k)
					   _findNearestNeighbor(query, p->h, point, k);
				   else
				   {
					   if (target_val + query.minDistFound > val)
						   _findNearestNeighbor(query, p->h, point, k);
				   }
			   } else {
				   _findNearestNeighbor(query, p->h, point, k);
				   if (query.nearest.size() < k)
					   _findNearestNeighbor(query, p->l, point, k);
				   else
				   {
					   if (target_val - query.minDistFound < val)
						   _findNearestNeighbor(query, p->l, point, k);
				   }
			   }
		   }
	   }

	   //
	   // Build algorithm
	   // Complexity O(k.n log n)
	   //
	   _KdTree* _build(int l, int u)
	   {
		   _KdTree* p = new _KdTree;
		   if (u - l + 1 <= cutoff)
		   {
			   p->bucket	= true;
			   p->lopt		= l;
			   p->hipt		= u;
		   } else {
			   p->bucket	= false;
			   p->cut_dim	= _findMaxSpread(l, u);
			   size_t m;
			   _select(l, u, m, p->cut_dim);
			   p->cut_val	= (px(m, p->cut_dim));

			   p->l = _build(l, m);
			   p->h = _build(m + 1, u);
		   }
		   return p;
	   }

	   f32 px(size_t id, size_t dim) const
	   {
		   return (*_points)[_perm[id]][dim];
	   }


	   #define PA(n, d)		(px(n, d))
	   #define PASWAP(n1, n2)	std::swap(_perm[n1], _perm[n2])

	   //
	   // QuickSelect algorithm 
	   // www.cse.yorku.ca/~andy/courses/3101/lecture-notes/LN4.ps 
	   // average complexity O(n)
	   //
	   void _select(size_t l, size_t u, size_t& k, size_t dim)
	   {
		   size_t n_lo = (l + u) / 2;
		   while (l < u)
		   {
			   size_t i = (u + l) / 2;				// select middle as pivot

			   if (PA(i, dim) > PA(u, dim))		// make sure last > pivot
				   PASWAP(i, u);
			   PASWAP(l, i);						// move pivot to first position

			   float c = PA(l, dim);				// pivot value
			   i = l;
			   k = u;
			   for(;;) {							// pivot about c
				   while (PA(++i, dim) < c)
					   ;
				   while (PA(--k, dim) > c)
					   ;
				   if (i < k) PASWAP(i,k); else break;
			   }
			   PASWAP(l,k);						// pivot winds up in location k

			   if (k > n_lo)	   u = k - 1;		// recurse on proper subarray
			   else if (k < n_lo) l = k + 1;
			   else break;							// got the median exactly
		   }
	   }

	   //
	   // O(n)
	   //
	   size_t _findMaxSpread(size_t l, size_t u)
	   {
		   for (size_t n = 0; n < _pointSize; ++n)
		   {
			   _tmpMin[n] = (f32)INT_MAX;
			   _tmpMax[n] = (f32)INT_MIN;
		   }
		   for (size_t n = l; n <= u; ++n)
			   for (size_t dim = 0; dim < _pointSize; ++dim) 
			   {
				   f32 val = px(n, dim);
				   if (val > _tmpMax[dim])
					   _tmpMax[dim] = val;
				   if (val < _tmpMin[dim])
					   _tmpMin[dim] = val;
			   }
   		
		   f32	max = (f32)INT_MIN;
		   size_t	index = -1;
		   for (size_t dim = 0; dim < _pointSize; ++dim)
		   {
			   f32 val = (f32)(_tmpMax[dim] - _tmpMin[dim]);
			   if (val > max)
			   {
				   max = val;
				   index = dim;
			   }
		   }
         ensure( index != -1, "something wrong here!" );
		   return index;
	   }

   private:
      std::vector<size_t>    _perm;
      std::vector<size_t>    _invPerm;
	   _KdTree*		         _root;
      std::vector<f32>     _tmpMin;
      std::vector<f32>     _tmpMax;
	   const Points*	      _points;
      size_t                 _pointSize;
      Distance             _distance;
   };
}
}

# pragma warning( pop )

#endif
