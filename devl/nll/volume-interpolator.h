#ifndef NLL_IMAGING_VOLUME_INTERPOLATOR_H_
# define NLL_IMAGING_VOLUME_INTERPOLATOR_H_

namespace nll
{
namespace imaging
{
   /**
    @brief Nearest neighbour interpolator. (0,0) is the center of the voxel.

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
       @brief (x, y, z) must be an index. It returns background if the point is outside the volume. (0,0) is the center of the voxel.
       */
      double operator()( double x, double y, double z ) const
      {
         const int ix = core::floor( x + 0.5 );
         const int iy = core::floor( y + 0.5 );
         const int iz = core::floor( z + 0.5 );

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
    @brief Trilinear interpolator of a volume. (0,0) is the center of the voxel.

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

       v must remain valid until the end of the calls to the interpolator
       */
      InterpolatorTriLinear( const VolumeType& v ) : _volume( v )
      {}

      /**
       @brief (x, y, z) must be an index. It returns background if the point is outside the volume
       */
      double operator()( double x, double y, double z ) const
      {
         const int ix = core::floor( x );
         const int iy = core::floor( y );
         const int iz = core::floor( z );

         const typename Volume::value_type background = _volume.getBackgroundValue();
         if ( !_volume.inside( ix, iy, iz ) )
            return background;

         const double sx = _volume.size()[ 0 ];
         const double sy = _volume.size()[ 1 ];
         const double sz = _volume.size()[ 2 ];

         const double dx = fabs( x - ix );
         const double dy = fabs( y - iy );
         const double dz = fabs( z - iz );

         VolumeType::ConstDirectionalIterator it = _volume.getIterator( ix, iy, iz );
         VolumeType::ConstDirectionalIterator itz( it );
         itz.addz();

         const double v000 = *it;
         double v001, v010, v011, v100, v110, v101, v111;

         const int ixn = ix + 1;
         const int iyn = iy + 1;
         const int izn = iz + 1;

         if ( ixn < sx && iyn < sy && izn < sz )
         {
            v100 = it.pickx();
            v101 = itz.pickx();
            v010 = it.picky();
            v011 = itz.picky();
            v001 = it.pickz();
            v110 = *it.addx().addy();
            v111 = it.pickz();
         } else return background;

         //
         // optimized version for
         //   v100 = it.pickx();
         //   v101 = itz.pickx();
         //   v010 = it.picky();
         //   v011 = itz.picky();
         //   v001 = it.pickz();
         //   v110 = *it.addx().addy();
         //   v111 = it.pickz();
         //

         const double i1 = v000 * ( 1 - dz ) + v001 * dz;
         const double i2 = v010 * ( 1 - dz ) + v011 * dz;
         const double j1 = v100 * ( 1 - dz ) + v101 * dz;
         const double j2 = v110 * ( 1 - dz ) + v111 * dz;

         const double w1 = i1 * ( 1 - dy ) + i2 * dy;
         const double w2 = j1 * ( 1 - dy ) + j2 * dy;

         return w1 * ( 1 - dx ) + w2 * dx;
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