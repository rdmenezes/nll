#ifndef NLL_IMAGING_VOLUME_INTERPOLATOR_H_
# define NLL_IMAGING_VOLUME_INTERPOLATOR_H_

namespace nll
{
namespace imaging
{
   /**
    @brief Nearest neighbour interpolator. The center of a voxel is considered the top left corner of a point.

    Volume must be of a volume type.
    */
   template <class Volume>
   class InterpolatorNearestNeighbour
   {
   public:
      typedef Volume   VolumeType;

   public:
      /**
       @brief Construct an interpolator for the volume v. 

       v must remain valid until the end of the calls to the interpolator
       */
      InterpolatorNearestNeighbour( const VolumeType& v ) : _volume( v )
      {}

      /**
       @brief (x, y, z) must be an index. It returns background if the point is outside the volume. The center of 
       a voxel is considered the top left corner of a point.
       */
      double operator()( double x, double y, double z ) const
      {
         const int ix = (int)core::round<double>( x );
         const int iy = (int)core::round<double>( y );
         const int iz = (int)core::round<double>( z );

         if ( _volume.inside( ix, iy, iz ) )
            return _volume( ix, iy, iz );
         return _volume.getBackgroundValue();
      }

   protected:
      /// non copiable
      InterpolatorNearestNeighbour& operator=( const InterpolatorNearestNeighbour& );

   protected:
      const VolumeType& _volume;
   };


   /**
    @Trilinear interpolator of a volume. The center of a voxel is considered as the top left corner of a point.

    Volume must be of a volume type or derived.

    See http://en.wikipedia.org/wiki/Trilinear_interpolation for equations
    */
   template <class Volume>
   class InterpolatorTriLinear
   {
   public:
      typedef Volume   VolumeType;

   public:
      /**
       @brief Construct an interpolator for the volume v. 

       Beware, the top left corner is considered as the center!

       v must remain valid until the end of the calls to the interpolator
       */
      InterpolatorTriLinear( const VolumeType& v ) : _volume( v )
      {}

      /**
       @brief (x, y, z) must be an index. It returns background if the point is outside the volume
       */
      double operator()( double x, double y, double z ) const
      {
         if ( !_volume.inside( x, y, z ) )
            _volume.getBackgroundValue();

         const int ix = int( x );
         const int iy = int( y );
         const int iz = int( z );

         const double dx = fabs( x - ix );
         const double dy = fabs( y - iy );
         const double dz = fabs( z - iz );

         const double v000 = _getValue( ix,     iy,     iz );
         const double v001 = _getValue( ix,     iy,     iz + 1 );
         const double v010 = _getValue( ix,     iy + 1, iz );
         const double v100 = _getValue( ix + 1, iy,     iz );
         const double v011 = _getValue( ix,     iy + 1, iz + 1 );
         const double v110 = _getValue( ix + 1, iy + 1, iz );
         const double v101 = _getValue( ix + 1, iy,     iz + 1 );
         const double v111 = _getValue( ix + 1, iy + 1, iz + 1 );

         const double i1 = v000 * ( 1 - dz ) + v001 * dz;
         const double i2 = v010 * ( 1 - dz ) + v011 * dz;
         const double j1 = v100 * ( 1 - dz ) + v101 * dz;
         const double j2 = v110 * ( 1 - dz ) + v111 * dz;

         const double w1 = i1 * ( 1 - dy ) + i2 * dy;
         const double w2 = j1 * ( 1 - dy ) + j2 * dy;

         return w1 * ( 1 - dx ) + w2 * dx;
      }

   protected:
      inline double _getValue( ui32 x, ui32 y, ui32 z ) const
      {
         if ( _volume.inside( x, y, z ) )
            return _volume( x, y, z );
         return _volume.getBackgroundValue();
      }

   protected:
      /// non copiable
      InterpolatorTriLinear& operator=( const InterpolatorTriLinear& );

   protected:
      const VolumeType& _volume;
   };
}
}

#endif