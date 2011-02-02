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
   template <class T, class Mapper, class Allocator>
   bool choleskyDecomposition( Matrix<T, Mapper, Allocator>& a )
   {
      int i, j, k;
      int size = static_cast<int>( a.sizex() );
      double sum;

      if ( a.sizex() != a.sizey() )
         return false;

      Matrix<T, Mapper, Allocator> res;
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
