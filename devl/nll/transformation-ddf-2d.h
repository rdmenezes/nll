/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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

#ifndef NLL_CORE_TRANSFORMATION_DDF_2D_H_
# define NLL_CORE_TRANSFORMATION_DDF_2D_H_

namespace nll
{
namespace core
{
   /**
    @brief Map a source image to a target image using a DDF and affine transform

    The DDF is mapping the source image (and with its own geometry and size) and will be applied
    source->target affine transformation

    Internally, we are using a <Storage> image to store the displacement field. This field
    has its own affine matrix.
    */
   class DeformableTransformationDenseDisplacementField2d
   {
   public:
      typedef f32                                  value_type;
      typedef core::Matrix<value_type>             Matrix;
      typedef core::Buffer1D<value_type>           Vector;
      typedef core::ImageSpatial<value_type>       Storage;

      DeformableTransformationDenseDisplacementField2d() // empty but not valid DDF
      {}

      /**
       @brief create an empty DDF with a specified size and PST
       @param storageSize specify the number of pixels composint the DDF
       @param mappedSize the size to be mapped
       @param mappedTfm the affine tfm 
       */
      DeformableTransformationDenseDisplacementField2d( const core::vector2ui& sourceSize, const Matrix& sourcePst, const core::vector2ui& ddfSize, const Matrix tfm ) : _affine( tfm )
      {
         _affineUpdated();
         _storage = Storage( ddfSize[ 0 ] + 1, ddfSize[ 1 ] + 1, 2, _computeDdfPst( sourceSize, sourcePst, ddfSize ) ); // note that we increase the DDF size by one as the interpolator will not perform well for the last pixel
      }

      /**

      /**
       @brief create a DDF from 
       @param sourceSize the source that will be mapped by the DDF
       @param sourcePst the source that will be mapped by the DDF
       @param ddfSize the number of voxels used to store the DDF
       @param rbfTfm the source->target affine deformable transformation
       */
      template <class Rbf>
      DeformableTransformationDenseDisplacementField2d( const core::vector2ui& sourceSize, const Matrix& sourcePst, const core::vector2ui& ddfSize, const DeformableTransformationRadialBasis<Rbf>& rbfTfm )
      {
         importFromRbfTfm( sourceSize, sourcePst, ddfSize, rbfTfm );
      }

      /**
       @brief create a DDF form a RBF transformation.
       @param size the new size of the DDF
       @param tfm the PST of the DDF, which map index coordinates to MM coordinates
       @param rbfTfm a deformable transformation based on RBFs
       */
      template <class Rbf>
      void importFromRbfTfm( const core::vector2ui& sourceSize, const Matrix& sourcePst, const core::vector2ui& ddfSize, const DeformableTransformationRadialBasis<Rbf>& rbfTfm )
      {
         // here we are approximating a RBF transformation by a fixed DDF
         // first we compute the DDF PST so that we are mapping the same source geometry
         // the affine part of the DDF is the same as the RBF
         // it remains the RBF deformation to compute. The RBF are defined MM in the source geometry
         // so we simply need to convert the Index DDF to MM and get the RBF value

         // set up the DDF
         ensure( rbfTfm.getAffineTfm().size() == 9, "must be a 2D RBF tfm" );
         Matrix pstStorage = _computeDdfPst( sourceSize, sourcePst, ddfSize );
         _storage = Storage( ddfSize[ 0 ] + 1, ddfSize[ 1 ] + 1, 2, pstStorage );   // note that we increase the DDF size by one as the interpolator will not perform well for the last pixel

         // use the same affine TFM
         _affine = rbfTfm.getAffineTfm();
         _affineUpdated();

         // compute the transformation DDF index->Source MM
         const core::vector2f dx( getPst()( 0, 0 ),
                                  getPst()( 1, 0 ) );
         const core::vector2f dy( getPst()( 0, 1 ),
                                  getPst()( 1, 1 ) );
         core::vector2f linePos(  getPst()( 0, 2 ),
                                  getPst()( 1, 2 ) );

         // finally iterate on all voxels and compute the displacement vector
         for ( ui32 y = 0; y < _storage.sizey(); ++y )
         {
            core::vector2f startLine = linePos;
            for ( ui32 x = 0; x < _storage.sizex(); ++x )
            {
               value_type* p = _storage.point( x, y );
               Vector d = rbfTfm.getDisplacementInSourceSpace( startLine );
               p[ 0 ] = d[ 0 ];
               p[ 1 ] = d[ 1 ];

               startLine[ 0 ] += dx[ 0 ];
               startLine[ 1 ] += dx[ 1 ];
            }

            linePos[ 0 ] += dy[ 0 ];
            linePos[ 1 ] += dy[ 1 ];
         }
      }

      /**
       @brief Return the displacement at a specified point expressed in the source space in MM (ie. before applying the affine transformation)
       */
      template <class VectorT>
      core::vector2f getDisplacement( const VectorT& psource ) const
      {
         // compute p in target space
         Vector v( 3 );
         for ( ui32 n = 0; n < 2; ++n )
         {
            v[ n ] = psource[ n ];
         }
         v[ 2 ] = 1;

         Vector vsrc = getInvertedPst() * Matrix( v, v.size(), 1 );

         // now interpolate
         return getDisplacementSource( vsrc );
      }

      /**
       @brief Compute the displacement in index space directly
       */
      template <class VectorT>
      core::vector2f getDisplacementSource( const VectorT& index ) const
      {
         // now interpolate
         core::vector2f out;
         core::InterpolatorLinear2D<value_type, Storage::IndexMapper, Storage::Allocator> interpolator( _storage );
         interpolator.interpolateValues( index[ 0 ], index[ 1 ], &out[ 0 ] );
         return out;
      }

      ui32 sizex() const
      {
         return _storage.sizex();
      }

      ui32 sizey() const
      {
         return _storage.sizey();
      }

      const Matrix& getAffineTfm() const
      {
         return _affine;
      }

      const Matrix& getPst() const
      {
         return _storage.getPst();
      }

      const Matrix& getAffineInvTfm() const
      {
         return _invAffine;
      }

      const Matrix& getInvertedPst() const
      {
         return _storage.getInvertedPst();
      }

      const Storage& getStorage() const
      {
         return _storage;
      }

   private:
      void _affineUpdated()
      {
         _invAffine.clone( _affine );
         const bool r = core::inverse( _invAffine );
         ensure( r, "matrix is not affine" );
      }

      // given the source geometry, compute the PST so that the DDF has exactly the same orientation and size in MM than the source given the DDF size in pixel
      Matrix _computeDdfPst( const core::vector2ui& sourceSize, const Matrix& sourcePst, const core::vector2ui& ddfSize )
      {
         // compute the storage PST mapping to (sourceSize, tfm)
         const core::vector2f ddfSpacing = getSpacing3x3( sourcePst );
         const core::vector2f sizeMm( sourceSize[ 0 ] * ddfSpacing[ 0 ],
                                      sourceSize[ 1 ] * ddfSpacing[ 1 ] );

         // now compute the spacing for the storage
         const core::vector2f storageSpacing( sizeMm[ 0 ] / ddfSize[ 0 ],
                                              sizeMm[ 1 ] / ddfSize[ 1 ] );

         // finally compute the storage affine transformation: we simply update the spacing so that
         // we are mapping exactly the same area with the storage
         Matrix storagePst;
         storagePst.clone( sourcePst );
         for ( ui32 n = 0; n < 2; ++n )
         {
            storagePst( 0, n ) = storagePst( 0, n ) / ddfSpacing[ n ] * storageSpacing[ n ];
            storagePst( 1, n ) = storagePst( 1, n ) / ddfSpacing[ n ] * storageSpacing[ n ];
         }

         return storagePst;
      }

   private:
      Matrix      _affine;
      Matrix      _invAffine;
      Storage     _storage;
   };
}
}

#endif
