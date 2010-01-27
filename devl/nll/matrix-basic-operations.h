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
   template <class T1, class Mapper1, class T2, class Allocator>
   void add( Matrix<T1, Mapper1, Allocator>& dst, const Matrix<T2, Mapper1, Allocator>& src )
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
   template <class T1, class Mapper1, class T2, class Allocator>
   void sub( Matrix<T1, Mapper1, Allocator>& dst, const Matrix<T2, Mapper1, Allocator>& src )
   {
      assert( dst.sizex() == src.sizex() );
      assert( dst.sizey() == src.sizey() );
      generic_sub<T1*, const T2*>( dst.getBuf(), src.getBuf(), dst.size() );
   }

   /**
    @ingroup core
    @brief Define matrix multiplication by an integral value.
    */
   template <class T1, class Mapper1, class Allocator>
   void mul( Matrix<T1, Mapper1, Allocator>& dst, const double val )
   {
      generic_mul_cte<T1*>( dst.getBuf(), val, dst.size() );
   }

   /**
    @ingroup core
    @brief Define matrix division by an integral value.
    */
   template <class T1, class Mapper1, class Allocator>
   void div( Matrix<T1, Mapper1, Allocator>& dst, const double val )
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
   template <class T1, class Mapper1, class Allocator>
   Matrix<T1, Mapper1, Allocator> mul( const Matrix<T1, Mapper1, Allocator>& op1, const Matrix<T1, Mapper1, Allocator>& op2 )
   {
      assert( op1.sizex() == op2.sizey() );
      Matrix<T1, Mapper1, Allocator> m(op1.sizey(), op2.sizex());

	   for (ui32 nx = 0; nx < op2.sizex(); ++nx)
		   for (ui32 ny = 0; ny < op1.sizey(); ++ny)
		   {
			   T1 val = 0;
			   for (ui32 n = 0; n < op1.sizex(); ++n)
            {
				   val += op1(ny, n) * op2(n, nx);
            }
			   m(ny, nx) = val;
		   }
	   return m;   
   }

   /**
    @brief Multiplies matrix by vector. Vector must define <code>size()</code> and <code>operator[]( unsigned )</code>
    */
   template <class T1, class Mapper1, class Mapper2, class Allocator, class Allocator2>
   Matrix<T1, Mapper1, Allocator> mul( const Matrix<T1, Mapper1, Allocator>& op1, const Buffer1D<T1, Mapper2, Allocator2>& op2 )
   {
      assert( op1.sizex() == op2.size() );
      Matrix<T1, Mapper1, Allocator> m( op1.sizey(), 1 );

	   for ( ui32 ny = 0; ny < op1.sizey(); ++ny )
	   {
		   T1 val = 0;
		   for ( ui32 n = 0; n < op1.sizex(); ++n )
			   val += op1( ny, n ) * op2[ n ];
		   m( ny, 0 ) = val;
	   }
	   return m;   
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class Allocator>
   Matrix<T, Mapper, Allocator> operator+( const Matrix<T, Mapper, Allocator>& lhs, const Matrix<T, Mapper, Allocator>& rhs )
   {
      Matrix<T, Mapper, Allocator> op;
      op.clone( lhs );
      add( op, rhs );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class Allocator>
   Matrix<T, Mapper, Allocator> operator+=( Matrix<T, Mapper, Allocator>& lhs, const Matrix<T, Mapper, Allocator>& rhs )
   {
      add( lhs, rhs );
      return lhs;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class Allocator>
   Matrix<T, Mapper, Allocator> operator-( const Matrix<T, Mapper, Allocator>& lhs, const Matrix<T, Mapper, Allocator>& rhs )
   {
      Matrix<T, Mapper, Allocator> op;
      op.clone( lhs );
      sub( op, rhs );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class Allocator>
   Matrix<T, Mapper, Allocator> operator-=( Matrix<T, Mapper, Allocator>& lhs, const Matrix<T, Mapper, Allocator>& rhs )
   {
      sub( lhs, rhs );
      return lhs;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class Allocator>
   Matrix<T, Mapper, Allocator> operator*( const Matrix<T, Mapper, Allocator>& lhs, const Matrix<T, Mapper, Allocator>& rhs )
   {
      return mul( lhs, rhs );
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2, class Allocator>
   Matrix<T, Mapper, Allocator> operator*( const Matrix<T, Mapper, Allocator>& lhs, const T2 val )
   {
      Matrix<T, Mapper, Allocator> op;
      op.clone( lhs );
      mul( op, val );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2, class Allocator>
   Matrix<T, Mapper, Allocator> operator*=( Matrix<T, Mapper, Allocator>& lhs, const T2 val )
   {
      mul( lhs, val );
      return lhs;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2, class Allocator>
   Matrix<T, Mapper, Allocator> operator*( const T2 val, const Matrix<T, Mapper, Allocator>& rhs)
   {
      Matrix<T, Mapper, Allocator> op;
      op.clone( rhs );
      mul( op, val );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2, class Allocator>
   Matrix<T, Mapper, Allocator> operator/( const Matrix<T, Mapper, Allocator>& lhs, const T2 val)
   {
      Matrix<T, Mapper, Allocator> op;
      op.clone( lhs );
      div( op, val );
      return op;
   }

   /**
    @ingroup core
    @brief Define operator on matrix.
    */
   template <class T, class Mapper, class T2, class Allocator>
   Matrix<T, Mapper, Allocator> operator/=( Matrix<T, Mapper, Allocator>& lhs, const T2 val)
   {
      div( lhs, val );
      return lhs;
   }
}
}

#endif
