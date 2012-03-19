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

#ifndef NLL_ALGORITHM_FUNCTION_H_
# define NLL_ALGORITHM_FUNCTION_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Function for optimization problem

    Lets define a function f(x; theta), a function using x and parameterized by theta
    
    In order to apply gradient based optimization techniques, we need to define
    - a cost function J(theta), wich evaluate theta on some training data
    - a gradient function, which compute the gradient on some training data
    */
   template <class Database>
   class ObjectiveFunction : public core::NonCopyable
   {
   public:
      typedef typename Database::Sample::Input::value_type  value_type;
      typedef typename Database::Sample::Input              Point;
      typedef typename Database::Sample::Output             Output;
      typedef typename Database::Sample                     Sample;

      ObjectiveFunction()
      {}

      virtual ~ObjectiveFunction()
      {}

      /**
       @brief compute the cost function (i.e. for all the samples)
       */
      virtual double computeCostFunction( const core::Buffer1D<double>& theta, const Database& dat, const std::vector<double>& weights ) const = 0;

      /**
       @brief Given our model <theta>, Compute the gradient of a specific sample
       */
      virtual core::Buffer1D<double> computeGradient( const core::Buffer1D<double>& theta, const Sample& sample, double weight ) const = 0;
   };

   /**
    @brief Optimization algorithm for <Function>
    */
   template <class Database>
   class FunctionOptimizer
   {
   public:
      typedef typename Database::Sample::Input  Point;

      /**
       @brief Optimize the <function>
       @param learningDatabase the learning database only!
       @param initialParameters the initial starting point
       @param nbIterations the number of iterations
       */
      virtual core::Buffer1D<double> compute( const Database& learningDatabase,
                                              const std::vector<double>& weights,
                                              const ObjectiveFunction<Database>& function,
                                              const core::Buffer1D<double>& initialParameters,
                                              ui32 nbIterations ) const = 0;

      virtual ~FunctionOptimizer()
      {}
   };
}
}

#endif