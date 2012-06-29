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

#ifndef NLL_CORE_BUFFER1D_CONVOLVE_H_
# define NLL_CORE_BUFFER1D_CONVOLVE_H_

namespace nll
{
namespace core
{
   /**
    @brief Compute a 1D convolution

    Note that the behaviour on the data sides (i.e., +/- convolution.size() / 2) is to do an average of the
    kernel that fits inside the data.

    Complexity in time is o(NM) in processing and o(M+N) in size, with N = data size and M = convolution kernel size

    We are assuming the data has at least the size of the kernel.
   */
   template <class T>
   core::Buffer1D<T> convolve( const core::Buffer1D<T>& data, const core::Buffer1D<T>& convolution )
   {
      #ifdef NLL_SECURE
      {
         const T accum = std::accumulate( convolution.begin(), convolution.end(), static_cast<T>( 0 ) );
         ensure( core::equal<T>( accum, 1, 1e-5 ), "the sum of convolution coef must sum to 1" );
      }
      #endif

      ensure( convolution.size() % 2 == 1, "convolution size must be odd" );
      ensure( convolution.size() <= data.size(), "data size must be > kernel size!" ); // if this is not true, data will not be useful anyway

      core::Buffer1D<T> convolved( data.size(), false );

      // compute the "regular" sequence, i.e. in domain [kernelSize/2..dataSize-kernelSize/2]
      const size_t halfKernelSize = convolution.size() / 2;
      const size_t lastRegularIndex = data.size() - halfKernelSize;
      for ( size_t n = halfKernelSize; n < lastRegularIndex; ++n )
      {
         T accum = 0;
         for ( size_t nn = 0; nn < convolution.size(); ++nn )
         {
            accum += data[ n - halfKernelSize + nn ] * convolution[ nn ];
         }

         convolved[ n ] = accum;
      }

      // compute the normalization factor for part of the kernel fitting in.
      core::Buffer1D<double> maxRegularizationLeft( halfKernelSize, false );
      core::Buffer1D<double> maxRegularizationRight( halfKernelSize, false );

      // assuming half the kernel always fit in the data
      {
         // accum holds the total kernel weight that fits in
         T accum = std::accumulate( convolution.begin() + halfKernelSize + 1, convolution.end(), (T)0.0 ); // we have at least half the filter
         for ( size_t n = 0; n < halfKernelSize; ++n )
         {
            accum += convolution[ halfKernelSize - n ]; 
            maxRegularizationLeft[ n ] = 1.0 / accum;
         }
      }

      {
         // accum holds the total kernel weight that fits in
         T accum = std::accumulate( convolution.begin(), convolution.begin() + halfKernelSize, (T)0.0 ); // we have at least half the filter
         for ( size_t n = 0; n < halfKernelSize; ++n )
         {
            accum += convolution[ halfKernelSize + n ]; 
            maxRegularizationRight[ n ] = 1.0 / accum;
         }
      }

      // now take care of the sides, they will be more noisy as we are only using part of the kernel...
      for ( size_t n = 0; n < halfKernelSize; ++n )
      {
         // left side
         {
            T accum = 0;
            for ( size_t nn = 0; nn <= halfKernelSize + n; ++nn )
            {
               accum += convolution[ nn + halfKernelSize - n ] * data[ nn ];
            }
            convolved[ n ] = accum * maxRegularizationLeft[ n ]; // here we normalize by the weighted kernel fitting in
         }

         // right side
         {
            T accum = 0;
            for ( size_t nn = 0; nn <= halfKernelSize + n; ++nn )
            {
               accum += convolution[ n + halfKernelSize - nn ] * data[ data.size() - nn - 1 ];
            }
            convolved[ data.size() - n - 1 ] = accum * maxRegularizationRight[ n ]; // here we normalize by the weighted kernel fitting in
         }
      }
      return convolved;
   }
}
}

#endif
