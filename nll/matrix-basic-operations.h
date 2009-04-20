#ifndef NLL_MATRIX_BASIC_OPERATIONS_H_
# define NLL_MATRIX_BASIC_OPERATIONS_H_

# include "matrix.h"
# include "generic-operations.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define matrix addition.

    Matrices should have the same dimensions
    */
   template <class T1, class Mapper1, class T2>
   void add( Matrix<T1, Mapper1>& dst, const Matrix<T2, Mapper1>& src )
   {
      assert( dst.sizex() == src.sizex() );
      assert( dst.sizey() == src.sizey() );
      generic_add<T1*, const T2*>( dst.getBuf(), src.getBuf(), dst.size() );
   }

   /**
    @ingroup core
    @brief Define matrix substraction.

    Matrices should have the same dimensions
    */
   template <class T1, class Mapper1, class T2>
   void sub( Matrix<T1, Mapper1>& dst, const Matrix<T2, Mapper1>& src )
   {
      assert( dst.sizex() == src.sizex() );
      assert( dst.sizey() == src.sizey() );
      generic_sub<T1*, const T2*>( dst.getBuf(), src.getBuf(), dst.size() );
   }

   /**
    @ingroup core
    @brief Define matrix multiplication by an integral value.
    */
   template <class T1, class Mapper1>
   void mul( Matrix<T1, Mapper1>& dst, const double val )
   {
      generic_mul_cte<T1*>( dst.getBuf(), val, dst.size() );
   }

   /**
    @ingroup core
    @brief Define matrix division by an integral value.
    */
   template <class T1, class Mapper1>
   void div( Matrix<T1, Mapper1>& dst, const double val )
   {
      generic_div_cte<T1*>( dst.getBuf(), val, dst.size() );
   }

   /**
    @ingroup core
    @brief Define matrix multiplication.

    Matrix A: m.n
    Matrix B: n:p
    compute A * B

    @note naive implementation (not using block decomposition)
    */
   template <class T1, class Mapper1>
   Matrix<T1, Mapper1> mul( const Matrix<T1, Mapper1>& op1, const Matrix<T1, Mapper1>& op2 )
   {
      assert( op1.sizex() == op2.sizey() );
      Matrix<T1, Mapper1> m(op1.sizey(), op2.sizex());

	   for (ui32 nx = 0; nx < op2.sizex(); ++nx)
		   for (ui32 ny = 0; ny < op1.sizey(); ++ny)
		   {
			   T1 val = 0;
			   for (ui32 n = 0; n < op1.sizex(); ++n)
				   val += op1(ny, n) * op2(n, nx);
			   m(ny, nx) = val;
		   }
	   return m;   
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper>
   Matrix<T, Mapper> operator+( const Matrix<T, Mapper>& lhs, const Matrix<T, Mapper>& rhs )
   {
      Matrix<T, Mapper> op;
      op.clone( lhs );
      add( op, rhs );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper>
   Matrix<T, Mapper> operator+=( Matrix<T, Mapper>& lhs, const Matrix<T, Mapper>& rhs )
   {
      add( lhs, rhs );
      return lhs;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper>
   Matrix<T, Mapper> operator-( const Matrix<T, Mapper>& lhs, const Matrix<T, Mapper>& rhs )
   {
      Matrix<T, Mapper> op;
      op.clone( lhs );
      sub( op, rhs );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper>
   Matrix<T, Mapper> operator-=( Matrix<T, Mapper>& lhs, const Matrix<T, Mapper>& rhs )
   {
      sub( lhs, rhs );
      return lhs;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper>
   Matrix<T, Mapper> operator*( const Matrix<T, Mapper>& lhs, const Matrix<T, Mapper>& rhs )
   {
      return mul( lhs, rhs );
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2>
   Matrix<T, Mapper> operator*( const Matrix<T, Mapper>& lhs, const T2 val )
   {
      Matrix<T, Mapper> op;
      op.clone( lhs );
      mul( op, val );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2>
   Matrix<T, Mapper> operator*=( Matrix<T, Mapper>& lhs, const T2 val )
   {
      mul( lhs, val );
      return lhs;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2>
   Matrix<T, Mapper> operator*( const T2 val, const Matrix<T, Mapper>& rhs)
   {
      Matrix<T, Mapper> op;
      op.clone( rhs );
      mul( op, val );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2>
   Matrix<T, Mapper> operator/( const Matrix<T, Mapper>& lhs, const T2 val)
   {
      Matrix<T, Mapper> op;
      op.clone( lhs );
      div( op, val );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2>
   Matrix<T, Mapper> operator/=( Matrix<T, Mapper>& lhs, const T2 val)
   {
      div( lhs, val );
      return lhs;
   }
}
}

#endif
