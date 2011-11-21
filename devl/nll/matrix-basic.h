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

#ifndef NLL_MATRIX_BASIC_H_
# define NLL_MATRIX_BASIC_H_

# include "matrix-decomposition.h"
# include "type-traits.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Inverse a square matrix.
    @param a matrix to be inversed
    @param determinant export the determinant is necessary
    @return false if the matrix is singular
    @note use a LU decomposition internally, complexity N^3
          BEWARE: the matrix a is directly modified!
    */
   template <class type, class mapper, class allocator>
	bool inverse(Matrix<type, mapper, allocator>& a, type* determinant = 0)
	{
		assert(a.sizex() == a.sizey()); //  "non square matrix"
		Matrix<type, mapper, allocator> y(a.sizey(), a.sizex());
		Buffer1D<ui32> perm(a.sizex());
		Buffer1D<type> col(a.sizex());
		
		type d;
		bool ok = luDecomposition(a, perm, d);
		if (!ok)
			return false;
		if (determinant)
		{
			for (ui32 n = 0; n < a.sizex(); ++n)
				d *= a(n, n);
			*determinant = d;
		}

		for (ui32 j = 0; j < a.sizex(); ++j)
		{
			for (ui32 i = 0; i < a.sizex(); ++i)
				col(i) = 0.0;
			col(j) = 1.0;
			bool flag = luBackSubstitution<type, mapper, allocator>(a, perm, col);
			if (!flag)
				return false;
			for (ui32 i = 0; i < a.sizex(); ++i)
				y(i, j) = col(i);
		}
		a = y;
		return true;
	}


   template <class type, class mapper, class allocator>
	type det( const Matrix<type, mapper, allocator>& a )
	{
		assert(a.sizex() == a.sizey()); //  "non square matrix"
		Buffer1D<ui32> perm(a.sizex());
		
		type d;
      Matrix<type, mapper, allocator> cp;
      cp.clone( a );

		bool ok = luDecomposition(cp, perm, d);
      if ( !ok )
         return 0;

		for (ui32 n = 0; n < cp.sizex(); ++n)
			d *= (double)cp(n, n);
		return d;
	}

   /**
    @ingroup core
    @brief Generate an identity matrix of a fixed size.
    */
   template <class type, class mapper, class allocator>
   Matrix<type, mapper, allocator> identity( ui32 n )
   {
      Matrix<type, mapper, allocator> id( n, n );
      for (ui32 nn = 0; nn < n; ++nn)
         id( nn, nn ) = 1;
      return id;
   }

   /**
    @brief create an identity matrix (if non square, only elements i == j will be set to 1, else 0)
    */
   template <class type, class mapper, class allocator>
   void identity( Matrix<type, mapper, allocator>& m )
   {
      for (ui32 nx = 0; nx < m.sizex(); ++nx)
      {
			for (ui32 ny = 0; ny < m.sizey(); ++ny)
         {
            m( ny, nx ) = nx == ny;
         }
      }
   }

   /**
    @ingroup core
    @brief Generate an identity matrix of a fixed size.
    */
   template <class Matrix>
   Matrix identityMatrix( ui32 n )
   {
      Matrix id( n, n );
      for (ui32 nn = 0; nn < n; ++nn)
         id( nn, nn ) = 1;
      return id;
   }

   /**
    @ingroup core
    @brief Generate a matrix of a fixed size filled with a constant value.
    @param val the matrix is filled with this value
    */
   template <class type, class mapper, class allocator>
   Matrix<type, mapper, allocator> null( ui32 ny, ui32 nx, type val = 0 )
   {
      Matrix<type, mapper, allocator> null( ny, nx );
      for (ui32 nn = 0; nn < ny * nx; ++nn)
         null( nn ) = val;
      return null;
   }

   /**
    @ingroup core
    @brief Compute the trace of the matrix (sum_i (a_ii)).
    */
   template <class type, class mapper, class allocator>
   double trace( const Matrix<type, mapper, allocator>& m )
   {
      ensure( m.sizex() == m.sizey(), "operation is only defined for square matrix" );
      double accum = 0;
      for (ui32 nn = 0; nn < m.sizex(); ++nn)
         accum += static_cast<double>( m( nn, nn ) );
      return accum;
   }

   /**
    @ingroup core
    @brief Transpose a matrix.
    */
   template <class type, class mapper, class allocator>
   void transpose( Matrix<type, mapper, allocator>& m )
	{
		if (m.sizex() == 1 || m.sizey() == 1)
		{
         m.ref();
         Matrix<type, mapper, allocator> nmat( m, m.sizex(), m.sizey() );
         m = nmat;
			return;
		}
		if (m.sizex() == m.sizey())
			for (ui32 nx = 0; nx < m.sizex(); ++nx)
				for (ui32 ny = 0; ny < nx; ++ny)
					std::swap(m(ny, nx), m(nx, ny));
		else
		{
			Matrix<type, mapper, allocator> nn(m.sizex(), m.sizey());
			for (ui32 nx = 0; nx < m.sizex(); ++nx)
				for (ui32 ny = 0; ny < m.sizey(); ++ny)
					nn(nx, ny) = m(ny, nx);
			m = nn;
		}
	}

   /**
     @brief Computes v * v^t in place
    */
   template <class type, class mapper, class allocator, class Vector>
   void mulidt( const Vector& v, Matrix<type, mapper, allocator>& out )
	{
      out = Matrix<type, mapper, allocator>( v.size(), v.size(), false );
      for ( ui32 y = 0; y < v.size(); ++y )
      {
         for ( ui32 x = 0; x < v.size(); ++x )
         {
            out( y, x ) = v[ x ] * v[ y ];
         }
      }
   }

      /**
    @ingroup core
    @brief Computes quickly v^t * m * v
    */
   template <class T, class IndexMapper2D, class AllocatorT, class Vector>
   typename T fastDoubleMultiplication( const Vector& v, const Matrix<T, IndexMapper2D, AllocatorT>& m )
   {
      typedef T value_type;

      const ui32 sizex = m.sizex();
      const ui32 sizey = m.sizey();

      ensure( v.size() == sizex, "dim don't match" );

      value_type accum = 0;
      for ( ui32 y = 0; y < sizey; ++y )
      {
         const value_type px = v[ y ];
         for ( ui32 x = 0; x < sizex; ++x )
         {
            accum += m( y, x ) * px * v[ x ];
         }
      }
      return accum;
   }

   /**
    @ingroup core
    @brief Recompose a matrix given 2 index vector X and Y so that the matrix has this format:
             src = | XX XY |
                   | YX XX |
    */
   template <class T, class IndexMapper2D, class AllocatorT, class Index>
   void partitionMatrix( const core::Matrix<T, IndexMapper2D, AllocatorT>& src,
                         const Index& x, const Index& y,
                         core::Matrix<T, IndexMapper2D, AllocatorT>& xx,
                         core::Matrix<T, IndexMapper2D, AllocatorT>& yy,
                         core::Matrix<T, IndexMapper2D, AllocatorT>& xy,
                         core::Matrix<T, IndexMapper2D, AllocatorT>& yx )
   {
      typedef core::Matrix<T, IndexMapper2D, AllocatorT> MatrixT;

      // now create the submatrix XX YY XY YX
      xx = MatrixT( (ui32)x.size(), (ui32)x.size() );
      for ( ui32 ny = 0; ny < xx.sizey(); ++ny )
      {
         const ui32 idy = x[ ny ];
         for ( ui32 nx = 0; nx < xx.sizex(); ++nx )
         {
            const ui32 idx = x[ nx ];
            xx( ny, nx ) = src( idy, idx );
         }
      }

      yy = MatrixT( (ui32)y.size(), (ui32)y.size() );
      for ( ui32 ny = 0; ny < yy.sizey(); ++ny )
      {
         const ui32 idy = y[ ny ];
         for ( ui32 nx = 0; nx < yy.sizex(); ++nx )
         {
            const ui32 idx = y[ nx ];
            yy( ny, nx ) = src( idy, idx );
         }
      }

      xy = MatrixT( (ui32)x.size(), (ui32)y.size() );
      for ( ui32 ny = 0; ny < xy.sizex(); ++ny )
      {
         const ui32 idy = y[ ny ];
         for ( ui32 nx = 0; nx < xy.sizey(); ++nx )
         {
            const ui32 idx = x[ nx ];
            xy( nx, ny ) = src( idx, idy );
         }
      }

      yx = MatrixT( (ui32)y.size(), (ui32)x.size() );
      for ( ui32 ny = 0; ny < xy.sizey(); ++ny )
      {
         const ui32 idy = x[ ny ];
         for ( ui32 nx = 0; nx < xy.sizex(); ++nx )
         {
            const ui32 idx = y[ nx ];
            yx( nx, ny ) = src( idx, idy );
         }
      }
   }
}
}

#endif
