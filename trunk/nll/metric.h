#ifndef NLL_METRIC_H_
# define NLL_METRIC_H_

# include <typeinfo>

namespace nll
{
namespace algorithm
{
   /**
    @brief Define a metric base class. It implements a distance function between 2 points.

           <code>Point</code> needs to provide size(), operator[]
           Derived classes must be copiable
    */
   template <class Point>
   class Metric
   {
   public:
      typedef Point  value_type;

      /**
       @return the distance between 2 points.
       */
      virtual f64 distance( const Point& p1, const Point& p2 ) const = 0;

      /**
       @return a unique name for this metric.
       */
      virtual std::string name() const = 0;

   public:
      f64 operator()( const Point& p1, const Point& p2 ) const { return distance( p1, p2 ); }
      virtual ~Metric(){}
   };

   /**
    @brief Defines the euclidian distance
    */
   template <class Point>
   class MetricEuclidian : public Metric<Point>
   {
   public:
      virtual f64 distance( const Point& p1, const Point& p2 ) const
      {
         assert( p1.size() == p2.size() );
         return core::generic_norm2<Point, f64>( p1, p2, static_cast<ui32>( p1.size() ) );
      }

      virtual std::string name() const { return typeid( this ).name(); }
   };

   /**
    @brief Defines the Manhattan distance
    */
   template <class Point>
   class MetricManhattan : public Metric<Point>
   {
   public:
      virtual f64 distance( const Point& p1, const Point& p2 ) const
      {
         assert( p1.size() == p2.size() );
         f64 dist = 0;
         for ( ui32 n = 0; n < p1.size(); ++n)
            dist += core::absolute( p1[ n ] - p2[ n ] );
         return dist;
      }
      virtual std::string name() const { return typeid( this ).name(); }
   };

}
}

#endif
