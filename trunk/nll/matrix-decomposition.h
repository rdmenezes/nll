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
    @brief Compute the LU decomposition. PA = LU
    @note code from numerical recipes
    */
	template <class type, class mapper>
	bool luDecomposition(Matrix<type, mapper>& a, Buffer1D<ui32>& perm, type& d)
	{
		const type TINY=1.0e-20f;
		int i,imax = 0,j,k;
		type big,dum,sum,temp;
		bool error = false;

		int n=a.nrows();
		Buffer1D<type> vv(n);
		perm = Buffer1D<ui32>(n);
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
	template <class type, class mapper>
	bool luBackSubstitution(Matrix<type, mapper>& a, Buffer1D<ui32>& perm, Buffer1D<type>& b)
	{
		bool error = false;
		assert(b.size() == a.sizey()); // "bad dimention"
		int i,ii=0,ip,j;
		type sum;

		int n=a.nrows();
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
	template <class type, class mapper>
	Matrix<type, mapper> solve_lu(const Matrix<type, mapper>& a, const Buffer1D<type>& b)
	{
		Buffer1D<type> result(b);
		Matrix<type, mapper> m;
		m.clone(a);

		type d = 0;
		Buffer1D<ui32> perm;

		if (!luDecomposition(m, perm, d))
			return Matrix<type, mapper>();
		bool flag = luBackSubstitution(m, perm, result);
		assert(flag);
		return result;
	}
}
}

#pragma warning( pop )

#endif
