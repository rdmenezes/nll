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

#ifndef NLL_ALGORITHM_POWELL_H_
# define NLL_ALGORITHM_POWELL_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Powell optimizer. Particularly suited for optimizing problems with low dimensionality (<=10) and find
           solutions in narrow valley problems

    Point must define operator[], size(), Point(n)
    @param p the starting point
    @param xi the set of directions that can be used
    @param tol the tolerance
    */
   template <class Point, class Functor>
   double powell( Point& p, const std::vector< core::Buffer1D<double> >& xi_orig, double tol, const Functor& f, size_t itMax = 200, bool* error = 0, bool logging = true )
   {
      // set the init
      std::vector< core::Buffer1D<double> > xi( xi_orig.size() );
      for ( size_t n = 0; n < xi_orig.size(); ++n )
      {
         xi[ n ].clone( xi_orig[ n ] );
      }

      // logging info
      if ( logging )
      {
         std::stringstream ss;
         ss << "powell optimization:" << std::endl
            << "start point=";
         for ( size_t n = 0; n < p.size(); ++n )
            ss << p[ n ] << " ";
         ss << std::endl <<
               "tolerance = " << tol << std::endl <<
               "max iter = " << itMax << std::endl <<
               "optimization set directions=" << std::endl;
         for ( size_t n = 0; n < xi.size(); ++n )
         {
            xi[ n ].print( ss );
         }

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
      }

      if ( error )
         *error = false;
      const double tiny = 1e-20;
      double fret;
      const int n = static_cast<int>( p.size() );
      int i, ibig, j;
      double del, fp, fptt, t;
      size_t iter;

      Point pt( n );
      Point ptt( n );
      core::Buffer1D<double> xit( n );

      fret = f( p );
      for ( j = 0; j< n; j++ )
         pt[ j ] = p[ j ];
      for ( iter = 0; ; ++iter )
      {
         fp = fret;
         ibig = 0;
         del = 0;

         if ( logging )
         {
            std::stringstream ss;
            ss << "point=";
            for ( size_t n = 0; n < p.size(); ++n )
            {
               ss << p[ n ] << " ";
            }
            ss << " similarity=" << fp;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         for ( i = 0; i < n; i++ )
         {
            //In each iteration, loop over all directions in the set.
            for ( j = 0; j < n; j++ )
               xit[ j ] = xi[ i ][ j ]; // Copy the direction,
            fptt = fret;
            lineMinimization( p, xit, fret, f, tol );
            if ( fptt - fret > del )
            {
               //and record it if it is the largest decrease so far.
               del = fptt - fret;
               ibig = i;
            }
         }
         if ( 2.0 * ( fp - fret ) <= tol * ( fabs( fp ) + fabs( fret ) + tiny ) )
         {
            if ( logging )
            {
               std::stringstream ss;
               ss << "powell end params=";
               for ( size_t n = 0; n < p.size(); ++n )
               {
                  ss << p[ n ] << " ";
               }
               ss << " Tolerance reached. value=" << fret << " nbIterations=" << iter;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }
            return fret;
         }
         
         if ( iter == itMax )
         {
            if ( error )
            {
               *error = true;
            }

            if ( logging )
            {
               std::stringstream ss;
               ss << "powell end params=";
               for ( size_t n = 0; n < p.size(); ++n )
               {
                  ss << p[ n ] << " ";
               }
               ss << " Max number of iterations reached. value=" << fret;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }
            return fret;
         }

         for ( j = 0; j < n; j++ )
         {
            // Construct the extrapolated point and the average direction moved. Save the old starting point.
            ptt[ j ] = 2.0 * p[ j ] - pt[ j ];
            xit[ j ] = p[ j ] - pt[ j ];
            pt[ j ] = p[ j ];
         }
         fptt = f( ptt ); // Function value at extrapolated point.
         if ( fptt < fp )
         {
            t = 2.0 * ( fp - 2.0 * fret + fptt ) * core::sqr( fp - fret - del ) - del * core::sqr( fp - fptt );
            if ( t < 0.0 )
            {
               lineMinimization( p, xit, fret, f, tol );
               for ( j = 0; j < n; j++ )
               {
                  xi[ ibig ][ j ] = xi[ n - 1 ][ j ];
                  xi[ n - 1 ][ j ] = xit[ j ];
               }
            }
         }
      }
   }
}
}

#endif
