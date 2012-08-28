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

#ifndef NLL_ALGORITHM_GRADIENT_DESCENT_H_
# define NLL_ALGORITHM_GRADIENT_DESCENT_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Optimize the function using gradient descent in stochastic mode
    @deprecated this should be achieved by the Optimizer OptimizerClient interface

    Everytime a sample is presented, the model is updated. Usually more noisy, but conerge
    much faster than the regular gradient descent
    */
   template <class Database>
   class FunctionOptimizationGradientDescentStochastic : public FunctionOptimizer<Database>
   {
   public:
      FunctionOptimizationGradientDescentStochastic( double learningRate ) : _learningRate( learningRate )
      {}

      virtual core::Buffer1D<double> compute( const Database& datLearning,
                                              const std::vector<double>& weights,
                                              const ObjectiveFunction<Database>& function,
                                              const core::Buffer1D<double>& initialParameters,
                                              size_t nbIterations ) const
      {
         #ifdef NLL_SECURE
         for ( size_t n = 0; n < datLearning.size(); ++n )
         {
            ensure( datLearning[ n ].type == Database::Sample::LEARNING, "only learning database allowed!" );
         }
         #endif

         ensure( weights.size() == datLearning.size(), "size mismatch" );

         core::Buffer1D<double> model;
         model.clone( initialParameters );
         for ( size_t n = 0; n < nbIterations; ++n )
         {
            for ( size_t s = 0; s < datLearning.size(); ++s )
            {
               core::Buffer1D<double> gradient = function.computeGradient( model, datLearning[ s ], weights[ s ] );
               for ( size_t m = 0; m < gradient.size(); ++m )
               {
                  model[ m ] -= _learningRate * gradient[ m ] / datLearning.size();
               }
            }
         }

         return model;
      }

   private:
      double   _learningRate;
   };
}
}

#endif