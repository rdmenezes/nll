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

   /**
    @ingroup core
    @brief Compute the covariance of a list of vectors. Each row is a sample.
           Cov = sum ( X^tX ) / ( nbSamples - 1 )
    @note IMPORTANT it is assumed the points are already centred
    @return the covariance matrix
    Supposed to have more rows than column else the covariance matrix won't be invertible

    @note the matrix is symetric, so only compute upper triangular and copy the lower one
    */
   template <class type, class mapper, class allocator>
	inline Matrix<type, mapper, allocator> covarianceCentred(const Matrix<type, mapper, allocator>& vec)
	{
      assert( vec.sizey() );

		Matrix<type, mapper, allocator> cov( vec.sizex(), vec.sizex() );
		for ( ui32 i = 0; i < vec.sizex(); ++i )
      {
         // compute upper triangular
			for ( ui32 j = i; j < vec.sizex(); ++j )
			{
				type sum = 0;
				for ( ui32 k = 0; k < vec.sizey(); ++k )
					sum += vec( k, i ) * vec( k, j );
				cov( i, j ) = sum / (type)( vec.sizey() );
			}

         // copy lower triangular
         for (ui32 j = 0; j < i; ++j)
         {
            cov(i, j) = cov(j, i);
         }
      }
		return cov;
	}

   /**
    @ingroup core
    @brief Compute the of a list of points.
    @note Points must define:
     size()
     Point operator[]( index ) const

     Point must define:
     size()
     Point operator[]( index ) const

    @return a vector
    */
   template <class Points, class Output>
	inline Output meanData( const Points& points )
	{
      if ( points.size() == 0 )
         return Output();
      const ui32 nbFeatures = static_cast<ui32>( points[ 0 ].size() );
      Output mean( nbFeatures );

		for (ui32 ny = 0; ny < points.size(); ++ny)
      {
			for (ui32 nx = 0; nx < nbFeatures; ++nx)
         {
				mean[ nx ] += points[ ny ][ nx ];
         }
      }
      for (ui32 nx = 0; nx < nbFeatures; ++nx)
      {
         mean[ nx ] /= points.size();
      }
		return mean;
	}

   /**
    @ingroup core
    @brief Compute the covariance of points arranged in rows
    */
   template <class PointsRow, class OutputMean>
	inline Matrix<double> covariance(const PointsRow& points, OutputMean* exportedMean = 0)
	{
      assert( points.size() );
      if ( points.size() == 0 )
         return Matrix<double>();
      const ui32 nbFeatures = (ui32)points[ 0 ].size();

		Matrix<double> cov( nbFeatures, nbFeatures );
		OutputMean mean = meanData<PointsRow, OutputMean>( points );
		for (ui32 i = 0; i < nbFeatures; ++i)
      {
         // compute upper triangular
			for (ui32 j = i; j < nbFeatures; ++j)
			{
				double sum = 0;
				for (ui32 k = 0; k < points.size(); ++k)
					sum += ( points[ k ][ i ] - mean[ i ] ) * ( points[ k ][ j ] - mean[ j ] );
				cov( i, j ) = sum / points.size();
			}

         // copy lower triangular
         for ( ui32 j = 0; j < i; ++j )
         {
            cov( i, j ) = cov( j, i );
         }
      }
		if ( exportedMean )
			*exportedMean = mean;
		return cov;
	}

   /**
    @ingroup core
    @brief Compute the covariance matrix on an indexed list of points only
    @note IMPORTANT it is assumed the mean of the indexed points is 0
    */
   template <class Points>
   Matrix<double> covariance( const Points& points, const std::vector<ui32>& index )
   {
      if ( index.size() == 0 || points.size() == 0 )
         return Matrix<double>();
      const ui32 nbFeatures = points[ 0 ].size();
      Matrix<double> cov( nbFeatures, nbFeatures );
		for (ui32 i = 0; i < nbFeatures; ++i)
      {
         // compute upper triangular
			for (ui32 j = i; j < nbFeatures; ++j)
			{
				double sum = 0;
				for (ui32 k = 0; k < index.size(); ++k)
            {
               const ui32 p = index[ k ];
					sum += ( points[ p ][ i ] ) * ( points[ p ][ j ] );
            }
				cov( i, j ) = sum / index.size();
			}

         // copy lower triangular
         for ( ui32 j = 0; j < i; ++j )
         {
            cov( i, j ) = cov( j, i );
         }
      }
      return cov;
   }

   /**
    @ingroup core
    @brief computes the covariance of 2 sets of points
    @param p1 it must have the same size (nb points & dimensions) as p2
    */
   template <class Points1, class Points2>
   core::Matrix<double> covariance( const Points1& p1, const Points2& p2, core::Buffer1D<double>& mean1_out, core::Buffer1D<double>& mean2_out )
   {
      typedef core::Buffer1D<double> Vector;
      typedef core::Matrix<double>   Matrix;

      // constants
      ensure( p1.size() == p2.size() && p2.size() > 0, "must be pair of points, not empty" );   
      const ui32 nbPoints = static_cast<ui32>( p1.size() );
      const ui32 nbDim = static_cast<ui32>( p1[ 0 ].size() );
      ensure( p2[ 0 ].size() == nbDim, "must be the same dimension" );

      // processing
      core::Buffer1D<double> mean1 = core::meanData<Points1, Vector>( p1 );
      core::Buffer1D<double> mean2 = core::meanData<Points2, Vector>( p2 );

      // compute the covariance
      Matrix cov( nbDim, nbDim, false );
      for ( ui32 i = 0; i < nbDim; ++i )
      {
         for ( ui32 j = 0; j < nbDim; ++j )
         {
            double accum = 0;
            for ( ui32 k = 0; k < nbPoints; ++k )
            {
               accum += ( p1[ k ][ i ] - mean1[ i ] ) * ( p2[ k ][ j ] - mean2[ j ] );
            }
            cov( j, i ) = accum / nbPoints;
         }
      }

      mean1_out = mean1;
      mean2_out = mean2;
      return cov;
   }
}
}

#endif
