#ifndef NLL_UTILITIES_COMPUTE_BARYCENTRE_H_
# define NLL_UTILITIES_COMPUTE_BARYCENTRE_H_

# include <nll/nll.h>

namespace nll
{
namespace utility
{
   /**
    @brief Computes the barycentre of the volume, using this specific LUT
    */
   template <class Volume, class Lut>
   core::vector3f computeBarycentre( const Volume& vol, const Lut& lut )
   {
      // by default, it is the centre of the volume
      core::VolumeGeometry geometry( vol.getPst() );

      // compute the barycentre
      float nb = 0;
      const ui32 lutSize = lut.getSize();
      core::vector3f tmp( 0, 0, 0 );
      for ( ui32 z = 0; z < vol.size()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < vol.size()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < vol.size()[ 0 ]; ++x )
            {
               const float value = vol( x, y, z );
               const float computed = lut.transform( value )[ 0 ];
               const float ratio = computed / lutSize;
               tmp += core::vector3f( static_cast<f32>( x ),
                                      static_cast<f32>( y ), 
                                      static_cast<f32>( z ) ) * ratio;
               nb += ratio;
            }
         }
      }
      return geometry.indexToPosition( tmp / nb );
   }
}
}
#endif