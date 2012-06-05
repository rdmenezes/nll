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

#ifndef NLL_ALGORITHM_INFORMATION_MEASURE_H_
# define NLL_ALGORITHM_INFORMATION_MEASURE_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Computes entropy for building trees
    @see www.cs.cmu.edu/~guestrin/Class/10701-S06/Handouts/recitations/recitation-decision_trees-adaboost-02-09-2006.ppt
         Information Gain,Decision Trees and Boosting, 10-701 ML recitation 9 Feb 2006
    */
   class Entropy
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
    @ingroup algorithm
    @brief Computes entropy for building trees
    @see www.seas.harvard.edu/courses/cs181/docs/lecture4-notes.pdf
         CS181 Lecture 4 | Committees and Boosting, Avi Pfeer; Revised by David Parkes, Feb 1, 2011
    */
   class EntropyWeighted
   {
   public:
      /**
       @brief computes the entropy of a vector of integrals

       Instead of just counting the number of classes as in entroy, we simply sum their probability given by weights
       @param v a set of integers
       @param weights the weights. Note that the on all instances must sum to 1 (not necessarily for this function though)
       */
      template <class Vector, class Vector2>
      double compute( const Vector& v, const Vector2& weights ) const
      {
         typedef typename Vector::value_type value_type;
         typedef typename Vector2::value_type value_type_weights;

         STATIC_ASSERT( core::IsIntegral<value_type>::value ); // this implementation only works for integral type
         ensure( v.size() == weights.size(), "weights vector doesn't match" );

         value_type min = std::numeric_limits<value_type>::max();
         value_type max = std::numeric_limits<value_type>::min();
         for ( ui32 n = 0; n < v.size(); ++n )
         {
            min = std::min( min, v[ n ] );
            max = std::max( max, v[ n ] );
         }

         const ui32 range = static_cast<ui32>( max - min ) + 1;
         std::vector<double> counts( range );
         double totalWeights = 0;
         for ( ui32 n = 0; n < v.size(); ++n )
         {
            const double w = static_cast<double>( weights[ n ] );
            ui32 c = static_cast<ui32>( v[ n ] - min );
            counts[ c ] += w;
            totalWeights += w;
         }

         #ifdef NLL_SECURE
         ensure( totalWeights > 0, "error, sum weights must be > 0" );
         #endif

         double entropy = 0;
         for ( size_t n = 0; n < counts.size(); ++n )
         {
            if ( counts[ n ] > 0 )
            {
               const double p = counts[ n ] / totalWeights;
               entropy -= p * core::log2( p );
            }
         }

         return entropy;
      }

      /**
       @brief Computes the conditional entropy H(y|x)
       @param x the attribut values
       @param y the class values
       @param weights the weights. Note that the on all instances must sum to 1 (not necessarily for this function though)
       @note values contained by x and y must be as close to zero as possible! (else arrays with extra padding are created)

       H(Y|X) = sum_i p(X=vi) H(Y|X=vi)
       */
      template <class Vector1, class Vector2, class Vector3>
      double compute( const Vector1& x, const Vector2& y, const Vector3& weights ) const
      {
         typedef typename Vector1::value_type value_type1;
         typedef typename Vector2::value_type value_type2;
         typedef typename Vector3::value_type value_type_weights;

         STATIC_ASSERT( core::IsIntegral<value_type1>::value ); // this implementation only works for integral type
         STATIC_ASSERT( core::IsIntegral<value_type2>::value ); // this implementation only works for integral type
         ensure( x.size() == y.size(), "must be the same size" );
         ensure( weights.size() == x.size(), "weights doesn't match" );

         value_type1 max = std::numeric_limits<value_type1>::min();
         for ( ui32 n = 0; n < x.size(); ++n )
         {
            max = std::max( max, x[ n ] );
         }

         double totalWeights = 0;
         std::vector<double> counts( max + 1 );
         for ( ui32 n = 0; n < x.size(); ++n )
         {
            const double w = static_cast<double>( weights[ n ] );
            ui32 i = static_cast<ui32>( x[ n ] );
            counts[ i ] += w;
            totalWeights += w;
         }

         std::vector< std::vector< value_type_weights > > condw( max + 1 );
         std::vector< std::vector< value_type2 > > cond( max + 1 );
         for ( size_t n = 0; n < cond.size(); ++n )
         {
            cond[ n ].reserve( x.size() / 2 );
            condw[ n ].reserve( x.size() / 2 );
         }

         for ( ui32 n = 0; n < x.size(); ++n )
         {
            ui32 i = static_cast<ui32>( x[ n ] );
            cond[ i ].push_back( y[ n ] );
            condw[ i ].push_back( weights[ n ] );
         }

         double entropy = 0;
         for ( size_t n = 0; n < cond.size(); ++n )
         {
            if ( counts[ n ] )
            {
               const double e = compute( cond[ n ], condw[ n ] );
               entropy += counts[ n ] / totalWeights * e;
            }
         }

         return entropy;
      }
   };

   /**
    @ingroup algorithm
    @brief Computes information gain on an integral input/output
    @see http://en.wikipedia.org/wiki/Information_gain_in_decision_trees
    */
   class InformationGain
   {
   public:
      /**
       @brief Compute the information gain IG(y|x) = E(y) - E(y|x)

       It can be seen as the amount of information saved by knowing x when transmiting y
       */
      template <class Vector1, class Vector2>
      double compute( const Vector1& x, const Vector2& y ) const
      {
         Entropy entropy;
         return entropy.compute( y ) - entropy.compute( x, y );
      }
   };

   /**
    @ingroup algorithm
    @brief Computes information gain on an integral input/output
    @see http://en.wikipedia.org/wiki/Information_gain_in_decision_trees
    */
   class InformationGainWeighted
   {
   public:
      /**
       @brief Compute the information gain IG(y|x) = E(y) - E(y|x)
       @param x the attribut values
       @param y the class values
       @param weights the weights. Note that the on all instances must sum to 1 (not necessarily for this function though)

       It can be seen as the amount of information saved by knowing x when transmiting y
       */
      template <class Vector1, class Vector2, class Vector3>
      double compute( const Vector1& x, const Vector2& y, const Vector3& weights ) const
      {
         EntropyWeighted entropy;
         return entropy.compute( y, weights ) - entropy.compute( x, y, weights );
      }
   };
}
}

#endif
