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

#ifndef NLL_IMAGING_TRANSFORMATION_DDF_3D_H_
# define NLL_IMAGING_TRANSFORMATION_DDF_3D_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Defines a dense Deformable field transformation

    We can see the transformation as follow:
    - start from a point in MM = pa
    - transform <pa> using the affine component = pm
    - transform <pm> in index using the DDF PST = pi
    - interpolate the DDF at <pi> to get the deformable displacement = pd
    - return pm + pd

    Internally, the Ddf stores an affine transformation and a PST for the displacement field.
    The DDF PST is used to position the DDF grid, while the affine transformation is moving this grid

    As an example, an index x in the DDF is transformed by the DDF's PST, returning a point in mm, this point is further transformed by the affine transformation to get
    a final position in MM
    */
   class TransformationDenseDeformableField : public Transformation
   {
   public:
      typedef VolumeSpatial<core::vector3f>   Ddf;
      typedef InterpolatorTriLinearDummy<Ddf> DdfInterpolator;

      /**
       @brief The most basic constructor
       @param ddfPst the affine transformation mapping the DDF index in MM
       */
      template <class T>
      TransformationDenseDeformableField( const core::Matrix<T>& source2target, const core::Matrix<T>& ddfPst, const core::vector3ui& ddfSize )
      {
         core::StaticVector<float, 3> initVal;
         _ddf = Ddf( ddfSize, ddfPst, initVal );
         _source2target.import( source2target );

         _source2targetInv.import( source2target );
         const bool success = core::inverse( _source2targetInv );
         ensure( success, "non affine tfm" );
      }

      /**
       @brief clone the transformation
       */
      virtual Transformation* clone() const
      {
         TransformationDenseDeformableField* ddf = new TransformationDenseDeformableField( _source2target, _ddf.getPst(), _ddf.getSize() );
         return ddf;
      }

      /**
       @brief get the affine part of the transformation
       */
      virtual const Matrix& getAffineMatrix() const
      {
         return _source2target;
      }

      /**
       @brief get the inverse of the affine transformation
       */
      virtual const Matrix& getInvertedAffineMatrix() const
      {
         return _source2targetInv;
      }

      /**
       @brief Returns the DDF storage
       */
      const Ddf& getStorage() const
      {
         return _ddf;
      }

      /**
       @brief Returns the DDF storage
       */
      Ddf& getStorage()
      {
         return _ddf;
      }

      /**
       @brief transform a point defined in MM, returns the deformable displacement at this position in MM
       */
      virtual core::vector3f transformDeformableOnly( const nll::core::vector3f& p ) const
      {
         const core::vector3f index = core::transf4( _ddf.getInvertedPst(), p );
         DdfInterpolator interpolator( _ddf );
         return interpolator( index.getBuf() );
      }

      /**
       @brief same as <transformDeformableOnly> but with an index in the DDF index directly
       */
      core::vector3f transformDeformableOnlyIndex( const nll::core::vector3f& pIndex ) const
      {
         DdfInterpolator interpolator( _ddf );
         return interpolator( pIndex.getBuf() );
      }

      /**
       @brief transform a point defined in source geometry in MM to the new target in MM
       */
      virtual core::vector3f transform( const nll::core::vector3f& p ) const
      {
         const core::vector3f paffine = core::transf4( _source2target, p );
         const core::vector3f def = transformDeformableOnly( paffine );
         return core::vector3f( paffine[ 0 ] + def[ 0 ],
                                paffine[ 1 ] + def[ 1 ],
                                paffine[ 2 ] + def[ 2 ] );
      }

      /**
       @brief helper method create a DDF

       The most common use case is to create a DDF mapping a target volume which has an affine source2target transformation associated. Given this, create a DDF
       which will fully map this target volume

       @note internally, we will create a DDF with size + 1 as the trilinear interpolator will not behave correctly at the volume boundary
       */
      static TransformationDenseDeformableField create( const core::Matrix<float>& source2TargetTfm, const core::Matrix<float>& targetPst, const core::vector3f& targetSizeMm, const core::vector3ui ddfSize )
      {
         typedef Ddf::Matrix  Matrix;

         // compute the DDF spacing and corresponding tfm
         const core::vector3f ddfSpacing( targetSizeMm[ 0 ] / ddfSize[ 0 ],
                                          targetSizeMm[ 1 ] / ddfSize[ 1 ],
                                          targetSizeMm[ 2 ] / ddfSize[ 2 ] );
         const Matrix ddfSpacingTfm = core::createScaling4x4( ddfSpacing );

         const Matrix ddfPst = targetPst * ddfSpacingTfm;
         return TransformationDenseDeformableField( source2TargetTfm, ddfPst, ddfSize );
      }

   protected:
      Ddf         _ddf;
      Matrix      _source2target;
      Matrix      _source2targetInv;
   };
}
}

#endif
