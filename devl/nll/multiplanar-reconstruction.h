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
       @brief Compute the slice according to a position and 2 vectors and a size factor.
              The volume's spacing is used to compute the correct MPR.
       @param slice the slice to be filled
       */
      void getSlice( Slice& slice ) const
      {
         assert( slice.getSpacing()[ 0 ] > 0 && slice.getSpacing()[ 1 ] > 0 );

         // compute the slopes. First rotate the vectors so we are in the same coordinate system
         core::vector3f dx = core::mul4Rot( _volume.getInversedPst(), slice.getAxisX() );
         const float c1 = (float)dx.norm2() / slice.getSpacing()[ 0 ];
         dx[ 0 ] = dx[ 0 ] / ( c1 * _volume.getSpacing()[ 0 ] );
         dx[ 1 ] = dx[ 1 ] / ( c1 * _volume.getSpacing()[ 1 ] );
         dx[ 2 ] = dx[ 2 ] / ( c1 * _volume.getSpacing()[ 2 ] );

         core::vector3f dy = core::mul4Rot( _volume.getInversedPst(), slice.getAxisY() );
         const float c2 = (float)dy.norm2() / slice.getSpacing()[ 1 ];
         dy[ 0 ] = dy[ 0 ] / ( c2 * _volume.getSpacing()[ 0 ] );
         dy[ 1 ] = dy[ 1 ] / ( c2 * _volume.getSpacing()[ 1 ] );
         dy[ 2 ] = dy[ 2 ] / ( c2 * _volume.getSpacing()[ 2 ] );

         // reconstruct the slice
         core::vector3f index = _volume.positionToIndex ( slice.getOrigin() );
         float startx = ( index[ 0 ] - ( slice.size()[ 0 ] * dx[ 0 ] / 2 + slice.size()[ 1 ] * dy[ 0 ] / 2 ) );
         float starty = ( index[ 1 ] - ( slice.size()[ 0 ] * dx[ 1 ] / 2 + slice.size()[ 1 ] * dy[ 1 ] / 2 ) );
         float startz = ( index[ 2 ] - ( slice.size()[ 0 ] * dx[ 2 ] / 2 + slice.size()[ 1 ] * dy[ 2 ] / 2 ) );

         // set up the interpolator
         // if SSE is not supported, use the default interpolator
         if ( core::Equal<Interpolator, InterpolatorTriLinear< VolumeSpatial<float> > >::value && !core::Configuration::instance().isSupportedSSE2() )
         {
            typedef InterpolatorTriLinearDummy< Volume > InterpolatorNoSSE;
            InterpolatorNoSSE interpolator( _volume );
            interpolator.startInterpolation();
            _fill<InterpolatorNoSSE>( startx, starty, startz, dx, dy, interpolator, slice );
            interpolator.endInterpolation();
         } else {
            Interpolator interpolator( _volume );
            interpolator.startInterpolation();
            _fill<Interpolator>( startx, starty, startz, dx, dy, interpolator, slice );
            interpolator.endInterpolation();
         }
      }

   protected:
      template <class Interpolator>
      void _fill( float startx, float starty, float startz, const core::vector3f& dx, const core::vector3f& dy, Interpolator& interpolator, Slice& slice ) const
      {
         Slice::DirectionalIterator it = slice.getIterator( 0, 0 );
         for ( ui32 y = 0; y < slice.size()[ 1 ]; ++y )
         {
            NLL_ALIGN_16 float pos[ 4 ] =
            {
               startx, starty, startz
            };

            it = slice.getIterator( 0, y );
            for ( ui32 x = 0; x < slice.size()[ 0 ]; ++x )
            {
               *it = interpolator( pos );
               pos[ 0 ] += dx[ 0 ];
               pos[ 1 ] += dx[ 1 ];
               pos[ 2 ] += dx[ 2 ];
               it.addx();
            }

            startx += dy[ 0 ];
            starty += dy[ 1 ];
            startz += dy[ 2 ];
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
