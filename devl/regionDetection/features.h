#ifndef NLL_DETECT_FEATURES_H_
# define NLL_DETECT_FEATURES_H_

# include "types.h"
# include "globals.h"

namespace nll
{
namespace detect
{
   /**
    @brief Extract an image in XY plane, normalized and resampled
    */
   inline core::Image<ui8> extractSlice( const Volume& v, double zposition_mm )
   {
      typedef imaging::Mpr< Volume, imaging::InterpolatorTriLinear<Volume> >  Mpr;

      // extract a centered slice, should have the same size than number of voxels in x/y
      Lut lut( REGION_DETECTION_BARYCENTRE_LUT_MIN, REGION_DETECTION_BARYCENTRE_LUT_MAX, 256 );
      lut.createGreyscale();

      core::vector3f vx( v.getPst()( 0, 0 ), 
                         v.getPst()( 1, 0 ),
                         v.getPst()( 2, 0 ) );
      core::vector3f vy( v.getPst()( 0, 1 ), 
                         v.getPst()( 1, 1 ),
                         v.getPst()( 2, 1 ) );
      assert( v.getSpacing()[ 0 ] == v.getSpacing()[ 1 ] ); // we are expecting same spacing

      f32 spacing = std::min( v.getSpacing()[ 0 ], v.getSpacing()[ 1 ] );
      core::vector3f position = v.indexToPosition( core::vector3f( static_cast<f32>( v.size()[ 0 ] ) / 2,
                                                                   static_cast<f32>( v.size()[ 1 ] ) / 2,
                                                                   static_cast<f32>( v.size()[ 2 ] ) / 2 ) );
      position[ 2 ] = zposition_mm;
      Slice slice( core::vector3ui( v.size()[ 0 ] - 1, v.size()[ 1 ] - 1, 3 ),   // we extract a smaller slice => rounding errors, which can create one full row of non zero voxel, depending on the lut...
                   vx,
                   vy,
                   position,
                   core::vector2f( spacing, spacing ) );
      Mpr mpr( v );
      mpr.getSlice( slice );

      // convert to greyscale image
      core::Image<ui8> sliceTfm( v.size()[ 0 ], v.size()[ 1 ], 1 );
      for ( ui32 y = 0; y < slice.size()[ 1 ]; ++y )
      {
         for ( ui32 x = 0; x < slice.size()[ 0 ]; ++x )
         {
            const float* value = lut.transform( slice( x, y, 0 ) );

            ui8* pos = sliceTfm.point( x, y );
            pos[ 0 ] = static_cast<ui8>( NLL_BOUND( value[ 0 ], 0, 255 ) );
         }
      }

      // center the image
      cropVertical( sliceTfm, 0.02f, 20 );
      cropHorizontal( sliceTfm, 0.06f, 20 );
      return sliceTfm;
   }
}
}

#endif