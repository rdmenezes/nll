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
       @param tfm an affine that transform source->target, assuming we have a target volume. We can see the transformation as
              transforming the source geometry and then do as if there was no transformation
       @note Typical use case is, we have a source and target volumes, with a registration matrix tfm
             source->target. Assume we want to add a translation in target space, like in Ex 1,
             we actually need to inverse this transformation!

             Ex 1: slice origin(0, 0, 0) with a translation(1,-1,0)
             |00010     |00000
             |00200 =>  |00001
             |00000     |00020
       */
      void getSlice( Slice& slice, const TransformationAffine& tfm ) const
      {
         assert( slice.getSpacing()[ 0 ] > 0 && slice.getSpacing()[ 1 ] > 0 );

         // compute the slopes. First rotate the vectors so we are in the same coordinate system
         Transformation::Matrix transformation = tfm.getAffineMatrix() * _volume.getInvertedPst();

         core::vector3f dx = core::mul4Rot( transformation, slice.getAxisX() );
         const float c1 = (float)dx.norm2() / slice.getSpacing()[ 0 ];
         dx[ 0 ] = dx[ 0 ] / ( c1 * _volume.getSpacing()[ 0 ] );
         dx[ 1 ] = dx[ 1 ] / ( c1 * _volume.getSpacing()[ 1 ] );
         dx[ 2 ] = dx[ 2 ] / ( c1 * _volume.getSpacing()[ 2 ] );

         core::vector3f dy = core::mul4Rot( transformation, slice.getAxisY() );
         const float c2 = (float)dy.norm2() / slice.getSpacing()[ 1 ];
         dy[ 0 ] = dy[ 0 ] / ( c2 * _volume.getSpacing()[ 0 ] );
         dy[ 1 ] = dy[ 1 ] / ( c2 * _volume.getSpacing()[ 1 ] );
         dy[ 2 ] = dy[ 2 ] / ( c2 * _volume.getSpacing()[ 2 ] );

         core::vector3f tr( tfm.getAffineMatrix()( 0, 3 ) ,
                            tfm.getAffineMatrix()( 1, 3 ),
                            tfm.getAffineMatrix()( 2, 3 ) );

         Transformation::Matrix tfmRot;
         tfmRot.clone( tfm.getAffineMatrix() );
         tfmRot( 0, 3 ) = 0;
         tfmRot( 1, 3 ) = 0;
         tfmRot( 2, 3 ) = 0;

         core::vector3f index = transf4( tfmRot * _volume.getInvertedPst(), slice.getOrigin() ) + 
                                transf4( tfmRot, core::vector3f( _volume.positionToIndex( tr ) - _volume.positionToIndex( core::vector3f( 0, 0, 0 ) ) ) );

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
         typename Slice::DirectionalIterator it = slice.getIterator( 0, 0 );
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
