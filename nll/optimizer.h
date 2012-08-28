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

#ifndef NLL_OPTIMIZER_H_
# define NLL_OPTIMIZER_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief interface for a generic optimizer implementation
    */
   class NLL_API Optimizer : public core::NonCopyable
   {
   public:
      /**
       @brief Optimize a function by finding the minimum. How well the problem is solved is expressed by a function fitness
              provided by the <code>client</code> We are trying to find the best parameters minimizing this cost function.
       @param client the client to be optimized. It is used to give a score of a particular solution
       @param parameters defines the properties of the parameters. Properties like min, max, distribution, search operators...
              These properties are <b>very</b> important. If knowledge of the problem can be added, it would narrow a lot
              the search space, making the optimizer successful.
       @param seed the starting position
       */
      virtual core::Buffer1D<double> optimize( const OptimizerClient& client,
                                               const ParameterOptimizers& parameters,
                                               const core::Buffer1D<double>& seed ) = 0;

      /**
       @brief Optimize the client function
       @note the seed is randomly generated from the parameters
       */
      virtual core::Buffer1D<double> optimize( const OptimizerClient& client,
                                               const ParameterOptimizers& parameters )
      {
         core::Buffer1D<double> seed( parameters.size() );
         for ( size_t n = 0; n < seed.size(); ++n )
         {
            seed[ n ] = parameters[ n ].generate();
         }

         return optimize( client, parameters, seed );
      }

      virtual ~Optimizer()
      {}
   };
}
}

#endif
