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

#ifndef NLL_CORE_MATH_PDF_GAUSSIAN_H_
# define NLL_CORE_MATH_PDF_GAUSSIAN_H_

namespace nll
{
namespace core
{
   /**
    @TODO replace all independent instances of gaussian PDF with this one

    @brief Gaussian probability distribution function
    @see http://en.wikipedia.org/wiki/Multivariate_normal_distribution
    
    We represent a gaussian PDF = (2*pi)^-k/2 * |cov|^-0.5 * e( -0.5 * (X-u)^t * cov^-1 * (X-u) )
    */
   template <class T>
   class ProbabilityDistributionFunctionGaussian
   {
   public:
      typedef T                  value_type;
      typedef core::Buffer1D<T>  Vector;
      typedef core::Matrix<T>    Matrix;

   public:
      ProbabilityDistributionFunctionGaussian()
      {}

      ProbabilityDistributionFunctionGaussian( const Matrix& covariance, const Vector& mean )
      {
         set( covariance, mean );
      }

      /**
       @brief set the PDF parameters
       */
      template <class MatrixT, class VectorT>
      void set( const MatrixT& covariance, const VectorT& mean )
      {
         STATIC_ASSERT( IsFloating<T>::value );

         ensure( covariance.sizex() == covariance.sizey(), "covariance matrix must be square" );
         ensure( mean.size() == covariance.sizex(), "vector and covariance size must match" );
         bool isDiag = true;

         _tmp = Vector( mean.size() );
         _cov = Matrix( mean.size(), mean.size(), false );
         for ( ui32 y = 0; y < _cov.sizey(); ++y )
         {
            for ( ui32 x = 0; x < _cov.sizex(); ++x )
            {
               const value_type val = covariance( y, x );
               if ( x != y && fabs( covariance( y, x ) ) > std::numeric_limits<T>::epsilon() )
               {
                  isDiag = false;
               }

               _cov( y, x ) = val;
            }
         }

         _isCovDiagonal = isDiag;
         _mean = Vector( mean.size(), false );
         _isMeanZero = true;
         for ( ui32 n = 0; n < mean.size(); ++n )
         {
            value_type val = static_cast<value_type>( mean[ n ] );
            if ( fabs( val ) > std::numeric_limits<value_type>::epsilon() )
            {
               _isMeanZero = false;
            }
            _mean[ n ] = val;
         }

         _covInv.import( covariance );
         value_type det = 0;
         const bool success = core::inverse( _covInv, &det );
         ensure( success && det > 0, "problem in covariance matrix" );

         const value_type cte = std::pow( core::PI * 2.0, - static_cast<f64>( mean.size() ) / 2 );
         _cte = cte / sqrt( det );
      }

      /**
       @brief evaluate the PDF at the position <val>
       @note optimization for diagonal covariance matrix/0-mean vector
       */
      value_type eval( const Vector& val ) const
      {
         assert( val.size() == _tmp.size() );
         if ( _isMeanZero )
         {
            if ( _isCovDiagonal )
            {
               value_type accum = 0;
               for ( ui32 n = 0; n < val.size(); ++n )
               {
                  accum += val[ n ] * val[ n ] * _covInv( n, n );
               }
               return std::exp( -0.5 * accum ) * _cte;
            } else {
               return std::exp( -0.5 * fastDoubleMultiplication( val, _covInv ) ) * _cte;
            }
         } else {
            for ( ui32 n = 0; n < val.size(); ++n )
            {
               _tmp[ n ] = val[ n ] - _mean[ n ];
            }

            if ( _isCovDiagonal )
            {
               T accum = 0;
               for ( ui32 n = 0; n < val.size(); ++n )
               {
                  accum += _tmp[ n ] * _tmp[ n ] * _covInv( n, n );
               }
               return std::exp( -0.5 * accum ) * _cte;
            } else {
               return std::exp( -0.5 * fastDoubleMultiplication( _tmp, _covInv ) ) * _cte;
            }
         }
      }

   private:
      Matrix   _cov;
      Vector   _mean;
      mutable Vector   _tmp;        // temporary value for computation. To avoids allocating memory on the heap
      bool     _isCovDiagonal;      // true is the covariance matrix is diagonal
      bool     _isMeanZero;         // true is mean vector is 0

      value_type  _cte;             // = (2*pi)^-k/2 * |cov|^-0.5
      Matrix      _covInv;          // = inv(cov)
   };
}
}

#endif