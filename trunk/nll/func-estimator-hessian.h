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

#ifndef NLL_ALGORITHM_FUNC_APPROXIMATOR_HESSIAN_H_
# define NLL_ALGORITHM_FUNC_APPROXIMATOR_HESSIAN_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Compute the Hessian using the forward finite difference
    @see A new method to compute second derivatives, Hugo D. Scolnik, 2001
         http://journal.info.unlp.edu.ar/journal/journal6/papers/ipaper.pdf
    */
   class HessianCalculatorForwardFiniteDifference : public core::NonAssignable
   {
   public:
      typedef OptimizerClient::Vector     Vector;
      typedef OptimizerClient::Matrix     Matrix;
      typedef OptimizerClient::value_type value_type;

   public:
      /**
       @param step the step used to compute the finite difference. It must be sufficiently low for better accuracy, but suffiently high to avoid undeflow
              problems. This is problem dependent.
       */
      HessianCalculatorForwardFiniteDifference( const OptimizerClient& f, value_type step = 1e-4 ) : _f( f ), _step( step )
      {}

      Matrix evaluate( const Vector& parameters ) const
      {
         const value_type f0 = _f.evaluate( parameters );
         const value_type stepTimes2 = 2 * _step;
         const value_type step2 = _step * _step;

         Matrix hessian( parameters.size(), parameters.size() );


         // precompute compute f( x + step * e_i ) as they are shared between several Hij
         Vector tmpParameters;
         tmpParameters.clone( parameters );
         std::vector<value_type> evals( parameters.size() );
         for ( size_t n = 0; n < evals.size(); ++n )
         {
            tmpParameters[ n ] += _step;
            evals[ n ] = _f.evaluate( tmpParameters );
            tmpParameters[ n ] -= _step;
         }

         // now compute the upper half matrix and copy to the lower half : the Hessian must be symmetric
         for ( size_t y = 0; y < parameters.size(); ++y )
         {
            for ( size_t x = y + 1; x < parameters.size(); ++x )
            {
               tmpParameters[ y ] += _step;
               tmpParameters[ x ] += _step;
               const value_type fxy = _f.evaluate( tmpParameters );
               tmpParameters[ y ] -= _step;
               tmpParameters[ x ] -= _step;
               
               const value_type h = ( fxy - evals[ x ] - evals[ y ] + f0 ) / step2;
               hessian( y, x ) = h;
               hessian( x, y ) = h;
            }
         }

         // finally compute the diagonal
         for ( size_t n = 0; n < parameters.size(); ++n )
         {
            tmpParameters[ n ] += stepTimes2;
            const value_type f2n = _f.evaluate( tmpParameters );
            tmpParameters[ n ] -= stepTimes2;

            hessian( n, n ) = ( f2n - 2 * evals[ n ] + f0 ) / step2;
         }

         return hessian;
      }

   private:
      const OptimizerClient&  _f;
      value_type              _step;
   };
}
}

#endif
