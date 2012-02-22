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

#ifndef NLL_MATH_SAMPLING_H_
# define NLL_MATH_SAMPLING_H_

# include <algorithm>

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Sampling of a set of points according to a probability for being sampled.
    @param p the probabilities to be used. It <b>must</b> sums to 1
    @param nbSampledPoints the number of points to generate.
    @return the index of the selected points

    Complexity is n1 * log n2 (n2 the number of points to be selected, n1 in the probabilities)
    <code>Probabilities</code> types needs to define:
    - <code>floatingType operator[]( ui32 n ) const</code>
    - <code>ui32 size() const</code>
    */
   template <class Probabilities>
   Buffer1D<ui32> sampling( const Probabilities& p, ui32 nbSampledPoints )
   {
      if ( !nbSampledPoints )
         return Buffer1D<ui32>();
      std::vector<double> sd( p.size() + 1 );
      double prob = 0;
      for ( ui32 n = 0; n < p.size(); ++n )
      {
         prob += p[ n ];
         sd[ n + 1 ] = prob;
      }
      sd[ p.size() ] = 10; // we set an impossible "probability" so we are sure we won't miss the last one
      ensure( fabs( prob - 1 ) <= 0.01, "probability must sum to 1" );
      
      Buffer1D<ui32> points( nbSampledPoints );
      for ( ui32 n = 0; n < nbSampledPoints; ++n )
      {
         double point = static_cast<double>( rand() ) / RAND_MAX;
         size_t nn = 0;
         // we don't need to test the end of the buffer as it is not possible to reach it!
         for ( ; point >= sd[ nn + 1 ]; ++nn )
            ;
         points[ n ] = static_cast<ui32>( nn );
         assert( points[ n ] < p.size() );
      }
      return points;
   }
}
}

#endif
