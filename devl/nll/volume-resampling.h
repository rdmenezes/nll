#ifndef NLL_IMAGING_VOLUME_RESAMPLE_H_
# define NLL_IMAGING_VOLUME_RESAMPLE_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Resample a source volume to an arbitrary target geometry

    The target must already be allocated.
    */
   template <class T, class Storage, class Interpolator>
   void resampleVolume( const VolumeSpatial<T, Storage>& source, VolumeSpatial<T, Storage>& target )
   {
      typedef VolumeSpatial<T, Storage>   VolumeType;

      if ( !source.getSize()[ 0 ] || !source.getSize()[ 1 ] || !source.getSize()[ 2 ] ||
           !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] )
      {
         throw std::exception( "invalid volume" );
      }

      // finite difference to find the base in source space
      const core::vector3f originInSource = source.positionToIndex( target.getOrigin() );
      const core::vector3f dx = source.positionToIndex( target.indexToPosition( core::vector3f( 1, 0, 0 ) ) ) - originInSource;
      const core::vector3f dy = source.positionToIndex( target.indexToPosition( core::vector3f( 0, 1, 0 ) ) ) - originInSource;
      const core::vector3f dz = source.positionToIndex( target.indexToPosition( core::vector3f( 0, 0, 1 ) ) ) - originInSource;

      Interpolator interpolator( source );
      typename VolumeType::DirectionalIterator  sliceIt = target.getIterator( 0, 0, 0 );
      core::vector3f slicePosSrc = originInSource;
      
      interpolator.startInterpolation();
      for ( ui32 z = 0; z < target.getSize()[ 2 ]; ++z )
      {
         typename VolumeType::DirectionalIterator  lineIt = sliceIt;
         core::vector3f linePosSrc = slicePosSrc;
         for ( ui32 y = 0; y < target.getSize()[ 1 ]; ++y )
         {
            typename VolumeType::DirectionalIterator  voxelIt = lineIt;
            
            NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
            { 
               linePosSrc[ 0 ],
               linePosSrc[ 1 ],
               linePosSrc[ 2 ],
               0
            };

            for ( ui32 x = 0; x < target.getSize()[ 0 ]; ++x )
            {
               *voxelIt = interpolator( voxelPosSrc );

               voxelPosSrc[ 0 ] += dx[ 0 ];
               voxelPosSrc[ 1 ] += dx[ 1 ];
               voxelPosSrc[ 2 ] += dx[ 2 ];
               voxelIt.addx();
            }
            linePosSrc += dy;
            lineIt.addy();
         }
         slicePosSrc += dz;
         sliceIt.addz();
      }
      interpolator.endInterpolation();
   }

   /**
    @ingroup imaging
    @brief Resample a source volume to an arbitrary target geometry. Use a default trilinear interpolation for resampling.

    The target must already be allocated.
    */
   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& source, VolumeSpatial<T, Storage>& target )
   {
      resampleVolume<T, Storage, InterpolatorTriLinear< VolumeSpatial<T, Storage> > >( source, target );
   }

   /**
    @ingroup imaging
    @brief Resample a source volume to an arbitrary target geometry. Use a default nearest neighbour interpolation for resampling.

    The target must already be allocated.
    */
   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& source, VolumeSpatial<T, Storage>& target )
   {
      resampleVolume<T, Storage, InterpolatorNearestNeighbour< VolumeSpatial<T, Storage> > >( source, target );
   }
}
}

#endif
