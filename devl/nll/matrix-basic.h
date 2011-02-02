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
	double det( const Matrix<type, mapper, allocator>& a )
	{
		assert(a.sizex() == a.sizey()); //  "non square matrix"
		Buffer1D<ui32> perm(a.sizex());
		
		double d;
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
			for (ui32 nx = 0; nx < m.sizey(); ++nx)
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
}
}

#endif
