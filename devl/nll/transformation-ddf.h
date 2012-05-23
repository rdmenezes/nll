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

         // fast index/MM pre-computations
         _ddfIndexToMM = _source2targetInv * ddfPst;
         _mmToDdfIndex.clone( _ddfIndexToMM );
         const bool success2 = core::inverse( _mmToDdfIndex );
         ensure( success2, "non affine tfm" );
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
       @note Internally, we need to apply the affine TFM
       */
      virtual core::vector3f transformDeformableOnly( const nll::core::vector3f& p ) const
      {
         const core::vector3f index = core::transf4( _mmToDdfIndex, p );
         DdfInterpolator interpolator( _ddf );
         return interpolator( index.getBuf() );
      }

      /**
       @brief transform a point defined in MM, the input point is transformed by the affine transformation only
       */
      core::vector3f transformAffineOnly( const nll::core::vector3f& p ) const
      {
         return core::transf4( _source2target, p );
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
         const core::vector3f indexDdf = _ddf.positionToIndex( paffine );
         const core::vector3f def = transformDeformableOnlyIndex( indexDdf );
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

      /**
       @brief Return a 3x3 matrix, the gradient of the DDF at this point <x> in MM. First column is dx, second dy...
       @param x the positionin MM
       @param pfx if not null, the DDF value at x
       @param scale the finite difference distance

       The gradient is computed using finite difference
       */
      Matrix getGradient( const core::vector3f& x, core::vector3f* pfx = 0, float scale = 0.1f ) const
      {
         core::vector3f fx = transform( x );
         if ( pfx )
         {
            *pfx = fx;
         }

         // compute the gradient  in each direction using finite difference
         // e.g., dx = ( f( x + scale * ( 1, 0, 0 ) ) - fx ) / scale
         const core::vector3f dx = ( transform( core::vector3f( x[ 0 ] + scale,
                                                                x[ 1 ],
                                                                x[ 2 ] ) ) - fx ) / scale;
         const core::vector3f dy = ( transform( core::vector3f( x[ 0 ],
                                                                x[ 1 ] + scale,
                                                                x[ 2 ] ) ) - fx ) / scale;
         const core::vector3f dz = ( transform( core::vector3f( x[ 0 ],
                                                                x[ 1 ],
                                                                x[ 2 ] + scale ) ) - fx ) / scale;

         Matrix grad( 3, 3 );
         for ( ui32 n = 0; n < 3; ++n )
         {
            grad( n, 0 ) = dx[ n ];
            grad( n, 1 ) = dy[ n ];
            grad( n, 2 ) = dz[ n ];
         }

         return grad;
      }

      /**
       @brief Given an point in MM, find the inverse
       @param v a point in MM
       @param converged_out if no null, true will be returned if the algorithm converged
       @note we want to find x such that transform( x ) = v;
       */
      core::vector3f getInverseTransform( const core::vector3f& v, ui32 maxIter = 1000, bool* converged_out = 0 ) const
      {
         // v = f(x) => if we don't have deformable displacement, then x = affine^-1 * v
         // which is our best initial guess
         core::vector3f x = core::transf4( _source2targetInv, v );
         core::vector3f fx;

         const float epsilon = 0.1f;
         const float epsilon2 = core::sqr( epsilon );

         ui32 iter = 0;
         core::vector3f d( 1, 1, 1 );
         while ( d.dot( d ) >= epsilon2 &&  iter < maxIter )
         {
            // a gradient descent to look for x such that f(x) = x0 (point)
            //                        (-1)
            // x    = x  - grad(f(x ))      . (f(x ) - x )
            // n+1    n           n              n     0
            Matrix gradinv = getGradient( x, &fx );
            core::inverse( gradinv );
            d = fx - v;

            Matrix update = gradinv * Matrix( core::Buffer1D<float>( d.getBuf(), 3, false ), 3, 1 );
            x[ 0 ] -= update[ 0 ];
            x[ 1 ] -= update[ 1 ];
            x[ 2 ] -= update[ 2 ];
            ++iter;
         }

         if ( converged_out )
         {
            *converged_out = iter < maxIter;
         }

         return x;
      }


      /**
       @brief helper method create a DDF

       The most common use case is to create a DDF mapping a target volume which has an affine source2target transformation associated. Given this, create a DDF
       which will fully map this target volume

       @note internally, we will create a DDF with size + 1 as the trilinear interpolator will not behave correctly at the volume boundary
       @param rbfTfm contains the affine source->target transform
       */
      static TransformationDenseDeformableField create( const core::DeformableTransformationRadialBasis<core::RbfGaussian>& rbfTfm, const core::Matrix<float>& targetPst, const core::vector3f& targetSizeMm, const core::vector3ui ddfSize )
      {
         typedef core::DeformableTransformationRadialBasis<core::RbfGaussian> RbfTfm;
         TransformationDenseDeformableField ddf = create( rbfTfm.getAffineTfm(), targetPst, targetSizeMm, ddfSize );

         // here we are approximating a RBF transformation by a fixed DDF
         // first we compute the DDF PST so that we are mapping the same source geometry
         // the affine part of the DDF is the same as the RBF (so we can discard it in the computations as they would be relatively at the same position)
         // it remains the RBF deformation to compute. The RBF are defined MM in the source geometry
         // so we simply need to convert the Index DDF to MM and get the RBF value

         // compute the transformation DDF index->Source MM
         const Matrix& ddfPst = ddf.getStorage().getPst();
         const core::vector3f dx( ddfPst( 0, 0 ),
                                  ddfPst( 1, 0 ),
                                  ddfPst( 2, 0 ) );
         const core::vector3f dy( ddfPst( 0, 1 ),
                                  ddfPst( 1, 1 ),
                                  ddfPst( 2, 1 ) );
         const core::vector3f dz( ddfPst( 0, 2 ),
                                  ddfPst( 1, 2 ),
                                  ddfPst( 2, 2 ) );
         core::vector3f start(  ddfPst( 0, 3 ),
                                ddfPst( 1, 3 ),
                                ddfPst( 2, 3 ) );

         // finally iterate on all voxels and compute the displacement vector
         #if !defined(NLL_NOT_MULTITHREADED)
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < (int)ddfSize[ 2 ]; ++z )
         {
            Ddf::DirectionalIterator  lineIt = ddf.getStorage().getIterator( 0, 0, z );
            core::vector3f linePosSrc = core::vector3f( start[ 0 ] + z * dz[ 0 ],
                                                        start[ 1 ] + z * dz[ 1 ],
                                                        start[ 2 ] + z * dz[ 2 ] );
            for ( ui32 y = 0; y < ddfSize[ 1 ]; ++y )
            {
               Ddf::DirectionalIterator  voxelIt = lineIt;
               
               core::vector3f mmPosSrc = linePosSrc;
               for ( ui32 x = 0; x < ddfSize[ 0 ]; ++x )
               {
                  // assign the DDF to the RBF displacement
                  core::Buffer1D<float> d = rbfTfm.getRawDeformableDisplacementOnly( core::Buffer1D<float>( mmPosSrc.getBuf(), 3, false ) );
                  (*voxelIt)[ 0 ] = d[ 0 ];
                  (*voxelIt)[ 1 ] = d[ 1 ];
                  (*voxelIt)[ 2 ] = d[ 2 ];

                  mmPosSrc += dx;
                  voxelIt.addx();
               }
               linePosSrc += dy;
               lineIt.addy();
            }
         }

         return ddf;
      }

   protected:
      Ddf         _ddf;
      Matrix      _source2target;
      Matrix      _source2targetInv;

      // compose the DDF PST and affine TFM
      Matrix      _ddfIndexToMM;
      Matrix      _mmToDdfIndex;
   };
}
}

#endif
