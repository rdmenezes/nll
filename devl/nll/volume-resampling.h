#ifndef NLL_IMAGING_VOLUME_RESAMPLE_H_
# define NLL_IMAGING_VOLUME_RESAMPLE_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Resample a target volume to an arbitrary source geometry
    @param target the volume that will be resampled
    @param source the volume into wich it will be resampled.
    @param tfm a transformation defined from source to target

    The source must already be allocated.
    */
   template <class T, class Storage, class Interpolator>
   void resampleVolume( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm )
   {
      typedef VolumeSpatial<T, Storage>   VolumeType;
      typedef core::Matrix<f32>  Matrix;

      if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
           !source.getSize()[ 0 ] || !source.getSize()[ 1 ] || !source.getSize()[ 2 ] )
      {
         throw std::exception( "invalid volume" );
      }

      //
      // Compute the transformation source index->source position->transformation in source position to target->to target index
      // we don't really care about the order of composition as rotation and scalling are associative
      //
      Matrix transformation = target.getInvertedPst() * source.getPst() * tfm.getAffineMatrix();
      const core::vector3f dx( transformation( 0, 0 ),
                               transformation( 1, 0 ),
                               transformation( 2, 0 ) );
      const core::vector3f dy( transformation( 0, 1 ),
                               transformation( 1, 1 ),
                               transformation( 2, 1 ) );
      const core::vector3f dz( transformation( 0, 2 ),
                               transformation( 1, 2 ),
                               transformation( 2, 2 ) );

      core::vector3f tr( tfm.getAffineMatrix()( 0, 3 ),
                         tfm.getAffineMatrix()( 1, 3 ),
                         tfm.getAffineMatrix()( 2, 3 ) );
      Matrix tfmRot;
      tfmRot.clone( tfm.getAffineMatrix() );
      tfmRot( 0, 3 ) = 0;
      tfmRot( 1, 3 ) = 0;
      tfmRot( 2, 3 ) = 0;

      //
      // Now convert the source origin to index in target space and add the translation of the affine transformation 
      //

      core::vector3f originInTarget = transf4( tfmRot * target.getInvertedPst(), source.getOrigin() ) + 
                                      transf4( tfmRot, core::vector3f( target.positionToIndex( tr ) - target.positionToIndex( core::vector3f( 0, 0, 0 ) ) ) );


      //const core::vector3f originInTarget = transf4( target.getInvertedPst() * tfmRot, source.getOrigin() ) + target.positionToIndex( tr ) - target.positionToIndex( core::vector3f( 0, 0, 0 ) );
      //const core::vector3f center = target.positionToIndex( source.getOrigin() + tr );
      //const core::vector3f originInTarget = core::mul4Rot( tfm.getAffineMatrix(), center );
      //const core::vector3f originInTarget = target.positionToIndex( source.getOrigin() );
      
      //const core::vector3f originInTarget = transf4( target.getInvertedPst() * source.getPst() * tfm.getAffineMatrix(), core::vector3f( 0, 0, 0 ) );
      //const core::vector3f originInTarget = transf4( target.getInvertedPst() * tfm.getAffineMatrix() * source.getPst(), core::vector3f( 0, 0, 0 ) );

      Interpolator interpolator( target );
      typename VolumeType::DirectionalIterator  sliceIt = source.getIterator( 0, 0, 0 );
      core::vector3f slicePosSrc = originInTarget;
      
      interpolator.startInterpolation();
      for ( ui32 z = 0; z < source.getSize()[ 2 ]; ++z )
      {
         typename VolumeType::DirectionalIterator  lineIt = sliceIt;
         core::vector3f linePosSrc = slicePosSrc;
         for ( ui32 y = 0; y < source.getSize()[ 1 ]; ++y )
         {
            typename VolumeType::DirectionalIterator  voxelIt = lineIt;
            
            NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
            { 
               linePosSrc[ 0 ],
               linePosSrc[ 1 ],
               linePosSrc[ 2 ],
               0
            };

            for ( ui32 x = 0; x < source.getSize()[ 0 ]; ++x )
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

   template <class T, class Storage, class Interpolator>
   void resampleVolume( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source )
   {
      typedef core::Matrix<f32>  Matrix;

      Matrix id = core::identityMatrix<Matrix>( 4 );
      resampleVolume<T, Storage, Interpolator>( target, source, TransformationAffine( id ) );
   }

   /**
    @ingroup imaging
    @brief Resample a target volume to an arbitrary source geometry. Use a default trilinear interpolation for resampling.

    The source must already be allocated.
    */
   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source )
   {
      resampleVolume<T, Storage, InterpolatorTriLinear< VolumeSpatial<T, Storage> > >( target, source );
   }

   template <class T, class Storage>
   void resampleVolumeTrilinear( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm )
   {
      resampleVolume<T, Storage, InterpolatorTriLinear< VolumeSpatial<T, Storage> > >( target, source, tfm );
   }

   /**
    @ingroup imaging
    @brief Resample a target volume to an arbitrary source geometry. Use a default nearest neighbour interpolation for resampling.

    The source must already be allocated.
    */
   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source )
   {
      resampleVolume<T, Storage, InterpolatorNearestNeighbour< VolumeSpatial<T, Storage> > >( target, source );
   }

   template <class T, class Storage>
   void resampleVolumeNearestNeighbour( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm )
   {
      resampleVolume<T, Storage, InterpolatorNearestNeighbour< VolumeSpatial<T, Storage> > >( target, source, tfm );
   }
}
}

#endif
