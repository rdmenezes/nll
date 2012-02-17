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

#ifndef NLL_CORE_MATH_STATISTIC_INDEPENDENCE_H_
# define NLL_CORE_MATH_STATISTIC_INDEPENDENCE_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Assume point is a set of random variable at t=0...size,
           computes E(f(x))E(f(y)) - E(f(x)(f(y)) (1)
    @note if 2 random variable are independent, they must satisfy (1) == 0 for any function f
    @return if the 2 features are statistically independent returns 0 for any function f
    @TODO this is not good at all! it is just testing with func id, but should be with much more funcs...
    */
   template <class Points>
   double checkStatisticalIndependence( const Points& points, ui32 v1, ui32 v2 )
   {
      double xa = 0;
      double xb = 0;
      double xc = 0;
      for ( ui32 n = 0; n < points.size(); ++n )
      {
         xa += ( points[ n ][ v1 ] );
         xb += ( points[ n ][ v2 ] );
         xc += ( points[ n ][ v1 ] ) * ( points[ n ][ v2 ] );
      }
      return xa / points.size() * xb / points.size() - xc / points.size();
   }
}
}

#endif