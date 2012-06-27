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

#ifndef NLL_BRACKETING_H_
# define NLL_BRACKETING_H_

namespace nll
{
   namespace algorithm
   {
      namespace impl
      {
         inline void shift( double& a, double& b, double& c, double d )
         {
            a = b;
            b = c;
            c = d;
         }

         inline double signMod( double a, double b )
         {
            return b >= 0 ? fabs( a ) : - fabs( a );
         }
      }

      /**
       @ingroup algorithm
       @brief Result of a bracketing
       */
      struct BracketingResult
      {
         double ax;
         double bx;
         double cx;

         double fa;
         double fb;
         double fc;
      };

      /**
       @ingroup algorithm
       @brief Find 3 values (a, b, c) such that a < b < c || c < b < a and f(a) >= f( b ) && f( c ) >= f( b )

       It means f has a minimum in the interval (a, c)
       */
      template <class Functor>
      BracketingResult minimumBracketing( double ax, double bx, const Functor& f )
      {
         const double glimit = 100;
         const double gold = 1.618034;
         BracketingResult res;
         double fa, fb, fc, cx, ulim, fu;

         // downhill from a to b
         fa = f( ax );
         fb = f( bx );
         if ( fb > fa )
         {
            std::swap( ax, bx );
            std::swap( fa, fb );
         }

         // first guess for c
         cx = bx + gold * ( bx - ax );
         fc = f( cx );

         while ( fb > fc )
         {
            double r = ( bx - ax ) * ( fb - fc );
            double q = ( bx - cx ) * ( fb - fa );
            double u = bx - ( ( bx - cx ) * q - ( bx - ax ) * r ) / 
               ( 2 * impl::signMod( std::max( fabs( q - r ), std::numeric_limits<double>::epsilon() ), q - r  ) );
            ulim = bx + glimit * ( cx - bx );

            if ( ( bx - u ) * ( u - cx ) > 0 )
            {
               fu = f( u );
               if ( fu < fc )
               {
                  // got a minimum between b and c
                  res.ax = bx;
                  res.bx = u;
                  res.fa = fb;
                  res.fb = fu;
                  res.cx = cx;
                  res.fc = fc;
                  return res;
               } else if ( fu > fb )
               {
                  // got a minimum between a and u
                  res.ax = ax;
                  res.bx = bx;
                  res.fa = fa;
                  res.fb = fb;
                  res.cx = u;
                  res.fc = fu;
                  return res;
               }
               u = cx + gold * ( cx - bx);
               fu = f( u );
            } else if ( ( cx - u ) * ( u - ulim ) > 0 )
            {
               fu = f( u );
               if ( fu < fc )
               {
                  impl::shift( bx, cx, u, cx + gold * ( cx - bx ) );
                  impl::shift( fb, fc, fu, f( u ) );
               }
            } else if ( ( u - ulim ) * ( ulim - cx) > 0 )
            {
               u = ulim;
               fu = f ( u );
            } else {
               u = cx + gold * ( cx - bx );
               fu = f( u );
            }
            impl::shift( ax, bx, cx, u );
            impl::shift( fa, fb, fc, fu );
         }

         res.ax = ax;
         res.bx = bx;
         res.cx = cx;
         res.fa = fa;
         res.fb = fb;
         res.fc = fc;
         return res;
      }
   }
}

#endif
