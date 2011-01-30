#ifndef NLL_CORE_PSEUDO_INVERSE_H_
# define NLL_CORE_PSEUDO_INVERSE_H_

namespace nll
{
namespace core
{
   /**
    @brief Computes the Moore-Penrose pseudo inverse using SVD such as defined
           in http://en.wikipedia.org/wiki/Moore-Penrose_pseudoinverse#Singular_Value_Decomposition

           Definition: given pA the pseudo inverse of A,
           A*pA*A=A
           pA*A*pA=pA
           (A*pA)* = A*pA
           (pA*A)* = pA*A

           The Moore-Penrose pseudoinverse exists and is unique for any matrix. A matrix satisfying
           the first two conditions of the definition is known as a generalized inverse.
           Generalized inverses always exist but are not in general unique.

           A = U * S * V^t using SVD
           pA = V * pS * U^t, where pS is pseudo inverse of a diagonal matrix, which is 1/S(x,x)
    */
   template <class Type, class Mapper, class Allocator>
   Matrix<Type, Mapper, Allocator> pseudoInverse( const Matrix<Type, Mapper, Allocator>& m )
   {
      typedef Matrix<Type, Mapper, Allocator> MatrixT;
      typedef Buffer1D<Type> Vector;

      MatrixT u;
      u.clone( m );

      MatrixT v;
      Vector s;
      bool res = svdcmp( u, s, v );
      ensure( res, "can't compute SVD" );

      core::transpose( u );
      for ( ui32 x = 0; x < v.sizex(); ++x )
      {
         const double f = ( fabs( s[ x ] ) > 1e-10 ) ? 1.0 / s[ x ] : 0;
         for ( ui32 y = 0; y < v.sizey(); ++y )
         {
            v( y, x ) *= f;
         }
      }
      return v * u;
   }
}
}

#endif