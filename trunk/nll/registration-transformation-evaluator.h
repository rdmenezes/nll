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

#ifndef NLL_ALGORITHM_TRANSFORMATION_EVALUATOR_H_
# define NLL_ALGORITHM_TRANSFORMATION_EVALUATOR_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Evaluate a registration base class to support intensity based registration
    */
   template <class T, class Storage>
   class RegistrationEvaluator : public core::NonCopyable, public OptimizerClient
   {
   public:
      typedef imaging::VolumeSpatial<T, Storage>   Volume;

   public:
      RegistrationEvaluator( const Volume& source, const Volume& target, const TransformationCreator& creator ) : _source( &source ), _target( &target ), _transformationCreator( &creator )
      {}

      RegistrationEvaluator() : _source( 0 ), _target( 0 ), _transformationCreator( 0 )
      {}

      /**
       @brief Evaluate the transformation source->target
       */
      virtual double evaluate( const TransformationParametrized& transformationSourceToTarget ) const = 0;

      /**
       @brief Evaluate the gradient of the transformation
       */
      virtual core::Buffer1D<double> evaluateGradient( const TransformationParametrized& /*transformationSourceToTarget*/ ) const
      {
         throw std::runtime_error( "not implemented" );
      }

      virtual ~RegistrationEvaluator()
      {}

      const Volume& getSource() const
      {
         ensure( _source, "source volume was not set" );
         return *_source;
      }

      const Volume& getTarget() const
      {
         ensure( _target, "target volume was not set" );
         return *_target;
      }

      void setTarget( const Volume& target )
      {
         _target = &target;
      }

      void setSource( const Volume& source )
      {
         _source = &source;
      }

      /**
       @brief Helper class that bridges the optimization parameters and the imaging::Transformation
       */
      const TransformationCreator& getTransformationCreator() const
      {
         ensure( _target, "TransformationCreator was not set" );
         return *_transformationCreator;
      }

      /**
       @brief Helper class that bridges the optimization parameters and the imaging::Transformation
       */
      void setTransformationCreator( const TransformationCreator& transformationCreator )
      {
         _transformationCreator = &transformationCreator;
      }

   protected:
      // implement the function evaluation
      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const
      {
         ensure( _source && _target, "source/target not set" );
         ensure( _transformationCreator, "the transformation creator must be set!" );
         return evaluate( *_transformationCreator->create( parameters ) );
      }

      virtual core::Buffer1D<double> evaluateGradient( const core::Buffer1D<f64>& parameters ) const
      {
         ensure( _source && _target, "source/target not set" );
         ensure( _transformationCreator, "the transformation creator must be set!" );
         return evaluateGradient( *_transformationCreator->create( parameters ) );
      }

   protected:
      const Volume*                 _source;
      const Volume*                 _target;
      const TransformationCreator*  _transformationCreator;
   };
}
}

#endif