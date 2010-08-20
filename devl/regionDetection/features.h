#ifndef NLL_DETECT_FEATURES_H_
# define NLL_DETECT_FEATURES_H_

# include "types.h"
# include "globals.h"
# include "compute-barycentre.h"
# include <nll/nll.h>

namespace nll
{
namespace detect
{

   inline core::Image<ui8> extractSliceFull( const Volume& v, ui32 slicenb )
   {
      typedef imaging::Mpr< Volume, imaging::InterpolatorTriLinear<Volume> >  Mpr;

      // extract a centered slice, should have the same size than number of voxels in x/y
      Lut lut( REGION_DETECTION_BARYCENTRE_LUT_MIN, REGION_DETECTION_BARYCENTRE_LUT_MAX, 256 );
      lut.createGreyscale();

      core::vector3f vx( v.getPst()( 0, 0 ), 
                         v.getPst()( 1, 0 ),
                         v.getPst()( 2, 0 ) );
      core::vector3f vy( -v.getPst()( 0, 1 ), 
                         -v.getPst()( 1, 1 ),
                         -v.getPst()( 2, 1 ) );
      assert( v.getSpacing()[ 0 ] == v.getSpacing()[ 1 ] ); // we are expecting same spacing

      f32 spacing = std::min( v.getSpacing()[ 0 ], v.getSpacing()[ 1 ] );
      core::vector3f position = v.indexToPosition( core::vector3f( static_cast<f32>( v.size()[ 0 ] ) / 2,
                                                                   static_cast<f32>( v.size()[ 1 ] ) / 2,
                                                                   static_cast<f32>( slicenb ) ) );
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
      core::rescaleBilinear( sliceTfm, sliceTfm.sizex() / 3, sliceTfm.sizey() / 3 );
      return sliceTfm;
   }

   /**
    @brief Extract an image in XY plane, normalized and resampled
    */
   inline core::Image<ui8> extractSlice( const Volume& v, double zposition_mm )
   {
      typedef imaging::Mpr< Volume, imaging::InterpolatorTriLinear<Volume> >  Mpr;
      std::cout << "pos=" << zposition_mm << std::endl;

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
      position[ 2 ] = static_cast<f32>( zposition_mm );
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

      /*
      // we first rescale, so that we remove the same amount of voxel... // we need to resample at least twice the size, else artefacts will appear
      core::rescaleBilinear( sliceTfm, REGION_DETECTION_SOURCE_IMG_X * 2, REGION_DETECTION_SOURCE_IMG_Y * 2 );
      */
      typedef nll::algorithm::Labelize<nll::ui8,
                                       nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                                       nll::algorithm::RegionPixelSpecific<nll::ui8> >  Labelize;

      ui8 black[] = { 0 };
      Labelize::DifferentPixel different( 1, black, 10 );

      Labelize l( different );
      Labelize::ComponentsInfo info = l.run( sliceTfm, true );
      
      int max = 0;
      int maxIndex = -1;
      for ( ui32 n = 0; n < info.components.size(); ++n )
      {
         if ( info.components[ n ].size > max &&
              sliceTfm( info.components[ n ].posx, info.components[ n ].posy, 0 ) > 0 )
         {
            maxIndex = n;
            max = info.components[ n ].size;
         }
      }

      if ( maxIndex >= 0 )
      {
         sliceTfm = extract( sliceTfm, info.labels, info.components[ maxIndex ].id );
      }

      // center the image
      cropVertical( sliceTfm, 0.06f, 2 );
      cropHorizontal( sliceTfm, 0.03f, 2 );

      //centerImage( sliceTfm );

      // normalize the size
      //core::rescaleFast( sliceTfm, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y );
      core::rescaleBilinear( sliceTfm, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y );
      return sliceTfm;
   }

   inline core::Image<ui8> extractXZ( const Volume& v )
   {
      typedef imaging::Mpr< Volume, imaging::InterpolatorTriLinear<Volume> >  Mpr;

      // extract a centered slice, should have the same size than number of voxels in x/y
      Lut lut( REGION_DETECTION_BARYCENTRE_LUT_MIN, REGION_DETECTION_BARYCENTRE_LUT_MAX, 256 );
      lut.createGreyscale();

      core::vector3f vx( v.getPst()( 0, 0 ), 
                         v.getPst()( 1, 0 ),
                         v.getPst()( 2, 0 ) );
      core::vector3f vy( v.getPst()( 0, 2 ), 
                         v.getPst()( 1, 2 ),
                         v.getPst()( 2, 2 ) );
      assert( v.getSpacing()[ 0 ] == v.getSpacing()[ 1 ] ); // we are expecting same spacing

      core::vector3f barycentre = nll::detect::computeBarycentre( v, lut );
      core::vector3f position = v.indexToPosition( core::vector3f( static_cast<f32>( v.size()[ 0 ] ) / 2,
                                                                   static_cast<f32>( v.size()[ 1 ] ) / 2,
                                                                   static_cast<f32>( v.size()[ 2 ] ) / 2 ) );
      ui32 sx1 = static_cast<ui32>( v.size()[ 0 ] * v.getSpacing()[ 0 ] / v.getSpacing()[ 2 ] );
      position[ 1 ] = barycentre[ 1 ];
      Slice slice( core::vector3ui( sx1, v.size()[ 2 ], 3 ),   // we extract a smaller slice => rounding errors, which can create one full row of non zero voxel, depending on the lut...
                   vx,
                   vy,
                   position,
                   core::vector2f( v.getSpacing()[ 2 ], v.getSpacing()[ 2 ] ) );
      Mpr mpr( v );
      mpr.getSlice( slice );

      // convert to greyscale image
      core::Image<ui8> sliceTfm( sx1, v.size()[ 2 ], 1 );
      for ( ui32 y = 0; y < slice.size()[ 1 ]; ++y )
      {
         for ( ui32 x = 0; x < slice.size()[ 0 ]; ++x )
         {
            const float* value = lut.transform( slice( x, y, 0 ) );

            ui8* pos = sliceTfm.point( x, y );
            pos[ 0 ] = static_cast<ui8>( NLL_BOUND( value[ 0 ], 0, 255 ) );
         }
      }
      return sliceTfm;
   }


}
}

#endif