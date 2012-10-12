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

#ifndef NLL_ALGORITHM_FUNC_ESTIMATOR_H_
# define NLL_ALGORITHM_FUNC_ESTIMATOR_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Simple wrapper for computing the gradient and Hessian of a function where the analytical forms are complex

    Gradient and Hessian are computed unsing forward finite difference method.

    @see <code>HessianCalculatorForwardFiniteDifference</code>, <code>GradientCalculatorFiniteDifference</code>
    */
   class OptimizerClientWrapperFiniteDifference : public OptimizerClient
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
      OptimizerClientWrapperFiniteDifference( const OptimizerClient& func, value_type step = 0.001 ) : _func( func ), _gradientCalculator( func, step ), _hessianCalculator( func, step )
      {}

      virtual double evaluate( const Vector& parameters ) const
      {
         return _func.evaluate( parameters );
      }

      virtual Vector evaluateGradient( const Vector& parameters ) const
      {
         return _gradientCalculator.evaluate( parameters );
      }

      virtual Matrix evaluateHessian( const Vector& parameters ) const
      {
         return _hessianCalculator.evaluate( parameters );
      }

   private:
      const OptimizerClient&                    _func;
      GradientCalculatorFiniteDifference        _gradientCalculator;
      HessianCalculatorForwardFiniteDifference  _hessianCalculator;
   };
}
}

#endif