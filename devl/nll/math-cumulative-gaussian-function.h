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

#ifndef NLL_CORE_MATH_CUMULATIVE_GAUSSIAN_FUNCTION
# define NLL_CORE_MATH_CUMULATIVE_GAUSSIAN_FUNCTION

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Compute the cumulative error gaussian function and inverse cumulative error gaussian function
    @see http://en.wikipedia.org/wiki/Error_function
    */
   class CumulativeGaussianFunction
   {
   public:
      /**
       @brief use a look up table to compute extremly fast the erf which is quite inaccurate
       */
      static double erf_lut( double x )
      {
         ensure( x >= 0, "can't be negative" );

         static const double table[] = 
         {
            0,
            0.056372,
            0.1124629,
            0.167996,
            0.2227026,
            0.2763264,
            0.3286268,
            0.3793821,
            0.4283924,
            0.4754817,
            0.5204999,
            0.5633234,
            0.6038561,
            0.6420293,
            0.6778012,
            0.7111556,
            0.742101,
            0.7706681,
            0.7969082,
            0.8208908,
            0.8427008,
            0.8802051,
            0.910314,
            0.9340079,
            0.9522851,
            0.9661051,
            0.9763484,
            0.9837905,
            0.9890905,
            0.9927904,
            0.9953223,
            0.9970205,
            0.9981372,
            0.9988568,
            0.9993115,
            0.999593,
            0.999764,
            0.9998657,
            0.999925,
            0.9999589,
            0.9999779,
            0.9999884,
            0.999994,
            0.9999969,
            0.9999985,
            0.9999993,
         };
         static const double interval = 0.05;
         static const ui32 size = 46;

         const ui32 index = core::round( x / interval );
         if ( index >= size )
            return 1;
         return table[ index ];
      }

      /**
       @ingroup core
       @brief implementation of approximate erf, the maximum error is 5*10^-4
       @see http://en.wikipedia.org/wiki/Error_function, "Approximation with elementary functions"
       */
      static double erf_approx1( double x )
      {
         ensure( x >= 0, "can't be negative" );

         static const double a1=0.278393;
         static const double a2=0.230389;
         static const double a3=0.000972;
         static const double a4=0.078108;

         const double x2 = core::sqr( x );
         const double x4 = core::sqr( x2 );

         const double div = core::sqr( 1 + a1 * x + a2 * x2 + a3 * x2 * x + a4 * x4 );
         return 1.0 - 1.0 / core::sqr( div );
      }

      /**
       @ingroup core
       @brief extremly fast implementation of erfinv using look up table, also quite inaccurate. Use only if accuracy
              is not a problem
       */
      static double erfinv_lut( double x )
      {
         ensure( x >= -1 && x <= 1, "wrong range" );

         static const double table[] = 
         {
            -2.3268,
            -1.427709544,
            -1.210337216,
            -1.069420932,
            -0.961479525,
            -0.872212369,
            -0.795027188,
            -0.726307752,
            -0.66384197,
            -0.606170343,
            -0.552274304,
            -0.501411189,
            -0.453019902,
            -0.406663673,
            -0.361993568,
            -0.318724289,
            -0.276617527,
            -0.235470163,
            -0.195105655,
            -0.155367564,
            -0.116114554,
            -0.077216401,
            -0.038550694,
            0,
            0.038550694,
            0.077216401,
            0.116114554,
            0.155367564,
            0.195105655,
            0.235470163,
            0.276617527,
            0.318724289,
            0.361993568,
            0.406663673,
            0.453019902,
            0.501411189,
            0.552274304,
            0.606170343,
            0.66384197,
            0.726307752,
            0.795027188,
            0.872212369,
            0.961479525,
            1.069420932,
            1.210337216,
            1.427709544,
            2.3268
         };
         static const ui32 size = 47;
         static const double interval = 0.043478261;

         if ( x <= -1 )
            return table[ 0 ];
         if ( x >= 1 )
            return table[ size - 1 ];

         const ui32 index = core::round( ( x - ( -1.0 ) ) / interval );
         return table[ index ];
      }
   };
}
}

#endif