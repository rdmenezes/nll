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

#ifndef NLL_ALGORITHM_BRENT_H_
# define NLL_ALGORITHM_BRENT_H_

# include "bracketing.h"

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Minimization of a line
    @param ax bracketing such that ax < ay < az, f(ax) >= ay && f(az) >= ay
    @param bx bracketing such that ax < ay < az, f(ax) >= ay && f(az) >= ay
    @param cx bracketing such that ax < ay < az, f(ax) >= ay && f(az) >= ay
    @param f the function to minimize
    @param outMinX the position x of the minimum
    @param tol the tolerance
    @param itmax the maximum number of iteration allowed
    @param error return true if brent did not finished correctly

    This code is extracted from numerical recipes
    */
   template <class Functor>
   double brent( double ax, double bx, double cx, const Functor& f, double& outMinX, double tol, ui32 itMax = 100, bool* error = 0 )
   {
      if ( error )
         *error = false;
      const double gold = 0.3819660;
      const double eps = 1e-10;

      ui32 iter;
      double a, b, d = 0, etemp, fu, fv, fw, fx, p, q, r, tol1, tol2, u, v, w, x, xm;
      double e = 0.0;         // This will be the distance moved on the step before last.
      a = (ax < cx ? ax : cx);  // a and b must be in ascending order,
      b = (ax > cx ? ax : cx);  // but input abscissas need not be.
      x = w = v = bx;
      fw = fv = fx = f( x );
      for ( iter = 1; iter <= itMax; iter++ )
      {
         xm = 0.5 * ( a + b );
         tol2 = 2.0 * ( tol1 = tol * fabs( x ) + eps );
         if ( fabs( x - xm ) <= ( tol2 - 0.5 * ( b - a ) ) )
         {
            outMinX = x;
            return fx;
         }
         if ( fabs( e ) > tol1 )
         {
            r=( x - w ) * ( fx -fv );
            q=( x - v ) * ( fx -fw );
            p=( x - v ) * q -( x - w ) * r;
            q= 2.0 * ( q -r );
            if ( q > 0.0 )
               p = -p;
            q = fabs( q );
            etemp = e;
            e = d;
            if ( fabs( p ) >= fabs( 0.5 * q * etemp ) || p <= q * ( a - x ) || p >= q * ( b - x ) )
               d = gold * (e = (x >= xm ? a - x : b - x ) );
            // The above conditions determine the acceptability of the parabolic fit. Here we
            // take the golden section step into the larger of the two segments.
            else
            {
               d = p / q;
               u = x + d;
               if ( u - a < tol2 || b - u < tol2 )
                  d = impl::signMod( tol1, xm - x );
            }
         } else {
            d = gold * ( e = ( x >= xm ? a - x : b - x ) );
         }
         u = ( fabs( d ) >= tol1 ? x + d : x + impl::signMod( tol1, d ) );
         fu = f( u );
         // This is the one function evaluation per iteration.
         if ( fu <= fx )
         { //Now decide what to do with our funcif
            if ( u >= x )
               a = x;
            else
               b = x;
            impl::shift( v, w, x, u );
            impl::shift( fv, fw, fx, fu );
         } else {
            if ( u < x )
               a = u;
            else
               b = u;
            if ( fu <= fw || w == x )
            {
               v = w;
               w = u;
               fv = fw;
               fw = fu;
            } else if ( fu <= fv || v == x || v == w )
            {
               v = u;
               fv = fu;
            }
         }
      }
      core::LoggerNll::write( core::LoggerNll::WARNING, "Too many iterations in brent" );
      if ( error )
         *error = true;
      outMinX = x;
      return fx;
   }

   namespace impl
   {
      /**
       @ingroup algorithm
       @brief Helper to bridge with the line minimization with brent algorithm
       */
      template <class Point, class Functor>
      class HelperLineMinimizationFunctionWrapper
      {
      public:
         HelperLineMinimizationFunctionWrapper( const Point& i, const core::Buffer1D<double>& dir, const Functor& f ) :
            _i( i ), _dir( dir ), _f( f )
         {
         }

         double operator()( double n ) const
         {
            Point p( _i.size() );
            for ( ui32 nn = 0; nn < _i.size(); ++nn )
            {
               p[ nn ] = n * _dir[ nn ] + _i[ nn ];
            }
            return _f( p );
         }

      private:
         HelperLineMinimizationFunctionWrapper( const HelperLineMinimizationFunctionWrapper& );
         HelperLineMinimizationFunctionWrapper& operator=( const HelperLineMinimizationFunctionWrapper& );

      private:
         const Point& _i;
         const core::Buffer1D<double>& _dir;
         const Functor& _f;
      };
   }

   /**
    @ingroup algorithm
    @brief Line minimization for multidimentional data
    @param p the entry point. It will be updated with the position of the minimum found
    @param direction the direction of the line search
    @param ret the minimum found
    @param f the function to minimize

    @return true if error occured

    Point must define operator[], size(), Point(n)
    */
   template <class Point, class Functor>
   bool lineMinimization( Point& p, core::Buffer1D<double>& direction, double& ret, const Functor& f, double tol = 2.0e-4 )
   {
      bool error = false;
      impl::HelperLineMinimizationFunctionWrapper<Point, Functor> ff( p, direction, f );
      BracketingResult bracket = minimumBracketing( 1, 0, ff );
      double xmin;
      ret = brent( bracket.ax, bracket.bx, bracket.cx, ff, xmin, tol, 100, &error );
      for ( ui32 n = 0; n < p.size(); ++n )
      {
         p[ n ] += direction[ n ] * xmin;
         direction[ n ] *= xmin;
      }
      return error;
   }
}
}

#endif
