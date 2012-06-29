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

#ifndef NLL_MATRIX_DECOMPOSITION_H_
# define NLL_MATRIX_DECOMPOSITION_H_

# include <limits>
# include "matrix.h"

#pragma warning( push )
#pragma warning( disable:4244 )

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Compute the LU decomposition with partial pivoting (intercahnge of rows). PA = LU.
    @param a the initial square matrix. After this call, the content is directly replaced by the
           2 triangular matrices arranged as:
           |b11 b12 b13|
           |a21 b22 b23|
           |a31 a32 b33|.
    @param per defines the row permutations
    @param d is 1 if the number of row interchange is even, -1 if not
    @note As the matrix are directly modified, beware if several matrices are sharing the same buffer.
          Code from numerical recipes
    */
	template <class type, class mapper, class allocator>
	bool luDecomposition(Matrix<type, mapper, allocator>& a, Buffer1D<size_t>& perm, type& d)
	{
      ensure( a.sizex() == a.sizey(), "only square matrix" );
		const type TINY=1.0e-20f;
		int i,imax = 0,j,k;
		type big,dum,sum,temp;
		bool error = false;

		int n=static_cast<int>( a.nrows() );
		Buffer1D<type> vv(n);
		perm = Buffer1D<size_t>(n);
		d=1.0;
		for (i=0;i<n;i++) {
			big=0.0;
			for (j=0;j<n;j++)
				if ((temp=absolute((double)a(i, j))) > big) big=temp;

         if ( absolute(big) <= std::numeric_limits<type>::epsilon() )
				return false;
			vv(i)=1.0f/big;
		}
		for (j=0;j<n;j++) {
			for (i=0;i<j;i++) {
				sum=a(i, j);
				for (k=0;k<i;k++) sum -= a(i, k)*a(k, j);
				a(i, j)=sum;
			}
			big=0.0;
			for (i=j;i<n;i++) {
				sum=a(i, j);
				for (k=0;k<j;k++) sum -= a(i, k)*a(k, j);
				a(i, j)=sum;
				if ((dum=vv(i)*absolute((double)sum)) >= big) {
					big=dum;
					imax=i;
				}
			}
			if (j != imax) {
				for (k=0;k<n;k++) {
					dum=a(imax, k);
					a(imax, k)=a(j, k);
					a(j, k)=dum;
				}
				d = -d;
				vv(imax)=vv(j);
			}
			perm(j)=imax;
			if (a(j, j) == 0.0) a(j, j)=TINY;
			if (j != n-1) {
				if (absolute(a(j, j)) <= std::numeric_limits<type>::epsilon())
					error = true;
				dum=1.0f/(a(j, j));
				for (i=j+1;i<n;i++) a(i, j) *= dum;
			}
		}
		return !error;
	}

   /**
    @ingroup core
    @brief Do the LU backsubstitution.

    Solve PAx = B, with A = LU
    */
	template <class type, class mapper, class allocator>
	bool luBackSubstitution(Matrix<type, mapper, allocator>& a, Buffer1D<size_t>& perm, Buffer1D<type>& b)
	{
		bool error = false;
		assert(b.size() == a.sizey()); // "bad dimention"
		int i,ii=0,j;
      size_t ip;
		type sum;

		int n=static_cast<int>( a.nrows() );
		for (i=0;i<n;i++) {
			ip=perm(i);
			sum=b(ip);
			b(ip)=b(i);
			if (ii != 0)
				for (j=ii-1;j<i;j++) sum -= a(i, j)*b(j);
			else if (sum != 0.0)
				ii=i+1;
			b(i)=sum;
		}
		for (i=n-1;i>=0;i--) {
			sum=b(i);
			for (j=i+1;j<n;j++) sum -= a(i, j)*b(j);
			if (absolute(a(i, i)) < std::numeric_limits<type>::epsilon())
				error = true;
			b(i)=sum/a(i, i);
		}
		return !error;
	}

   /**
    @ingroup core
    @brief Solve Ax = B using LU decomposition.
    */
	template <class type, class mapper, class allocator>
	Matrix<type, mapper, allocator> solve_lu(const Matrix<type, mapper, allocator>& a, const Buffer1D<type>& b)
	{
		Buffer1D<type> result(b);
		Matrix<type, mapper, allocator> m;
		m.clone(a);

		type d = 0;
		Buffer1D<size_t> perm;

		if (!luDecomposition(m, perm, d))
			return Matrix<type, mapper, allocator>();
		bool flag = luBackSubstitution(m, perm, result);
		assert(flag);
		return result;
	}
}
}

#pragma warning( pop )

#endif
