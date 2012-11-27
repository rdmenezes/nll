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

#ifndef NLL_ALGORITHM_REGISTRATION_GRADIENT_EVALUATOR_H_
# define NLL_ALGORITHM_REGISTRATION_GRADIENT_EVALUATOR_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Utility class that computes the gradient of a similarity measure according to the transformation

    Since a lot of parameters are influencing the gradient (e.g., interpolation mecanism, transformation model, similarity measure), an instance of this
    class is always provided for a registration algorithm using gradient or Hessian.
    */
   template <class T, class Storage>
   class RegistrationGradientHessianEvaluator
   {
   public:
      typedef RegistrationEvaluator<T, Storage>    Evaluator;

      virtual core::Buffer1D<double> evaluateGradient( const Evaluator& evaluator, const TransformationParametrized& transformationSourceToTarget ) const = 0;

      virtual core::Buffer1D<double> evaluateHessian( const Evaluator& evaluator, const TransformationParametrized& transformationSourceToTarget ) const = 0;

      virtual ~RegistrationGradientHessianEvaluator()
      {}
   };

   /**
    @brief Called to normalize the gradient in a specific way.

    For example in a rigid registration, the gradient of the rotation parameters will be much higher than the translation, so it is better to normalize them independently
    */
   class GradientPostprocessor
   {
   public:
      virtual void postprocess( core::Buffer1D<double>& gradient ) const = 0;

   };

   /**
    @ingroup algorithm
    @brief Utility class that computes the gradient of a similarity measure according to the transformation using finite difference
    @note make sure the interpolator handles sub-voxel transformation, else step must be >= 1 voxel (note that the tfm is defined in MM)

    computes an approximation of the derivative using:
    df(x)/dx = (f(x+step)-f(x)) / step
    */
   template <class T, class Storage>
   class RegistrationGradientHessianEvaluatorFiniteDifference : public RegistrationGradientHessianEvaluator<T, Storage>
   {
   public:
      typedef RegistrationGradientHessianEvaluator<T, Storage>        Base;
      typedef typename Base::Evaluator                         Evaluator;

      /**
       @param step the step used to compute the finite difference
       @param normalizeGradient if true, the gradient will be normalized

       @note the impact of the gradient normalization, some parameters may have much bigger impact!

       */
      RegistrationGradientHessianEvaluatorFiniteDifference( double step, bool normalizeGradient = true, std::shared_ptr<GradientPostprocessor> postprocessor = std::shared_ptr<GradientPostprocessor>() ) : _normalizeGradient( normalizeGradient ), _postprocessor( postprocessor )
      {
         _step = step;
      }

      virtual core::Buffer1D<double> evaluateGradient( const Evaluator& evaluator, const TransformationParametrized& transformationSourceToTarget ) const
      {
         GradientCalculatorFiniteDifference finiteDifference( evaluator, _step );
         const core::Buffer1D<double> parameters = transformationSourceToTarget.getParameters();
         core::Buffer1D<double> gradient = finiteDifference.evaluate( parameters );
         const double norm = core::norm2( gradient );

         if ( _normalizeGradient && norm > 1e-4 )
         {
            for ( size_t n = 0; n < parameters.size(); ++n )
            {
               gradient[ n ] /= norm;
            }
         }

         if ( _postprocessor.get() )
         {
            _postprocessor->postprocess( gradient );
         }

         return gradient;
      }

      virtual core::Buffer1D<double> evaluateHessian( const Evaluator& evaluator, const TransformationParametrized& transformationSourceToTarget ) const
      {
         HessianCalculatorForwardFiniteDifference calculator( evaluator, _step );
         return calculator.evaluate( transformationSourceToTarget.getParameters() );
      }

      virtual ~RegistrationGradientHessianEvaluatorFiniteDifference()
      {}

   protected:
      double                                 _step;
      bool                                   _normalizeGradient;
      std::shared_ptr<GradientPostprocessor> _postprocessor;
   };
}
}

#endif