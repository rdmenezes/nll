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
       @brief Given a transformation, dispatch the MPR computation to the correct method
       */
      void getSlice( Slice& slice, const Transformation& tfm, bool isSliceCenter = true ) const
      {
         const TransformationAffine* tfmAffine = dynamic_cast<const TransformationAffine*>( &tfm );
         if ( tfmAffine )
         {
            getSlice( slice, *tfmAffine, isSliceCenter );
            return;
         }

         const TransformationDenseDeformableField* tfmDdf = dynamic_cast<const TransformationDenseDeformableField*>( &tfm );
         if ( tfmDdf )
         {
            getSlice( slice, *tfmDdf, isSliceCenter );
            return;
         }

         ensure( 0, "the transformation type is not hanled!" );
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
      void getSlice( Slice& slice, const TransformationAffine& tfm, bool isSliceCenter = true ) const
      {
         const float isCenter = isSliceCenter ? 1.0f : 0.0f;

         // (1) compute the transformation index source position MM->target index with affine target->source TFM applied
         const core::Matrix<float> srcMmTargetIndex = _volume.getInvertedPst() * tfm.getAffineMatrix();
         const core::vector3f originInTarget = core::transf4( srcMmTargetIndex, slice.getOrigin() );

         // finally get the axis: rotate & scale the axis of the MPR
         const core::vector3f dx = core::mul4Rot( srcMmTargetIndex, slice.getAxisX() ) * slice.getSpacing()[ 0 ];
         const core::vector3f dy = core::mul4Rot( srcMmTargetIndex, slice.getAxisY() ) * slice.getSpacing()[ 1 ];

         // if isCenter == true, the slice geometric space (0,0) is the center and not the bottom left point of the slice
         const float startx = (float)( originInTarget[ 0 ] - isCenter * ( slice.size()[ 0 ] * dx[ 0 ] / 2 + slice.size()[ 1 ] * dy[ 0 ] / 2 ) );
         const float starty = (float)( originInTarget[ 1 ] - isCenter * ( slice.size()[ 0 ] * dx[ 1 ] / 2 + slice.size()[ 1 ] * dy[ 1 ] / 2 ) );
         const float startz = (float)( originInTarget[ 2 ] - isCenter * ( slice.size()[ 0 ] * dx[ 2 ] / 2 + slice.size()[ 1 ] * dy[ 2 ] / 2 ) );
         const core::vector3f start( startx, starty, startz );

         // set up the interpolator
         // if SSE is not supported, use the default interpolator
         if ( core::Equal<Interpolator, InterpolatorTriLinear< VolumeSpatial<float> > >::value && !core::Configuration::instance().isSupportedSSE2() )
         {
            typedef InterpolatorTriLinearDummy< Volume > InterpolatorNoSSE;
            InterpolatorNoSSE interpolator( _volume );
            _fill<InterpolatorNoSSE>( start, dx, dy, interpolator, slice );
         } else {
            Interpolator interpolator( _volume );
            _fill<Interpolator>( start, dx, dy, interpolator, slice );
         }
      }

      /**
       @brief Compute the slice according to a position and 2 vectors and a size factor.
              The volume's spacing is used to compute the correct MPR.
       @param slice the slice to be filled, defined in source space
       @param tfm a DDF transform composed of a DDF and an affine that transform source->target, assuming we have a target volume. Internally, if will invert
              the tfm and apply it to the <_volume>
       @note Typical use case is, we have a source and target volumes, with a registration matrix tfm
             source->target, and <_volume> playing as the target volume
       */
      void getSlice( Slice& slice, const TransformationDenseDeformableField& tfm, bool isSliceCenter = true ) const
      {
         const float isCenter = isSliceCenter ? 1 : 0;

         // (1) compute the transformation index source position MM->target index with affine target->source TFM applied
         const core::Matrix<float> srcMmTargetIndex    = _volume.getInvertedPst() * tfm.getAffineMatrix();
         const core::Matrix<float> srcMmTargetIndexDdf = tfm.getStorage().getInvertedPst() * tfm.getAffineMatrix();
         const core::vector3f originInTarget    = core::transf4( srcMmTargetIndex, slice.getOrigin() );
         const core::vector3f originInTargetDdf = core::transf4( srcMmTargetIndexDdf, slice.getOrigin() );

         // finally get the axis: rotate & scale the axis of the MPR
         const core::vector3f dx = core::mul4Rot( srcMmTargetIndex, slice.getAxisX() ) * slice.getSpacing()[ 0 ];
         const core::vector3f dy = core::mul4Rot( srcMmTargetIndex, slice.getAxisY() ) * slice.getSpacing()[ 1 ];
         const core::vector3f dxDdf = core::mul4Rot( srcMmTargetIndexDdf, slice.getAxisX() ) * slice.getSpacing()[ 0 ];
         const core::vector3f dyDdf = core::mul4Rot( srcMmTargetIndexDdf, slice.getAxisY() ) * slice.getSpacing()[ 1 ];

         // if isCenter == true, the slice geometric space (0,0) is the center and not the bottom left point of the slice
         const float startx = (float)( originInTarget[ 0 ] - isCenter * ( slice.size()[ 0 ] * dx[ 0 ] / 2 + slice.size()[ 1 ] * dy[ 0 ] / 2 ) );
         const float starty = (float)( originInTarget[ 1 ] - isCenter * ( slice.size()[ 0 ] * dx[ 1 ] / 2 + slice.size()[ 1 ] * dy[ 1 ] / 2 ) );
         const float startz = (float)( originInTarget[ 2 ] - isCenter * ( slice.size()[ 0 ] * dx[ 2 ] / 2 + slice.size()[ 1 ] * dy[ 2 ] / 2 ) );

         const float startxDdf = (float)( originInTargetDdf[ 0 ] - isCenter * ( slice.size()[ 0 ] * dxDdf[ 0 ] / 2 + slice.size()[ 1 ] * dyDdf[ 0 ] / 2 ) );
         const float startyDdf = (float)( originInTargetDdf[ 1 ] - isCenter * ( slice.size()[ 0 ] * dxDdf[ 1 ] / 2 + slice.size()[ 1 ] * dyDdf[ 1 ] / 2 ) );
         const float startzDdf = (float)( originInTargetDdf[ 2 ] - isCenter * ( slice.size()[ 0 ] * dxDdf[ 2 ] / 2 + slice.size()[ 1 ] * dyDdf[ 2 ] / 2 ) );

         const core::vector3f start( startx, starty, startz );
         const core::vector3f startDdf( startxDdf, startyDdf, startzDdf );

         // set up the interpolator
         // if SSE is not supported, use the default interpolator
         if ( core::Equal<Interpolator, InterpolatorTriLinear< VolumeSpatial<float> > >::value && !core::Configuration::instance().isSupportedSSE2() )
         {
            typedef InterpolatorTriLinearDummy< Volume > InterpolatorNoSSE;
            InterpolatorNoSSE interpolator( _volume );
            _fillDdf<InterpolatorNoSSE>( tfm, start, startDdf, dx, dxDdf, dy, dyDdf, interpolator, slice );
         } else {
            Interpolator interpolator( _volume );
            _fillDdf<Interpolator>( tfm, start, startDdf, dx, dxDdf, dy, dyDdf, interpolator, slice );
         }
      }

   protected:
      template <class Interpolator>
      void _fill( const core::vector3f& start, const core::vector3f& dx, const core::vector3f& dy, Interpolator& interpolator, Slice& slice ) const
      {
         #if !defined(NLL_NOT_MULTITHREADED) && !defined(NLL_NOT_MULTITHREADED_FOR_QUICK_OPERATIONS)
         # pragma omp parallel for
         #endif
         for ( int y = 0; y < (int)slice.size()[ 1 ]; ++y )
         {
            NLL_ALIGN_16 float pos[ 4 ] =
            {
               start[ 0 ] + dy[ 0 ] * y,
               start[ 1 ] + dy[ 1 ] * y,
               start[ 2 ] + dy[ 2 ] * y,
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

      template <class Interpolator>
      void _fillDdf( const TransformationDenseDeformableField& tfm, const core::vector3f& start, const core::vector3f& startDdf, const core::vector3f& dx, const core::vector3f& dxDdf, const core::vector3f& dy, const core::vector3f& dyDdf, const Interpolator& interpolator, Slice& slice ) const
      {
         const impl::TransformationHelper transformationHelper( _volume.getInvertedPst() );

         #if !defined(NLL_NOT_MULTITHREADED) && !defined(NLL_NOT_MULTITHREADED_FOR_QUICK_OPERATIONS)
         # pragma omp parallel for
         #endif
         for ( int y = 0; y < (int)slice.size()[ 1 ]; ++y )
         {
            core::vector3f pos = start + dy * y;
            core::vector3f posDdf = startDdf + dyDdf * y;

            Interpolator interpolatorCp = interpolator;
            interpolatorCp.startInterpolation();

            typename Slice::DirectionalIterator it = slice.getIterator( 0, y );
            for ( ui32 x = 0; x < slice.size()[ 0 ]; ++x )
            {
               // get the displacement in MM space. We need to convert it as an index in the target volume
               core::vector3f displacement = tfm.transformDeformableOnlyIndex( posDdf );  // displacement in MM
               transformationHelper.transform( displacement ); // translate the displacement in MM into the corresponding target index

               const NLL_ALIGN_16 float posWithDdf[ 4 ] =
               {
                  pos[ 0 ] + displacement[ 0 ],
                  pos[ 1 ] + displacement[ 1 ],
                  pos[ 2 ] + displacement[ 2 ],
                  0
               };

               *it = interpolatorCp( posWithDdf );
               pos += dx;
               posDdf += dxDdf;
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
