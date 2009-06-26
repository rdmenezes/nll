#ifndef NLL_CORE_MATRIX_CHOLESKY_H_
# define NLL_CORE_MATRIX_CHOLESKY_H_

namespace nll
{
namespace core
{
   /**
    @biref Compute the cholesky decomposition of a real matrix.
           It construct the matrix L such as A = L.L^t
    @param m the matrix to decompose. It must be a positive definite square symmetric matrix.
    @return false if the matrix is not a square positive definite matrix
    @note the matrix is duplicated
    */
   template <class T, class Mapper>
   bool choleskyDecomposition( Matrix<T, Mapper>& a )
   {
      int i, j, k;
      int size = static_cast<int>( a.sizex() );
      double sum;

      if ( a.sizex() != a.sizey() )
         return false;

      Matrix<T, Mapper> res;
      res.clone( a );

      // compute the lower triangular
      for ( i = 0; i < size; i++ )
      {
         for ( j = i; j < size; j++ )
         {
            for ( sum = res( i, j ), k = i - 1; k >= 0; k-- )
               sum -= res( i, k ) * res( j, k );
            if ( i == j )
            {
               if ( sum <= 0 )
                  return false;
               res( i, i ) = sqrt( sum );
            } else res( j, i ) = sum / res( i, i );
         }
      }

      // set to 0 the upper triangular
      for ( i = 0; i < size; i++ )
      {
         for ( j = i + 1; j < size; j++ )
         {
            res( i, j ) = 0;
         }
      }

      a = res;
      return true;
   }
}
}

#endif