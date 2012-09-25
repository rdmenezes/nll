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

#ifndef NLL_MATH_DISTRIBUTION_GAUSSIAN_H_
# define NLL_MATH_DISTRIBUTION_GAUSSIAN_H_

# include <cmath>

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief generate a sample of a specific gaussian distribution using the Box-Muller transform.
           See http://en.wikipedia.org/wiki/Box-Muller_transform
    @param mean the mean of the distribution
    @param stddev the standard deviation of the distribution
    @return a sample of this distribution
    @note 2 values instead of 1 could be generated for almost free
    */
   inline double NLL_API generateGaussianDistributionStddev( const double mean, const double stddev )
   {
      double u1 = ( static_cast<double> ( rand() ) + 1 ) / ( (double)RAND_MAX + 1 );
      double u2 = ( static_cast<double> ( rand() ) + 1 ) / ( (double)RAND_MAX + 1 );
      assert( -2 * log( u1 ) >= 0 );
      double t1 = sqrt( -2 * log( u1 ) ) * cos( 2 * core::PI * u2 );

      return mean + stddev * t1;
   }

   /**
    @ingroup core
    @brief generate a sample of a specific gaussian distribution using the Box-Muller transform.
           See http://en.wikipedia.org/wiki/Box-Muller_transform
    @param mean the mean of the distribution
    @param variance the variance of the distribution
    @return a sample of this distribution
    @note 2 values instead of 1 could be generated for almost free
    */
   inline double NLL_API generateGaussianDistribution( const double mean, const double variance )
   {
      const double stddev = std::sqrt( variance );
      return generateGaussianDistributionStddev( mean, stddev );
   }
}
}

#endif
