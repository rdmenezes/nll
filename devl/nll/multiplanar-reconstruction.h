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

#ifndef NLL_IMAGING_MULTIPLANAR_RECONSTRUCTION_H_
# define NLL_IMAGING_MULTIPLANAR_RECONSTRUCTION_H_

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression is constant, this is intended!

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Multiplanar reconstruction of a volume

    Volume must be a spatial volume as we need to know its position and orientation in space

    Extract a slice according to a plane.
    */
   template <class Volume, class Interpolator3D>
   class Mpr
   {
   public:
      typedef Volume          VolumeType; 
      typedef Interpolator3D  Interpolator;

      // if the volume is a floating point type, the interpolation is the same type
      // else a float
      typedef typename core::If<typename Volume::value_type, float, core::IsFloatingType<typename Volume::value_type>::value >::type SliceType;
      typedef imaging::Slice<SliceType>   Slice;

      //typedef core::Image<SliceType, core::IndexMapperRowMajorFlat2DColorRGBnMask> Slice;

   public:
      /**
       @brief set the size of the plane to be reconstructed in voxels
       */
      Mpr( const VolumeType& volume ) :
         _volume( volume )
      {}

      /**
       @brief Compute a MPR where it's transorfmation is set to identity.
       
       @note Typical use case is, we have a source and target volumes, with a registration matrix tfm
             source->target. We get the MPR of the source object with this method, as 'source' is
             already in source space.
       @param slice the slice to be filled, defined in source space
       */
      void getSlice( Slice& slice ) const
      {
         TransformationAffine id( core::identityMatrix<TransformationAffine::Matrix>( 4 ) );
         getSlice( slice, id );
      }

      /**
       @brief Compute the slice according to a position and 2 vectors and a size factor.
              The volume's spacing is used to compute the correct MPR.
       @param slice the slice to be filled, defined in source space
       @param tfm an affine that transform source->target, assuming we have a target volume. Internally, if will invert
              the tfm and apply it to the <_volume>
       @note Typical use case is, we have a source and target volumes, with a registration matrix tfm
             source->target, and <_volume> playing as the target volume
       */
      void getSlice( Slice& slice, const TransformationAffine& tfm2, bool isSliceCenter = true ) const
      {
         const double isCenter = isSliceCenter ? 1 : 0;

         // compute the rotation & scaling of the transformation tfm( pst( volume ) )
         // we inverse the resulting transformation: we actually fit the axis on the transformed volume!
         Transformation::Matrix transformationRot = ( tfm2.getInvertedAffineMatrix() * _volume.getPst() );
         core::inverse( transformationRot );

         // rotate & scale the axis of the MPR
         core::vector3f dx = core::mul4Rot( transformationRot, slice.getAxisX() );
         dx[ 0 ] = dx[ 0 ] * slice.getSpacing()[ 0 ];
         dx[ 1 ] = dx[ 1 ] * slice.getSpacing()[ 0 ];
         dx[ 2 ] = dx[ 2 ] * slice.getSpacing()[ 0 ];

         core::vector3f dy = core::mul4Rot( transformationRot, slice.getAxisY() );
         dy[ 0 ] = dy[ 0 ] * slice.getSpacing()[ 1 ];
         dy[ 1 ] = dy[ 1 ] * slice.getSpacing()[ 1 ];
         dy[ 2 ] = dy[ 2 ] * slice.getSpacing()[ 1 ];

         // compute the origin target within this transormation to build
         // a geometric space of the transformed volume
         core::Matrix<float> volumeToWorld = tfm2.getInvertedAffineMatrix() * _volume.getPst();
         core::vector3f targetOrigin = transf4( volumeToWorld, core::vector3f( 0, 0, 0 ) );

         // create the transformation representing this displacement and compute the source origin in this
         // coordinate system
         core::Matrix<float> g( 4, 4 );
         for ( ui32 y = 0; y < 3; ++y )
            for ( ui32 x = 0; x < 3; ++x )
               g( y, x ) = volumeToWorld(y, x);
         g( 3, 3 ) = 1;
         g( 0, 3 ) = targetOrigin[ 0 ];
         g( 1, 3 ) = targetOrigin[ 1 ];
         g( 2, 3 ) = targetOrigin[ 2 ];

         // compute the origin of the slice within the geometric space of the transformed volume
         core::VolumeGeometry geom2( g );
         core::vector3f index = geom2.positionToIndex( slice.getOrigin() );

         // if isCenter == true, the slice geometric space (0,0) is the center and not the bottom left point of the slice
         float startx = (float)( index[ 0 ] - isCenter * ( slice.size()[ 0 ] * dx[ 0 ] / 2 + slice.size()[ 1 ] * dy[ 0 ] / 2 ) );
         float starty = (float)( index[ 1 ] - isCenter * ( slice.size()[ 0 ] * dx[ 1 ] / 2 + slice.size()[ 1 ] * dy[ 1 ] / 2 ) );
         float startz = (float)( index[ 2 ] - isCenter * ( slice.size()[ 0 ] * dx[ 2 ] / 2 + slice.size()[ 1 ] * dy[ 2 ] / 2 ) );

         // set up the interpolator
         // if SSE is not supported, use the default interpolator
         if ( core::Equal<Interpolator, InterpolatorTriLinear< VolumeSpatial<float> > >::value && !core::Configuration::instance().isSupportedSSE2() )
         {
            typedef InterpolatorTriLinearDummy< Volume > InterpolatorNoSSE;
            InterpolatorNoSSE interpolator( _volume );
            _fill<InterpolatorNoSSE>( startx, starty, startz, dx, dy, interpolator, slice );
         } else {
            Interpolator interpolator( _volume );
            _fill<Interpolator>( startx, starty, startz, dx, dy, interpolator, slice );
         }
      }

   protected:
      template <class Interpolator>
      void _fill( float startx, float starty, float startz, const core::vector3f& dx, const core::vector3f& dy, Interpolator& interpolator, Slice& slice ) const
      {
         #if !defined(NLL_NOT_MULTITHREADED) && !defined(NLL_NOT_MULTITHREADED_FOR_QUICK_OPERATIONS)
         # pragma omp parallel for
         #endif
         for ( int y = 0; y < (int)slice.size()[ 1 ]; ++y )
         {
            NLL_ALIGN_16 float pos[ 4 ] =
            {
               startx + dy[ 0 ] * y,
               starty + dy[ 1 ] * y,
               startz + dy[ 2 ] * y,
               0
            };

            Interpolator interpolatorCp = interpolator;
            interpolatorCp.startInterpolation();

            typename Slice::DirectionalIterator it = slice.getIterator( 0, y );
            for ( ui32 x = 0; x < slice.size()[ 0 ]; ++x )
            {
               *it = interpolatorCp( pos );
               pos[ 0 ] += dx[ 0 ];
               pos[ 1 ] += dx[ 1 ];
               pos[ 2 ] += dx[ 2 ];
               it.addx();
            }

            interpolatorCp.endInterpolation();
         }
      }

      Mpr& operator=( const Mpr& );

   protected:
      const VolumeType& _volume;
   };
}
}

# pragma warning( pop )

#endif
