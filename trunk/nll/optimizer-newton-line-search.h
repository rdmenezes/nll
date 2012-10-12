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

#ifndef OPTIMIZER_ALGORITHM_OPTIMIZER_NEWTON_LINE_SEARCH_H_
# define OPTIMIZER_ALGORITHM_OPTIMIZER_NEWTON_LINE_SEARCH_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Newton method to search the direction combined with a line search, ensuring to find a local minimum

    The direction is given by (Hf^-1(x)) * grad f(x)

    See <code>OptimizerNewton</code> for more detailed explanation on the search direction.

    @sa OptimizerNewton
    @see http://en.wikipedia.org/wiki/Newton's_method_in_optimization
    */
   class OptimizerNewtonLineSearch : public Optimizer
   {
   public:
      using Optimizer::optimize;
      typedef OptimizerClient::Matrix  Matrix;
      typedef OptimizerClient::Vector  Vector;


   public:
      /**
       @param lambda in ]0..1] to satify the Wolfe condition (http://en.wikipedia.org/wiki/Wolfe_conditions)
       */
      OptimizerNewtonLineSearch( StopCondition& stop, double alpha = 0.1, size_t logEveryXIter = 10 ) : _stop( stop ), _alpha( alpha ), _logEveryXIter( logEveryXIter )
      {}

      virtual core::Buffer1D<double> optimize( const OptimizerClient& client, const ParameterOptimizers& /*parameters*/, const core::Buffer1D<double>& seed )
      {
         {
            std::stringstream ss;
            ss << "OptimizerNewtonLineSearch.optimize:" << std::endl
               << "  seed=" << seed << std::endl
               << "  alpha=" << _alpha;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         _stop.reinit();

         core::Buffer1D<double> params;
         params.clone( seed );

         size_t iter = 0;
         double eval = client.evaluate( params );
         while ( !_stop.stop( eval ) )
         {
            Matrix hessian = client.evaluateHessian( params );
            Vector gradient = client.evaluateGradient( params );

            while (1)
            {
               const bool inverted = core::inverse( hessian );
               if ( inverted )
                  break;

               // make sure the hessian is invertible
               for ( size_t n = 0; n < hessian.size(); ++n )
               {
                  hessian( n, n ) += _alpha;
               }
            }

            if ( iter % _logEveryXIter == 0 )
            {
               std::stringstream ss;
               ss << " iter=" << iter << std::endl
                  << "  point=" << params << std::endl
                  << "  gradient=" << gradient << std::endl
                  << "  Hessian=" << hessian << std::endl
                  << "  eval(point)=" << eval;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

            // compute the next step
            Vector direction = hessian * gradient;
            lineMinimization( params, direction, eval, client );
            ++iter;
         }

         {
            std::stringstream ss;
            ss << " Optimization Done, nbIter=" << iter << std::endl
               << "  point=" << params << std::endl
               << "  eval(point)=" << eval;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         return params;
      }

   protected:
      StopCondition&          _stop;
      double                  _alpha;
      size_t                  _logEveryXIter;
   };
}
}

#endif