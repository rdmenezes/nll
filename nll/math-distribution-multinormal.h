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

#ifndef NLL_CORE_DISTRIBUTION_MULTINORMAL_H_
# define NLL_CORE_DISTRIBUTION_MULTINORMAL_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief generate normal multivariate distribution samples
           according to http://en.wikipedia.org/wiki/Multivariate_normal_distribution
    */
   class NormalMultiVariateDistribution
   {
      typedef Matrix<double>     MatrixT;

   public:
      typedef Buffer1D<double>   VectorT;

   public:
      /**
       @brief initialize the generator as we need to do a cholesky decomposition.
       */
      template <class Matrix, class Vector>
      NormalMultiVariateDistribution( const Vector& mean, const Matrix& covariance )
      {
         _triangularSub.import( covariance );
         _mean.clone( mean );
         _success = choleskyDecomposition( _triangularSub );
         ensure( _success, "the matrix is not a covariance matrix" );
      }

      /**
       @brief Generate samples using the mean/covariance matrix given.
              Just do sample = mean + triangular * vector, with vector=vector of N independent standard variables
       */
      VectorT generate() const
      {
         MatrixT r( _mean.size(), 1 );
         for ( unsigned n = 0; n < _mean.size(); ++n )
            r[ n ] = generateGaussianDistribution( 0, 1 );
         MatrixT result = _triangularSub * r;
         VectorT final( _mean.size() );
         for ( unsigned n = 0; n < _mean.size(); ++n )
            final[ n ] = result[ n ] + _mean[ n ];
         return final;
      }

   private:
      VectorT   _mean;
      MatrixT  _triangularSub;
      bool     _success;
   };
}
}

#endif
