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

#ifndef NLL_CORE_MATH_KURTOSIS_H_
# define NLL_CORE_MATH_KURTOSIS_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief returns the kurtosis of a random variable
    @see http://en.wikipedia.org/wiki/Kurtosis

    Kurtosis of known distribution:
      - Laplace distribution, a.k.a. double exponential distribution, excess kurtosis = 3
      - hyperbolic secant distribution, excess kurtosis = 2
      - logistic distribution, excess kurtosis = 1.2
      - normal distribution, excess kurtosis = 0
      - raised cosine distribution, excess kurtosis = -0.593762...
      - Wigner semicircle distribution, excess kurtosis = -1
      - uniform distribution, excess kurtosis = -1.2. 
    */
   template <class Points>
   double kurtosis( const Points& p, double* outMean = 0, double* outVar = 0 )
   {
      ensure( p.size(), "empty size!" );

      double mean = 0;
      for ( size_t n = 0; n < p.size(); ++n )
      {
         mean += p[ n ];
      }
      mean /= p.size();
      
      double var = 0;
      double e = 0;
      for ( size_t n = 0; n < p.size(); ++n )
      {
         const double val = core::sqr( p[ n ] - mean );
         var += val;
         e += core::sqr( val );
      }
      var /= p.size();

      if ( outMean )
      {
         *outMean = mean;
      }

      if ( outVar )
      {
         *outVar = var;
      }

      return e / ( core::sqr( var ) * p.size() ) - 3;
   }
}
}

#endif
