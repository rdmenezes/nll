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
         const typename Volume::value_type background = _volume.getBackgroundValue();
         if ( !_volume.inside( x, y, z ) )
            return background;

         const int ix = int( x );
         const int iy = int( y );
         const int iz = int( z );

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
         }
         else
         {
            if ( iyn < sy )
            {
               v010 = it.picky();
            }
            else
            {
               v010 = background;
            }
            if ( ixn < sx )
            {
               v100 = it.pickx();
               v110 = it.addx().picky();
            } else {
               v100 = background;
               v110 = background;
            }

            if ( izn < sz )
            {
               v001 = *itz;
               if ( iyn < sy )
               {
                  v011 = itz.picky();
               } else {
                  v011 = background;
               }
               if ( ixn < sx )
               {
                  v101 = itz.pickx();
                  v111 = itz.addx().picky();
               } else {
                  v101 = background;
                  v111 = background;
               }
            } else {
               v001 = background;
               v011 = background;
               v101 = background;
               v111 = background;
            }
         }

         //
         // optimized version for
         //   const double v000 = _getValue( ix,     iy,     iz );
         //   const double v001 = _getValue( ix,     iy,     iz + 1 );
         //   const double v010 = _getValue( ix,     iy + 1, iz );
         //   const double v100 = _getValue( ix + 1, iy,     iz );
         //   const double v011 = _getValue( ix,     iy + 1, iz + 1 );
         //   const double v110 = _getValue( ix + 1, iy + 1, iz );
         //   const double v101 = _getValue( ix + 1, iy,     iz + 1 );
         //   const double v111 = _getValue( ix + 1, iy + 1, iz + 1 );
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