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

#ifndef NLL_ALGORITHM_TRANSFORMATION_CREATOR_H_
# define NLL_ALGORITHM_TRANSFORMATION_CREATOR_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Transformation creator base class

    Create a transformation from a list of parameters
    */
   class TransformationCreator
   {
   public:
      /**
       @brief create a transformation represented by the parameters
       */
      virtual std::shared_ptr<imaging::Transformation> create( const nll::core::Buffer1D<nll::f64>& parameters ) const = 0;

      /**
       @brief from a transformation, find the corresponding parameters. If the full transformation cannot be recovered,
              the parameters approaching the best will be returned
       */
      virtual core::Buffer1D<f64> getParameters( const imaging::Transformation& transformation ) const = 0;

      /**
       @brief Return the optimizer parameters, allowing the optimizer to generate new instances of the transformation
              (note that not all optimizers are using it)
       */
      virtual ParameterOptimizers getOptimizerParameters() const = 0;

      virtual ~TransformationCreator()
      {}
   };

   /**
    @brief Rigid Transformation
    
    It is modeled by 3 parameters (tx, ty, tz) such that:
        | 1 0 0 tx |
    T = | 0 1 0 ty |
        | 0 0 1 tz |
        | 0 0 0 1  |
    */
   class TransformationCreatorRigid : public TransformationCreator
   {
   public:
      virtual std::shared_ptr<imaging::Transformation> create( const nll::core::Buffer1D<nll::f64>& parameters ) const
      {
         ensure( parameters.size() == 3, "only (tx, ty, tz) parameters expected" );
         core::Matrix<float> tfmMat = core::identityMatrix< core::Matrix<float> >( 4 );
         tfmMat( 0, 3 ) = static_cast<float>( parameters[ 0 ] );
         tfmMat( 1, 3 ) = static_cast<float>( parameters[ 1 ] );
         tfmMat( 2, 3 ) = static_cast<float>( parameters[ 2 ] );

         std::shared_ptr<imaging::Transformation> tfm( new imaging::TransformationAffine( tfmMat ) );
         return tfm;
      }

      virtual core::Buffer1D<f64> getParameters( const imaging::Transformation& transformation ) const
      {
         const imaging::TransformationAffine* tfm = dynamic_cast<const imaging::TransformationAffine*>( &transformation );
         ensure( tfm, "it must be an affine transformatiob!" );

         core::Buffer1D<f64> params( 3 );
         params[ 0 ] = tfm->getAffineMatrix()( 0, 3 );
         params[ 1 ] = tfm->getAffineMatrix()( 1, 3 );
         params[ 2 ] = tfm->getAffineMatrix()( 2, 3 );
         return params;
      }

      virtual ParameterOptimizers getOptimizerParameters() const
      {
         ParameterOptimizers parameters;
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
         return parameters;
      }
   };
}
}

#endif