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

    "http://portal.acm.org/citation.cfm?doid=98524.98564"
    Point need to provide:
      "f32 operator[](ui32) const"

    Distance needs to provide
      double operator()( const Point& p1, const Point& p2 );
      Distance( const Distance& d )
   
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
		   ui32		cut_dim;
		   f32		cut_val;
		   _KdTree*	l;
		   _KdTree*	h;
		   ui32		lopt;
         ui32     hipt;

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
		   NearestNeighbor(ui32 pointId, f32 pointDist) : id(pointId), dist(pointDist){}
		   ui32		id;
		   f32		dist;
		   bool operator<(const NearestNeighbor& n) const {return dist < n.dist;}
	   };
	   typedef	std::multiset<NearestNeighbor> NearestNeighborList;
	   void print(std::ostream& o, const NearestNeighborList& nns)
	   {
		   for (typename NearestNeighborList::const_iterator i = nns.begin(); i != nns.end(); ++i)
			   o << "(" << i->id << ", " << i->dist << ") ";
	   }
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
         _minDistFound = cpy._minDistFound;
         _minDistIndex = cpy._minDistIndex;
         _nearest = cpy._nearest;
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
	   void build(const Points& points, ui32 pointSize )
	   {
		   if (!points.size())
			   return;
         _pointSize = pointSize;
		   _points = &points;
		   ui32 size = points.size();
         _perm = std::vector<ui32>( size );
         _tmpMin = std::vector<f32>( size );
         _tmpMax = std::vector<f32>( size );
		   for (ui32 n = 0; n < size; ++n)
			   _perm[ n ] = n;
		   _root = _build( 0, size - 1 );
		   _invPerm = std::vector<ui32>( points.size() );
		   for (ui32 n = 0; n < points.size(); ++n)
			   _invPerm[ _perm[ n ] ] = n;
	   }

      /**
       @brief Find the k nearest neighbours. Complexity O(k log n)
       @param point the point to find the k neighbours from
	    */
	   NearestNeighborList findNearestNeighbor(const Point& point, ui32 k) const
	   {
         ensure( k, "k must be > 1" );
		   _nearest.clear();
		   _findNearestNeighbor(_root, point, k);
		   return _nearest;
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
				   for (ui32 n = p->lopt; n <= p->hipt; ++n)
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

      void _findNearestNeighbor(const _KdTree* p, const Point& point, ui32 k) const
	   {
		   if (p->bucket)
		   {
			   for (ui32 n = p->lopt; n <= p->hipt; ++n)
			   {
				   f32 dist = _distance((*_points)[_perm[n]], point );
				   if (_nearest.size() < k)
				   {
					   _nearest.insert(NearestNeighbor(_perm[n], dist));
					   _minDistFound = _nearest.rbegin()->dist;
				   }
				   else 
				   {
					   f32 minDistFound = _nearest.rbegin()->dist;
					   if (dist < minDistFound)
					   {
						   _nearest.erase(--_nearest.end());
						   _nearest.insert(NearestNeighbor(_perm[n], dist));
						   _minDistFound = _nearest.rbegin()->dist;
					   }
				   }
			   }
		   } else {
			   f32 val = p->cut_val;
			   f32 target_val = point[ p->cut_dim ];
			   if (target_val <= val)
			   {
				   _findNearestNeighbor(p->l, point, k);
				   if (_nearest.size() < k)
					   _findNearestNeighbor(p->h, point, k);
				   else
				   {
					   if (target_val + _minDistFound > val)
						   _findNearestNeighbor(p->h, point, k);
				   }
			   } else {
				   _findNearestNeighbor(p->h, point, k);
				   if (_nearest.size() < k)
					   _findNearestNeighbor(p->l, point, k);
				   else
				   {
					   if (target_val - _minDistFound < val)
						   _findNearestNeighbor(p->l, point, k);
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
			   ui32 m;
			   _select(l, u, m, p->cut_dim);
			   p->cut_val	= (px(m, p->cut_dim));

			   p->l = _build(l, m);
			   p->h = _build(m + 1, u);
		   }
		   return p;
	   }

	   f32 px(ui32 id, ui32 dim) const
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
	   void _select(ui32 l, ui32 u, ui32& k, ui32 dim)
	   {
		   ui32 n_lo = (l + u) / 2;
		   while (l < u)
		   {
			   ui32 i = (u + l) / 2;				// select middle as pivot

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
	   ui32 _findMaxSpread(ui32 l, ui32 u)
	   {
		   for (ui32 n = 0; n < _pointSize; ++n)
		   {
			   _tmpMin[n] = (f32)INT_MAX;
			   _tmpMax[n] = (f32)INT_MIN;
		   }
		   for (ui32 n = l; n <= u; ++n)
			   for (ui32 dim = 0; dim < _pointSize; ++dim) 
			   {
				   f32 val = px(n, dim);
				   if (val > _tmpMax[dim])
					   _tmpMax[dim] = val;
				   if (val < _tmpMin[dim])
					   _tmpMin[dim] = val;
			   }
   		
		   f32	max = (f32)INT_MIN;
		   ui32	index = -1;
		   for (ui32 dim = 0; dim < _pointSize; ++dim)
		   {
			   f32 val = (f32)(_tmpMax[dim] - _tmpMin[dim]);
			   if (val > max)
			   {
				   max = val;
				   index = dim;
			   }
		   }
		   assert(index != -1); // "error index"
		   return index;
	   }

   private:
      std::vector<ui32>    _perm;
      std::vector<ui32>    _invPerm;
	   _KdTree*		         _root;
      std::vector<f32>     _tmpMin;
      std::vector<f32>     _tmpMax;
	   const Points*	      _points;
	   mutable f32				_minDistFound;
	   mutable ui32			_minDistIndex;
	   mutable NearestNeighborList  _nearest;
      ui32                 _pointSize;
      Distance             _distance;
   };
}
}

# pragma warning( pop )

#endif
