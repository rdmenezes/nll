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
