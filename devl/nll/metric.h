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

    @note Metric should be used directly but instead the actual implementation. It will be replaced by
          concepts in C++ 0x when available. If actual implementation is used, 'virtual' method will be
          optimized by VS2005.
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
   //   virtual std::string name() const = 0;

   //public:
   //   f64 operator()( const Point& p1, const Point& p2 ) const { return distance( p1, p2 ); }
   //   virtual ~Metric(){}
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

  //    virtual std::string name() const { return typeid( this ).name(); }
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
  //    virtual std::string name() const { return typeid( this ).name(); }
   };

}
}

#endif
