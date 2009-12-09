#ifndef NLL_IMAGING_MAXIMUM_INTENSITY_PROJECTION_H_
# define NLL_IMAGING_MAXIMUM_INTENSITY_PROJECTION_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Compute the maximum intensity projection of a volume
    @note volume must be a type of spatial volume
    */
   template <class Volume>
   class MaximumIntensityProjection
   {
   public:
      typedef typename core::If<typename Volume::value_type, float, core::IsFloatingType<typename Volume::value_type>::value >::type value_type;

   public:
      MaximumIntensityProjection( const Volume& volume ) : _volume( volume ), _boundingBox( core::vector3f( 0, 0, 0 ),
                                                                                            core::vector3f( static_cast<float>( volume.size()[ 0 ] ),
                                                                                                            static_cast<float>( volume.size()[ 1 ] ),
                                                                                                            static_cast<float>( volume.size()[ 2 ] ) ) )
      {
      }

      /**
       @brief Compute a maximum intensity projection on the slice.
       @param slice defines the attributs of the surface we want to project
       @param direction determines the projection direction
       @note if the volume is in front/back it will still be displayed as if it was in front
       */
      template <class VolumeInterpolator>
      void computeMip( Slice<value_type>& slice, const core::vector3f& direction )
      {
         typedef Slice<typename Volume::value_type>   SliceType;

         ensure( slice.size()[ 2 ] == 1, "only single valued slice is handled" );
         ensure( slice.size()[ 0 ] && slice.size()[ 1 ], "slice must not be empty" );
         if ( !_volume.getSize()[ 0 ] || !_volume.getSize()[ 1 ] || !_volume.getSize()[ 2 ] )
         {
            for ( SliceType::iterator it = slice.begin(); it != slice.end(); ++it )
               *it = _volume.getBackgroundValue();
         }


         // normalized vector director
         core::vector3f dirInStandardSpace = direction;
         float interpolationStepInMinimeter = std::min( std::min( _volume.getSpacing()[ 0 ], _volume.getSpacing()[ 1 ] ), _volume.getSpacing()[ 2 ] ) * 2;
         dirInStandardSpace /= static_cast<f32>( dirInStandardSpace.norm2() );
         dirInStandardSpace *= interpolationStepInMinimeter;

         // get the starting point of the slice in volume coordinate
         core::vector3f bottomLeftSliceInWorld = slice.sliceToWorldCoordinate( core::vector2f( -static_cast<float>( slice.size()[ 0 ] ) / 2,
                                                                                               -static_cast<float>( slice.size()[ 1 ] ) / 2 ) );
         // translate to volume coordinates
         const core::vector3f startIndex = _volume.positionToIndex( bottomLeftSliceInWorld );
         const core::vector3f dir = _volume.positionToIndex( dirInStandardSpace ) -  _volume.positionToIndex( core::vector3f( 0, 0, 0 ) );
         const core::vector3f SliceOringIndex = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 0, 0 ) ) );
         const core::vector3f dx = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 1, 0 ) ) ) - SliceOringIndex;
         const core::vector3f dy = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 0, 1 ) ) ) - SliceOringIndex;


         core::vector3f intersection1;
         core::vector3f intersection2;
         core::vector3f startLine = startIndex;

         SliceType::DirectionalIterator itLine = slice.getIterator( 0, 0 );
         SliceType::DirectionalIterator itLineNext = itLine;
         SliceType::DirectionalIterator itLineEnd = slice.getIterator( 0, slice.size()[ 1 ] );
         itLineNext.addy();

         VolumeInterpolator interpolator( _volume );
         interpolator.startInterpolation();
         const float background = _volume.getBackgroundValue();
         const float dirNorm = static_cast<float>( dir.norm2() );
         const core::vector3f dirOppose = core::vector3f( -dir[ 0 ], -dir[ 1 ], -dir[ 2 ] );
         for ( ; itLine != itLineEnd; )
         {
            SliceType::DirectionalIterator itPixels = itLine;
            core::vector3f position = startLine;
            for ( ; itPixels != itLineNext; ++itPixels )
            {
               if ( _boundingBox.getIntersection( position, dir, intersection1, intersection2 ) )
               {
                  const core::vector3f directionIntersection = intersection2 - intersection1;
                  float d = static_cast<float>( directionIntersection.norm2() ) / dirNorm;
                  const float max = _getMaxValue( intersection1, ( dir.dot( directionIntersection ) > 0 ) ? dir : dirOppose, static_cast<ui32>( std::ceil( d ) ), interpolator );
                  *itPixels = max;
               } else {
                  *itPixels = background;
               }

               position += dx;
            }

            itLineNext.addy();
            itLine.addy();
            startLine += dy;
         }
         interpolator.endInterpolation();
      }
   
   /**
    @brief Creates a slice from angle
    */
   template <class VolumeInterpolator>
   Slice<value_type> getAutoOrientedMip( float anglexRadian, ui32 sizex, ui32 sizey, f32 spacingx, f32 spacingy )
   {
      const core::vector3f centerPosition = _volume.indexToPosition( core::vector3f( static_cast<float>( _volume.getSize()[ 0 ] ) / 2, -1e4, static_cast<float>( _volume.getSize()[ 2 ] ) / 2 ) );
      const core::vector3f centerVolume = _volume.indexToPosition( core::vector3f( static_cast<float>( _volume.getSize()[ 0 ] ) / 2,
                                                                                   static_cast<float>( _volume.getSize()[ 1 ] ) / 2,
                                                                                   static_cast<float>( _volume.getSize()[ 2 ] ) / 2 ) );

      core::Matrix<float> tfm;
      core::matrix4x4RotationZ( tfm, anglexRadian );
      tfm = core::mul( _volume.getInversedPst(), tfm );

      // we need to remove the scaling...
      for ( ui32 t = 0; t < 4; ++t )
      {
         float dd = sqrt( core::sqr( tfm( 0, t ) ) + core::sqr( tfm( 1, t ) ) + core::sqr( tfm( 2, t ) ) );
         tfm( 0, t ) /= dd;
         tfm( 1, t ) /= dd;
         tfm( 2, t ) /= dd;
      }
      

      const core::vector3f centerPositionRotated = core::mul4Rot( tfm, centerPosition );
      const core::vector3f zero = _volume.indexToPosition( core::vector3f( 0, 0, 0 ) );
      const core::vector3f dx = _volume.indexToPosition( core::vector3f( 1, 0, 0 ) ) - zero;
      const core::vector3f dy = _volume.indexToPosition( core::vector3f( 0, 0, 1 ) ) - zero;

      const core::vector3f dxRotated = core::mul4Rot( tfm, dx );
      const core::vector3f dyRotated = core::mul4Rot( tfm, dy );

      Slice<value_type> slice( core::vector3ui( sizex, sizey, 1 ),
                               dxRotated,
                               dyRotated,
                               centerPositionRotated,
                               core::vector2f( spacingx, spacingy ) );
      computeMip<VolumeInterpolator>( slice, core::cross( dxRotated, dyRotated ) );
      return slice;
   }

   private:
      // non copyable
      MaximumIntensityProjection& operator=( const MaximumIntensityProjection& );

   private:
      template <class VolumeInterpolator>
      value_type _getMaxValue( const core::vector3f& start, const core::vector3f& dir, ui32 nbSteps, const VolumeInterpolator& interpolator )
      {
         NLL_ALIGN_16 float pos[ 4 ] =
         {
            start[ 0 ], start[ 1 ], start[ 2 ], 0
         };

         value_type max = _volume.getBackgroundValue();
         for ( ui32 n = 0; n < nbSteps; ++n )
         {
            value_type val = interpolator( pos );
            max = std::max( max, val );

            pos[ 0 ] += dir[ 0 ];
            pos[ 1 ] += dir[ 1 ];
            pos[ 2 ] += dir[ 2 ];
         }

         return max;
      }

   private:
      const Volume&        _volume;
      core::GeometryBox    _boundingBox;
   };
}
}

#endif