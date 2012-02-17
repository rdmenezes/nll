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

#ifndef NLL_OPTIMIZER_CLIENT_H_
# define NLL_OPTIMIZER_CLIENT_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Base class for a client to be optimized. The optimizer has to find the best parameters minimizing the cost
           funtion.
           
           A key point for optimization algorithms is how quickly, a solution can be evaluated. It is too slow,
           the number of solutions tested will be too slow, and the underlying algorithm may not converge to an
           acceptable solution.
    */
   class NLL_API OptimizerClient
   {
   public:
      /**
       @brief return the score of the optimization for a specific set of parameters.

       The lowest is the score, the better.
       @sa Classifier
       */
      virtual double evaluate( const nll::core::Buffer1D<nll::f64>& parameters ) const = 0;

      /**
       @todo deprecate...
       */
      double operator()( const nll::core::Buffer1D<nll::f64>& parameters ) const { return evaluate( parameters ); }

      virtual ~OptimizerClient()
      {}
   };
}
}

#endif
