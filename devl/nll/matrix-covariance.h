#ifndef NLL_MATRIX_COVARIANCE_H_
# define NLL_MATRIX_COVARIANCE_H_

# include "matrix.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Compute the mean row of a matrix.
    @return a vector
    */
   template <class type, class mapper, class allocator>
	inline Buffer1D<type> meanRow(const Matrix<type, mapper, allocator>& vec, ui32 start, ui32 end)
	{
		assert(start < vec.sizey() && end < vec.sizey() && start <= end);
		Buffer1D<type> mean(vec.sizex());
		for (ui32 ny = start; ny <= end; ++ny)
			for (ui32 nx = 0; nx < vec.sizex(); ++nx)
				mean(nx) += vec(ny, nx);
		generic_div_cte<type*>(mean.getBuf(), end - start + 1, vec.sizex());
		return mean;
	}

   /**
    @ingroup core
    @brief Compute the mean column of a matrix.
    @return a vector
    */
   template <class type, class mapper, class allocator>
	inline Buffer1D<type> meanCol(const Matrix<type, mapper, allocator>& vec, ui32 start, ui32 end)
	{
		assert(start < vec.sizex() && end < vec.sizex() && start <= end);
		Buffer1D<type> mean(vec.sizey());
		for (ui32 ny = 0; ny < vec.sizey(); ++ny)
         for (ui32 nx = start; nx <= end; ++nx)
				mean(ny) += vec(ny, nx);
		generic_div_cte<type*>(mean.getBuf(), end - start + 1, vec.sizey());
		return mean;
	}

   /**
    @ingroup core
    @brief Compute the covariance of a list of vectors. Each row is a sample.
           Cov = sum ( X^tX ) / ( nbSamples - 1 )

    @param start the starting row from what the covariance matrix will be generated
    @param end the ending row from what the covariance matrix will be generated
    @param exportedMean if exportedMean, then it is exported in this vector
    @return the covariance matrix
    Supposed to have more rows than column else the covariance matrix won't be invertible

    @note the matrix is symetric, so only compute upper triangular and copy the lower one
    */
   template <class type, class mapper, class allocator>
	inline Matrix<type, mapper, allocator> covariance(const Matrix<type, mapper, allocator>& vec, ui32 start, ui32 end, Buffer1D<type>* exportedMean = 0)
	{
      assert( vec.sizey() );

		Matrix<type, mapper, allocator> cov(vec.sizex(), vec.sizex());
		Buffer1D<type> mean = meanRow(vec, start, end);
		for (ui32 i = 0; i < vec.sizex(); ++i)
      {
         // compute upper triangular
			for (ui32 j = i; j < vec.sizex(); ++j)
			{
				type sum = 0;
				for (ui32 k = start; k <= end; ++k)
					sum += (vec(k, i) - mean(i)) * (vec(k, j) - mean(j));
				cov(i, j) = sum / (type)(end - start + 1 - 1);
			}

         // copy lower triangular
         for (ui32 j = 0; j < i; ++j)
         {
            cov(i, j) = cov(j, i);
         }
      }
		if (exportedMean)
			*exportedMean = mean;
		return cov;
	}
}
}

#endif
