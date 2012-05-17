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

#ifndef NLL_ALGORITHM_FFT1D_H_
# define NLL_ALGORITHM_FFT1D_H_

namespace nll
{
namespace algorithm
{
   class NLL_API Fft1D
   {
   public:
      /**
       @brief Forward FFT for 1D real data. Returns the discrete Fourier transform (DFT) of vector x, computed with a fast Fourier transform (FFT) algorithm
       @param real the input buffer of size N
       @param nfft the logical length, it must be nfft > real.size(), power of two are fasters, data will be padded with extra zero
       @param outReal the output, will be allocated by the method
       */
      void forward( const core::Buffer1D<double>& real, ui32 nfft, core::Buffer1D<double>& outComplex );

      /**
       @brief backward FFT for 1D real data. Returns the inverse discrete Fourier transform (DFT) of vector x, computed with a fast Fourier transform (FFT) algorithm
       @param real the input buffer of size N
       @param nfft the logical length, it must be  nfft > real.size(), power of two are fasters, data will be padded with extra zero
       @param outReal the output, will be allocated by the method
       */
      void backward( const core::Buffer1D<double>& complex, ui32 nfft, core::Buffer1D<double>& outReal );
   };
}
}

#endif