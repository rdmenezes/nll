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

#ifndef NLL_OPTIMIZER_GRADIENT_DESCENT_LINESEARCH_H_
# define NLL_OPTIMIZER_GRADIENT_DESCENT_LINESEARCH_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Simple Gradient descent algorithm with a line search (variable step size)
    
    Efficient for problems with high dimentionality but very sensitive to local minimum. It must be noted that
    the algorithm is particularly sensitive to functions with high curvature (e.g., maybe because of badly scaled parameters).
    
    In the case of such pathological functions to optimize, a second order method that will use the curvature information to
    rescale the steps should be used.

    Compare to the normal gradient descent, only the gradient is used for the seach direction which is then combined to a line
    search. As a consequence, it is easier for the user (do not specify the step), may be more efficient (i.e., dynamic step size) but may
    also have a higher number of function evaluations.
    */
   class OptimizerGradientDescentLineSearch : public Optimizer
   {
   public:
      using Optimizer::optimize;

      /**
       @param stopCondition the condition when the algorithm must stop
       */
      OptimizerGradientDescentLineSearch( StopCondition& stopCondition, size_t logEveryXIter = 10 ) : _stopCondition( stopCondition ), _lastError( std::numeric_limits<double>::max() ), _logEveryXIter( logEveryXIter )
      {
      }

      /**
       @note throw an excpetion if optimization failed.
       */
      virtual core::Buffer1D<double> optimize( const OptimizerClient& client, const ParameterOptimizers& /* parameters */, const core::Buffer1D<double>& seed )
      {
         {
            std::stringstream ss;
            ss << "OptimizerGradientDescentLineSearch.optimize:" << std::endl
               << "  seed=" << seed;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         core::Buffer1D<double> point;
         point.clone( seed );

         // run the main loop
         _stopCondition.reinit();
         double bestSolutionEval = std::numeric_limits<double>::max();
         core::Buffer1D<double> bestSolution;
         size_t iteration = 0;
         do
         {
            core::Buffer1D<double> gradient = client.evaluateGradient( point );
            
            // normalize the gradient
            ensure( gradient.size() == point.size(), "size of gradient and function must be the same" );
            const double norm = core::norm2( gradient );
            if ( !core::equal( norm, 0.0 ) )
            {
               gradient /= norm;

               // find the next step
               const bool lineMinimizationError = lineMinimization( point, gradient, _lastError, client );
               ensure( !lineMinimizationError, "the function could not be bracketed" );
            }

            if ( _lastError < bestSolutionEval )
            {
               // store the best solution
               bestSolutionEval = _lastError;
               bestSolution.clone( point );
            }

            if ( ( iteration % _logEveryXIter ) == 0 )
            {
               std::stringstream ss;
               ss << "iteration=" << iteration << std::endl
                  << "  point=" << point << std::endl
                  << "  eval(point) =" << _lastError << std::endl
                  << "  gradient(point)=" << gradient;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

            ++iteration;
         } while ( !_stopCondition.stop( _lastError ) );

         {
            std::stringstream ss;
            ss << "OptimizerGradientDescentLineSearch.optimize result:" << std::endl
               << "  result=" << point << std::endl
               << "  eval(result)=" << bestSolutionEval;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         _lastError = bestSolutionEval;
         return bestSolution;
      }

      double getLastError() const
      {
         return _lastError;
      }

   protected:
      StopCondition&          _stopCondition;
      double                  _lastError;
      size_t                  _logEveryXIter;
   };
}
}

#endif