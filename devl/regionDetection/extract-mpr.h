#ifndef NLL_UTILITIES_EXTRACT_MPR_H_
# define NLL_UTILITIES_EXTRACT_MPR_H_

# include <nll/nll.h>

namespace nll
{
namespace utility
{
   /**
    @brief Extract a MPR with a size of the 
    */
   template <class Volume, class Lut>
   core::Image<ui8> extractMpr( const Volume& volume,
                                const core::vector2ui& size,
                                const core::vector3f& v1,
                                const core::vector3f& v2,
                                const core::vector3f& centre,
                                const core::vector2f& spacing,
                                const Lut& lut )
   {
      core::Image<ui8> sliceTfm( size[ 0 ], size[ 1 ], 3 );
      ensure( lut.getNbComponents() == 3, "must be a RGB transform" );

      typedef imaging::Mpr< Volume, imaging::InterpolatorTriLinear<Volume> >  Mpr;
      typedef typename Mpr::Slice                                             Slice;

      Slice slice( core::vector3ui( size[ 0 ], size[ 1 ], 3 ),
                   v1,
                   v2,
                   centre,
                   spacing );
      Mpr mpr( volume );
      mpr.getSlice( slice );
      for ( ui32 y = 0; y < size[ 1 ]; ++y )
      {
         for ( ui32 x = 0; x < size[ 0 ]; ++x )
         {
            const float* value = lut.transform( slice( x, y, 0 ) );

            ui8* pos = sliceTfm.point( x, y );
            pos[ 0 ] = static_cast<ui8>( NLL_BOUND( value[ 0 ], 0, 255 ) );
            pos[ 1 ] = static_cast<ui8>( NLL_BOUND( value[ 1 ], 0, 255 ) );
            pos[ 2 ] = static_cast<ui8>( NLL_BOUND( value[ 2 ], 0, 255 ) );
         }
      }
      return sliceTfm;
   }
}
}

#endif