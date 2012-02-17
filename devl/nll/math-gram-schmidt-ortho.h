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

#ifndef NLL_CORE_GRAM_SCHMIDT_ORTHO_H_
# define NLL_CORE_GRAM_SCHMIDT_ORTHO_H_

namespace nll
{
namespace core
{
   /**
    @brief Traits for inner product
    */
   class TraitInnerProductDot
   {
   public:
      template <class Vector>
      static double inner( const Vector& v1, const Vector& v2 )
      {
         assert( v1.size() == v2.size() );
         double accum = 0;
         for ( ui32 n = 0; n < v1.size(); ++n )
         {
            accum += v1[  n ] * v2[ n ];
         }
         return accum;
      }
   };

   /**
    @ingroup core
    @brief stabilized orthonormalization with gram-schmidt of a set of vectors

    @note TraitInnerProduct defines the inner product to be used for the orthonotmalization
          it must defines "static double inner(vector, vector)"
    @see http://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process
    */
   template <class TraitInnerProduct = TraitInnerProductDot>
   class GramSchmidt
   {
   public:
      template <class Vectors>
      static Vectors orthogonalize( const Vectors& vectors )
      {
         if ( vectors.size() == 0 )
            return Vectors();

         typedef typename Vectors::value_type   Vector;
         typedef typename Vector::value_type    value_type;

         Vectors results = vectors;
         const ui32 size = static_cast<ui32>( vectors[ 0 ].size() );
         std::vector<double> precomputed( vectors.size() );

         // orthogonalize
         for ( ui32 n = 0; n < vectors.size(); ++n )
         {
            // project
            for ( ui32 proj = 0; proj < n; ++proj )
            {
               const double inner = TraitInnerProduct::inner( results[ proj ], results[ n ] );
               const double innerb = precomputed[ proj ];
               for ( ui32 nn = 0; nn < size; ++nn )
               {
                  results[ n ][ nn ] -= results[ proj ][ nn ] * inner / innerb;
               }
            }

            // normalize the vector
            const value_type norm = generic_norm2<Vector, value_type>( results[ n ], size );
            generic_div_cte<Vector>( results[ n ], norm, size );

            // precompute one of the inner product
            precomputed[ n ] = TraitInnerProduct::inner( results[ n ], results[ n ] );
         }
         return results;
      }

      /**
       @brief Assumed vectors [0..size - 1 ] are orthonormals, and we want to orthgonalize
              the last vector
       */
      template <class Vectors>
      static void orthogonalizeLast( Vectors& vectors )
      {
         if ( vectors.size() <= 1 )
            return;

         typedef typename Vectors::value_type   Vector;
         typedef typename Vector::value_type    value_type;

         const ui32 id = static_cast<ui32>( vectors.size() - 1 );
         const ui32 size = static_cast<ui32>( vectors[ 0 ].size() );

         // project
         for ( ui32 proj = 0; proj < id; ++proj )
         {
            const double inner = TraitInnerProduct::inner( vectors[ proj ], vectors[ id ] );
            const double innerb = TraitInnerProduct::inner( vectors[ proj ], vectors[ proj ] );
            for ( ui32 nn = 0; nn < size; ++nn )
            {
               vectors[ id ][ nn ] -= vectors[ proj ][ nn ] * inner / innerb;
            }
         }

         // normalize the vector
         const value_type norm = generic_norm2<Vector, value_type>( vectors[ id ], size );
         generic_div_cte<Vector>( vectors[ id ], norm, size );
      }
   };
}
}

#endif
