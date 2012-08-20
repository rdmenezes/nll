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
    */
   template <class T, class Storage>
   class RegistrationGradientEvaluator
   {
   public:
      typedef RegistrationEvaluator<T, Storage>    Evaluator;

      RegistrationGradientEvaluator( const Evaluator& evaluator ) : _evaluator( evaluator )
      {}

      virtual core::Buffer1D<double> evaluateGradient( const imaging::Transformation& transformationSourceToTarget ) const = 0;

      virtual ~RegistrationGradientEvaluator()
      {}

   protected:
      const Evaluator&      _evaluator;
   };

   /**
    @ingroup algorithm
    @brief Utility class that computes the gradient of a similarity measure according to the transformation using finite difference
    @note make sure the interpolator handles sub-voxel transformation, else step must be >= 1 voxel (note that the tfm is defined in MM)

    computes an approximation of the derivative using:
    df(x)/dx = (f(x+step)-f(x)) / step
    */
   template <class T, class Storage>
   class RegistrationGradientEvaluatorFiniteDifference : public RegistrationGradientEvaluator<T, Storage>
   {
   public:
      typedef RegistrationGradientEvaluator<T, Storage>        Base;
      typedef typename Base::Evaluator                         Evaluator;

      RegistrationGradientEvaluatorFiniteDifference( const Evaluator& evaluator, core::Buffer1D<double> steps ) : _evaluator( evaluator ), _steps( steps )
      {}

      virtual core::Buffer1D<double> evaluateGradient( const imaging::Transformation& transformationSourceToTarget ) const
      {
         const double val0 = evaluator.evaluate( transformationSourceToTarget );

         core::Buffer1D<double> parameters = evaluator.getTransformationCreator().getParameters( transformationSourceToTarget );
         ensure( parameters.size() == step.size(), "we must have a step for each parameter of the transformation" );
         core::Buffer1D<double> gradient( parameters.size() );
         for ( size_t n = 0; n < parameters.size(); ++n )
         {
            core::Buffer1D<double> parametersCpy;
            parametersCpy.clone( parameters );
            parametersCpy[ n ] += _steps[ n ]

            const double val = evaluator.evaluate( evaluator.getTransformationCreator().create( parametersCpy ) );
            gradient[ n ] = ( val - val0 ) / _steps[ n ];
         }

         return gradient;
      }

      virtual ~RegistrationGradientEvaluatorFiniteDifference()
      {}

   protected:
      const Evaluator&        _evaluator;
      core::Buffer1D<double>  _steps;
   };
}
}

#endif