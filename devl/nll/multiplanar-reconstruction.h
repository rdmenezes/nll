#ifndef NLL_IMAGING_MULTIPLANAR_RECONSTRUCTION_H_
# define NLL_IMAGING_MULTIPLANAR_RECONSTRUCTION_H_

namespace nll
{
namespace imaging
{
   namespace impl
   {
      /**
       @brief apply a rotation given a transformation (rotation+scale only)
              The matrix must be a 4x4 transformation matrix defined by the volume.
              
              Compute Mv using only the rotational part of M.
       */
      template <class T, class Mapper>
      core::vector3d mul3Rot( const core::Matrix<T, Mapper>& m, const core::vector3d& v )
      {
         assert( m.sizex() == 4 && m.sizey() == 4 );
         return core::vector3d( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + v[ 2 ] * m( 0, 2 ),
                                v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + v[ 2 ] * m( 1, 2 ),
                                v[ 0 ] * m( 2, 0 ) + v[ 1 ] * m( 2, 1 ) + v[ 2 ] * m( 2, 2 ) );
      }
   }

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
      typedef core::Image<double, core::IndexMapperRowMajorFlat2DColorRGBnMask> Slice;

   public:
      /**
       @brief set the size of the plane to be reconstructed in voxels
       */
      Mpr( const VolumeType& volume, f64 sxInVoxels, f64 syInVoxels ) :
         _volume( volume ), _voxelsx( sxInVoxels ), _voxelsy( syInVoxels )
      {}

      /**
       @brief Compute the slice according to a position and 2 vectors and a size factor.
              The volume's spacing is used to compute the correct MPR.
       @param point in mm (patient coordinate). It will be the center of the new slice.
       @param ax x-axis of the plane
       @param ay y-axis of the plane
       @param zoomFactor zoomFactor used toreconstruct the slice
       */
      Slice getSlice( const core::vector3d& point, const core::vector3d& ax, const core::vector3d& ay, const core::vector2d zoomFactor = core::vector2d( 1, 1 ) ) const
      {
         assert( zoomFactor[ 0 ] > 0 && zoomFactor[ 1 ] > 0 );

         // the slice has a speficied size, it needs to be resampled afterward if necesary
         Slice slice( static_cast<ui32>( core::round( _voxelsx ) ),
                      static_cast<ui32>( core::round( _voxelsy ) ),
                      1,
                      false );

         // compute the slopes. First rotate the vectors so we are in the same coordinate system
         core::vector3d dx = impl::mul3Rot( _volume.getInversedPst(), ax );
         const double c1 = dx.norm2() * zoomFactor[ 0 ];
         dx[ 0 ] = dx[ 0 ] / ( c1 * _volume.getSpacing()[ 0 ] );
         dx[ 1 ] = dx[ 1 ] / ( c1 * _volume.getSpacing()[ 1 ] );
         dx[ 2 ] = dx[ 2 ] / ( c1 * _volume.getSpacing()[ 2 ] );

         core::vector3d dy = impl::mul3Rot( _volume.getInversedPst(), ay );
         const double c2 = dy.norm2() * zoomFactor[ 1 ];
         dy[ 0 ] = dy[ 0 ] / ( c2 * _volume.getSpacing()[ 0 ] );
         dy[ 1 ] = dy[ 1 ] / ( c2 * _volume.getSpacing()[ 1 ] );
         dy[ 2 ] = dy[ 2 ] / ( c2 * _volume.getSpacing()[ 2 ] );

         // set up the interpolator
         Interpolator interpolator( _volume );

         // reconstruct the slice
         core::vector3d index = _volume.positionToIndex ( point );
         double startx = index[ 0 ] - ( _voxelsx * dx[ 0 ] / 2 + _voxelsy * dy[ 0 ] / 2 );
         double starty = index[ 1 ] - ( _voxelsx * dx[ 1 ] / 2 + _voxelsy * dy[ 1 ] / 2 );
         double startz = index[ 2 ] - ( _voxelsx * dx[ 2 ] / 2 + _voxelsy * dy[ 2 ] / 2 );
         for ( ui32 y = 0; y < _voxelsy; ++y )
         {
            double px = startx;
            double py = starty;
            double pz = startz;
            for ( ui32 x = 0; x < _voxelsx; ++x )
            {
               slice( x, y, 0 ) = interpolator( px, py, pz );
               px += dx[ 0 ];
               py += dx[ 1 ];
               pz += dx[ 2 ];
            }

            startx += dy[ 0 ];
            starty += dy[ 1 ];
            startz += dy[ 2 ];
         }
         return slice;
      }

   protected:
      Mpr& operator=( const Mpr& );

   protected:
      const VolumeType& _volume;
      f64               _voxelsx;
      f64               _voxelsy;
   };
}
}

#endif
