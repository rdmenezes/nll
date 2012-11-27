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
    @ingroup algorithm
    @brief Create a parametrized transformation
    @note it is assumed that the parameters are always synchronized with the parameters.
          Since a transformation can have different parametrizations (e.g. a 4x4 matrix can represent
          a rigid, similarity, affine transformations...) there is not a 1 to 1 mapping meaning
          that we need to enforce matrix immutability (which by default it is as we can't set transformation
          parameters after construction)
    */
   class TransformationParametrized : public virtual imaging::Transformation
   {
   public:
      virtual const core::Buffer1D<double>& getParameters() const = 0;
   };

   
   /**
    @ingroup algorithm
    @brief Create a parametrized transformation based on an affine representation (i.e., can be rigid, affine, similarity...
           so the parameter size may varies and will not be unique this is why we need to store them)
    @note TransformationParametrizedAffine must be a TransformationAffine so that we can directly use it in the transformation mapper
    */
   class TransformationParametrizedAffine : public imaging::TransformationAffine, public TransformationParametrized
   {
   public:
      typedef imaging::TransformationAffine::Matrix   Matrix;

      TransformationParametrizedAffine( const Matrix& init, const core::Buffer1D<double>& parameters ) : TransformationAffine( init )
      {
         _parameters.clone( parameters );
      }

      /**
       @brief Returns the parameters represented by the affine matrix. The caller needs to know what parametrization
              was used in order to use it correctly
       */
      virtual const core::Buffer1D<double>& getParameters() const
      {
         return _parameters;
      }



   protected:
      core::Buffer1D<double>  _parameters;
   };

   /**
    @brief Transformation creator base class

    Create a transformation from a list of parameters
    */
   class TransformationCreator
   {
   public:
      typedef TransformationParametrized::Matrix   Matrix;

      /**
       @brief set an initial tranformation

       Typically, the prepending transform is used as an initial transformation and mostly used to center the volume. This is to help the minization
       algorithm so that when 1 parameter is changed, the corresponding transformation varies only in one "direction". Consider the scaling,
       if the volumes are not centered, a change in scaling will require scaling + translation update by the optimizer (if they are centered,
       a scaling update will not introduce translation bias).

       Typically, "create" should return the transformation: create(params) * prependingTransformation
       */
      virtual void setPrependingMatrix( const Matrix& m ) = 0;

      /**
       @brief Return the optimizer parameters, allowing the optimizer to generate new instances of the transformation
              
       This will mostly be used in the context of an optimization with multi-seed scheme.
       */
      virtual ParameterOptimizers getOptimizerParameters() const = 0;

      virtual ~TransformationCreator()
      {}

      /**
       @brief create a transformation represented by the parameters
       */
      virtual std::shared_ptr<TransformationParametrized> create( const nll::core::Buffer1D<nll::f64>& parameters ) const = 0;
   };

   /**
    @brief Rigid Transformation
    
    It is modeled by 3 parameters (tx, ty, tz) such that:
        | 1 0 0 tx |
    T = | 0 1 0 ty |
        | 0 0 1 tz |
        | 0 0 0 1  |
    */
   class TransformationCreatorTranslation : public TransformationCreator
   {
   public:
      virtual std::shared_ptr<TransformationParametrized> create( const nll::core::Buffer1D<nll::f64>& parameters ) const
      {
         ensure( parameters.size() == 3, "only (tx, ty, tz) parameters expected" );
         core::Matrix<float> tfmMat = core::identityMatrix< core::Matrix<float> >( 4 );
         tfmMat( 0, 3 ) = static_cast<float>( parameters[ 0 ] );
         tfmMat( 1, 3 ) = static_cast<float>( parameters[ 1 ] );
         tfmMat( 2, 3 ) = static_cast<float>( parameters[ 2 ] );

         std::shared_ptr<TransformationParametrized> tfm( new TransformationParametrizedAffine( tfmMat * _prependingMatrix, parameters ) );
         return tfm;
      }

      virtual ParameterOptimizers getOptimizerParameters() const
      {
         ParameterOptimizers parameters;
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
         return parameters;
      }

      virtual void setPrependingMatrix( const Matrix& m )
      {
         _prependingMatrix = m.clone();
      }

      TransformationCreatorTranslation()
      {
         _prependingMatrix = core::identityMatrix<Matrix>( 4 );
      }

   protected:
      Matrix   _prependingMatrix;
   };

   /**
    @brief Translation + Scaling Transformation
    
    It is modeled by 3 parameters (tx, ty, tz, sx, sy, sz) such that:
        | sx 0  0  tx |
    T = | 0  sy 0  ty |
        | 0  0  sz tz |
        | 0  0  0  1  |
    */
   class TransformationCreatorTranslationScaling : public TransformationCreator
   {
   public:
      virtual std::shared_ptr<TransformationParametrized> create( const nll::core::Buffer1D<nll::f64>& parameters ) const
      {
         ensure( parameters.size() == 6, "only (tx, ty, tz, sx, sy, sz) parameters expected" );
         core::Matrix<float> tfmMat = core::identityMatrix< core::Matrix<float> >( 4 );
         tfmMat( 0, 3 ) = static_cast<float>( parameters[ 0 ] );
         tfmMat( 1, 3 ) = static_cast<float>( parameters[ 1 ] );
         tfmMat( 2, 3 ) = static_cast<float>( parameters[ 2 ] );

         tfmMat( 0, 0 ) = static_cast<float>( parameters[ 3 ] );
         tfmMat( 1, 1 ) = static_cast<float>( parameters[ 4 ] );
         tfmMat( 2, 2 ) = static_cast<float>( parameters[ 5 ] );

         std::shared_ptr<TransformationParametrized> tfm( new TransformationParametrizedAffine( tfmMat * _prependingMatrix, parameters ) );
         return tfm;
      }

      virtual ParameterOptimizers getOptimizerParameters() const
      {
         ParameterOptimizers parameters;
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );

         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( 0.5, 1.5, 1, 0.75, 0.05 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( 0.5, 1.5, 1, 0.75, 0.05 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( 0.5, 1.5, 1, 0.75, 0.05 ) );
         return parameters;
      }

      virtual void setPrependingMatrix( const Matrix& m )
      {
         _prependingMatrix = m.clone();
      }

      TransformationCreatorTranslationScaling()
      {
         _prependingMatrix = core::identityMatrix<Matrix>( 4 );
      }

   protected:
      Matrix   _prependingMatrix;
   };

   /**
    @brief Rigid Transformation
    
    It is modeled by 6 parameters (tx, ty, tz) the translation in MM
                                  (rx, ry, rz) the euler rotation angles in radians
        | 1 0 0 tx |
    T = | 0 1 0 ty | * R(rz) * R(ry) * R(rx)
        | 0 0 1 tz |
        | 0 0 0 1  |
    */
   class TransformationCreatorRigid : public TransformationCreator
   {
   public:
      virtual std::shared_ptr<TransformationParametrized> create( const nll::core::Buffer1D<nll::f64>& parameters ) const
      {
         ensure( parameters.size() == 6, "only (tx, ty, tz, rx, ry, rz) parameters expected" );
         core::Matrix<float> tfmMat = core::identityMatrix< core::Matrix<float> >( 4 );
         tfmMat( 0, 3 ) = static_cast<float>( parameters[ 0 ] );
         tfmMat( 1, 3 ) = static_cast<float>( parameters[ 1 ] );
         tfmMat( 2, 3 ) = static_cast<float>( parameters[ 2 ] );

         core::Matrix<float> rot;
         core::createRotationMatrix4x4FromEuler( core::vector3f( (float)parameters[ 3 ],
                                                                 (float)parameters[ 4 ],
                                                                 (float)parameters[ 5 ] ),
                                                 rot );

         std::shared_ptr<TransformationParametrized> tfm( new TransformationParametrizedAffine( tfmMat * rot * _prependingMatrix, parameters ) );
         return tfm;
      }

      virtual ParameterOptimizers getOptimizerParameters() const
      {
         ParameterOptimizers parameters;
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 1 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 1 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 1 ) );

         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -core::PIf / 2, core::PIf / 2, 0, 300, 0.01 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -core::PIf / 2, core::PIf / 2, 0, 300, 0.01 ) );
         parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -core::PIf / 2, core::PIf / 2, 0, 300, 0.01 ) );
         return parameters;
      }

      virtual void setPrependingMatrix( const Matrix& m )
      {
         _prependingMatrix = m.clone();
      }

      TransformationCreatorRigid()
      {
         _prependingMatrix = core::identityMatrix<Matrix>( 4 );
      }

   protected:
      Matrix   _prependingMatrix;
   };
}
}

#endif