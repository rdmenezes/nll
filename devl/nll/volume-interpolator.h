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
      {
         iix = -1000;
         iiy = -1000;
         iiz = -1000;
      }

      /**
       @brief (x, y, z) must be an index. It returns background if the point is outside the volume
       */
      double operator()( double x, double y, double z ) const
      {
         const int ix = core::floor( x );
         const int iy = core::floor( y );
         const int iz = core::floor( z );

         // 0 <-> size - 1 as we need an extra sample for linear interpolation
         const typename Volume::value_type background = _volume.getBackgroundValue();
         if ( ix < 0 || ix + 1 >= _volume.size()[ 0 ]  ||
              iy < 0 || iy + 1 >= _volume.size()[ 1 ]  ||
              iz < 0 || iz + 1 >= _volume.size()[ 2 ] )
         {
            return background;
         }


         const double dx = fabs( x - ix );
         const double dy = fabs( y - iy );
         const double dz = fabs( z - iz );

         // Often in the same neighbourhood, we are using the same voxel, but at a slightly different
         // position, so we are caching the previous result, and reuse it if necessary
         if ( ix != iix || iy != iiy || iz != iiz )
         {
            // update the position to possibly use the cached values next iteration
            iix = ix;
            iiy = iy;
            iiz = iz;

            // case we are not using the cached values
            typename VolumeType::ConstDirectionalIterator it = _volume.getIterator( ix, iy, iz );
            typename VolumeType::ConstDirectionalIterator itz( it );
            itz.addz();

            v000 = *it;

            const int ixn = ix + 1;
            const int iyn = iy + 1;
            const int izn = iz + 1;

            v100 = it.pickx();
            v101 = itz.pickx();
            v010 = it.picky();
            v011 = itz.picky();
            v001 = it.pickz();
            v110 = *it.addx().addy();
            v111 = it.pickz();            
         }

         const double i1 = v000 * ( 1 - dz ) + v001 * dz;
         const double i2 = v010 * ( 1 - dz ) + v011 * dz;
         const double j1 = v100 * ( 1 - dz ) + v101 * dz;
         const double j2 = v110 * ( 1 - dz ) + v111 * dz;

         const double w1 = i1 * ( 1 - dy ) + i2 * dy;
         const double w2 = j1 * ( 1 - dy ) + j2 * dy;

         const double value = w1 * ( 1 - dx ) + w2 * dx;
         return value;
      }

   protected:
      /// non copiable
      InterpolatorTriLinear& operator=( const InterpolatorTriLinear& );

   protected:
      const VolumeType& _volume;

      mutable double v000;
      mutable double v001, v010, v011, v100, v110, v101, v111;
      mutable int iix, iiy, iiz;
   };
}
}

#endif
