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

#ifndef NLL_OPTIMIZER_GRADIENT_DESCENT_H_
# define NLL_OPTIMIZER_GRADIENT_DESCENT_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Simple Gradient descent algorithm with constant step size
    
    Efficient for problems with high dimentionality but very sensitive to local minimum. It must be noted that
    the algorithm is particularly sensitive to functions with high curvature (e.g., maybe because of badly scaled parameters).
    
    In the case of such pathological functions to optimize, a second order method that will use the curvature information to
    rescale the steps should be used.

    The algorithm let you define the maximum step for each parameter as well as an independent learningRate. 
    */
   class OptimizerGradientDescent : public Optimizer
   {
   public:
      using Optimizer::optimize;

      /**
       @brief The algorithm let you define the maximum step for each parameter as well as an independent learningRate
       @param stopCondition the condition when the algorithm must stop
       @param learningRate the learning rate to be used if no individual rate is specified. If negative, the gradient will be normalized and learning rate applied
       @param maxDisplacement the maximum displacement for one parameter
       @param individualLearningRate override <learningRate> for each of the parameters
       */
      OptimizerGradientDescent( StopCondition& stopCondition, double learningRate = 0.1, bool turnOnLogging = true, size_t loggingEveryXCycle = 1, const core::Buffer1D<double> maxDisplacement = core::Buffer1D<double>(), const core::Buffer1D<double> individualLearningRate = core::Buffer1D<double>() ) : _stopCondition( stopCondition ), _learningRate( learningRate ), _lastError( std::numeric_limits<double>::max() ), _turnOnLogging( turnOnLogging ), _loggingEveryXCycle( loggingEveryXCycle )
      {
         _maxDisplacement.clone( maxDisplacement );
         _individualLearningRate.clone( individualLearningRate );

         ensure( _individualLearningRate.size() == 0 || _individualLearningRate.size() == _maxDisplacement.size(), "must have the same size!" );
      }

      /**
       @brief returns an empty set if optimization failed.
       */
      virtual core::Buffer1D<double> optimize( const OptimizerClient& client, const ParameterOptimizers& /* parameters */, const core::Buffer1D<double>& seed )
      {
         {
            std::stringstream ss;
            ss << "OptimizerGradientDescent.optimize:" << std::endl
               << "  seed=" << seed << std::endl
               << "  learning rate=" << _learningRate;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // set the learning rates
         core::Buffer1D<double> weights( seed.size(), false );
         if ( _individualLearningRate.size() == 0 )
         {
            for ( size_t n = 0; n < seed.size(); ++n )
            {
               weights[ n ] = fabs( _learningRate );
            }
         } else {
            weights = _individualLearningRate;
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
            const double f = client.evaluate( point );
            _lastError = f;

            if ( f < bestSolutionEval )
            {
               // store the best solution
               bestSolutionEval = f;
               bestSolution.clone( point );
            }

            core::Buffer1D<double> gradient = client.evaluateGradient( point );
            if ( _learningRate < 0 )
            {
               const double norm = core::norm2( gradient );
               for ( size_t n = 0; n < gradient.size(); ++n )
               {
                  gradient[ n ] /= norm;
               }
            }

            if ( _turnOnLogging && ( iteration % _loggingEveryXCycle ) == 0 )
            {
               std::stringstream ss;
               ss << "iteration=" << iteration << std::endl
                  << "  point=" << point << std::endl
                  << "  eval(point) =" << f << std::endl
                  << "  gradient(point)=" << gradient << std::endl
                  << "  learningRate=" << weights;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

            ensure( gradient.size() == point.size(), "size of gradient and function must be the same" );
            ensure( _maxDisplacement.size() == 0 || _maxDisplacement.size() == point.size(), "max displacement = 1 per parameter or empty" );
            if ( _maxDisplacement.size() )
            {
               for ( size_t n = 0; n < point.size(); ++n )
               {
                  double displacement = gradient[ n ] * weights[ n ];
                  displacement = NLL_BOUND( displacement, -_maxDisplacement[ n ], _maxDisplacement[ n ] );
                  point[ n ] -= displacement;
               }
            } else {
               for ( size_t n = 0; n < point.size(); ++n )
               {
                  point[ n ] -= gradient[ n ] * weights[ n ];
               }
            }

            ++iteration;
         } while ( !_stopCondition.stop( _lastError ) );

         {
            std::stringstream ss;
            ss << "OptimizerGradientDescent.optimize result:" << std::endl
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
      double                  _learningRate;
      double                  _lastError;
      bool                    _turnOnLogging;
      size_t                  _loggingEveryXCycle;
      core::Buffer1D<double>  _maxDisplacement;
      core::Buffer1D<double>  _individualLearningRate;
   };
}
}

#endif