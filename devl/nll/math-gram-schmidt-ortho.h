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
