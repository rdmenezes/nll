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

#ifndef NLL_ALGORITHM_PERIODOGRAM_H_
# define NLL_ALGORITHM_PERIODOGRAM_H_

namespace nll
{
namespace algorithm
{
   /**
      @brief Defines a square window, which keep the data intact within interval [0..1], set it to 0 otherwise
      */
   class SquareWindow
   {
   public:
      typedef double value_type;

      value_type operator()( value_type v ) const
      {
         #ifdef NLL_SECURE
         ensure( v >= 0 && v <= 1.0, "wrong domain!" );
         #endif

         return ( v >= 0 && v <= 1 ) ? v : 0;
      }
   };

   /**
      @brief Hanning window

      Useful for DFT/FFT purposes, such as in spectral analysis. The DFT/FFT contains an implicit periodic asumption. 
      The non periodic signal will become periodic when applied the window. 
      */
   class HanningWindow
   {
   public:
      typedef double value_type;

      value_type operator()( value_type v ) const
      {
         #ifdef NLL_SECURE
         ensure( v >= 0 && v <= 1.0, "wrong domain!" );
         #endif

         return 0.5 * ( 1.0 - std::cos( 2 * core::PI * v ) );
      }
   };

   /**
      @brief Create the periodogram of a 1D time serie
      @see http://www.stat.tamu.edu/~jnewton/stat626/topics/topics/topic4.pdf
         http://www.ltrr.arizona.edu/~dmeko/notes_6.pdf

      f(w) = 1/N * |sum_t=1-N(x(t)*exp(2*pi*i*(t-1)*w))|^2, w in [0..0.5]
      f(w) = f( 1 - w )

      Raw spectrum of a time serie suffers from the spectral bias and (the variance at a given frequency does not decrease as
      the number of samples used in the computation increases) and variance problems.

      The spectral bias problem arises from a sharp truncation of the sequence, and can be reduced by first multiplying the
      finite sequence by a window function which truncates the sequence gradually rather than abruptly.

      The variance problem is reduced but smoothing the periodogram

      Smoothness, stability and resolution of the data must be considered when choosing the smoothing kernel and
      window function. This is problem dependent.
      */
   class Periodogram
   {
   public:
      typedef double                                  value_type;

      /**
         @brief Compute the raw and smoothed periodogram
         @param timeSerie the time serie we are analysing
         @param smoothingKernel the kernel used to smooth the periodogram. Can be empty if no smoothing required
         @param funcWindow the window to use to reduce the sharp truncation effect
         @return the periodogram

         Example: if it returns [0 0 0 1 0] -> it means there is a spike of 1 at index = 3. This means the period in hertz is 3 / ( 2 * 4 ) = 0.375 hz, ie. the pattern repeats every 2.66 samples
         */
      template <class Function1D>
      core::Buffer1D<value_type> compute( const core::Buffer1D<value_type>& timeSerie, const core::Buffer1D<value_type>& smoothingKernel, Function1D& funcWindow )
      {
         // get the mean
         const value_type mean = std::accumulate( timeSerie.begin(), timeSerie.end(), (value_type)0.0 ) / timeSerie.size();

         // multiply by a window to reduce the spectral bias (see http://en.wikipedia.org/wiki/Periodogram)
         core::Buffer1D<value_type> data( timeSerie.size(), false );
         for ( ui32 n = 0; n < data.size(); ++n )
         {
            data[ n ] = ( timeSerie[ n ] - mean ) * funcWindow( static_cast<value_type>( n ) / timeSerie.size() );
         }

         // get the DFT coefficients
         Fft1D fft;
         core::Buffer1D<value_type> fftOutput;
         fft.forward( data, data.size(), fftOutput );
         ensure( fftOutput.size() % 2 == 0, "must be pair! we have real and imaginary parts" );

         // compute the raw periodogram from the DFT coefficients
         core::Buffer1D<value_type> basicPeriodogram( fftOutput.size() / 2 );
         for ( ui32 n = 0; n < basicPeriodogram.size(); ++n )
         {
            ui32 index = n * 2;
            basicPeriodogram[ n ] = ( core::sqr( fftOutput[ index ] ) + core::sqr( fftOutput[ index + 1 ] ) ) / timeSerie.size();
         }

         // smooth the DFT coefficients, we are done!
         if ( smoothingKernel.size() )
         {
            return core::convolve( basicPeriodogram, smoothingKernel );
         } else {
            return basicPeriodogram;
         }
      }
   };
}
}

#endif
