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

#ifndef NLL_MATH_DISTRIBUTION_UNIFORM_H_
# define NLL_MATH_DISTRIBUTION_UNIFORM_H_

# include <assert.h>

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief generate a sample of a specific uniform distribution
    @param min the min of the distribution
    @param max the max of the distribution
    @return a sample of this distribution
    */
   inline double NLL_API generateUniformDistribution( const double min, const double max )
   {
      assert( min <= max );
      return static_cast<double>( rand() ) / RAND_MAX * ( max - min ) + min;
   }

   /**
    @ingroup core
    @brief generate a sample of a specific uniform distribution
    @param min the min of the distribution
    @param max the max of the distribution
    @return a sample of this distribution
    */
   inline double NLL_API generateUniformDistributionf( const float min, const float max )
   {
      assert( min <= max );
      return static_cast<float>( rand() ) / RAND_MAX * ( max - min ) + min;
   }

   /**
    @ingroup core
    @brief generate a sample of a specific uniform distribution
    @param min the min of the distribution
    @param max the max of the distribution
    @return a sample of this distribution
    */
   inline int NLL_API generateUniformDistributioni( const int min, const int max )
   {
      assert( min <= max );
      const int interval = max - min;
      return ( rand() % interval ) + min;
   }
}
}

#endif
