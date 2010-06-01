#ifndef NLL_UTILITIES_CREATE_FEATURES_H_
# define NLL_UTILITIES_CREATE_FEATURES_H_

# include <nll/nll.h>
# include "globals.h"
# include "compute-barycentre.h"
# include "extract-mpr.h"

namespace nll
{
namespace utility
{
   typedef nll::imaging::VolumeSpatial<float>         Volumef;

   /**
    @brief Simply extract fron & side view of the volume
    */
   inline void extractMpr( const Volumef& volume, core::Image<ui8>& mpr1, core::Image<ui8>& mpr2 )
   {
      typedef nll::imaging::LookUpTransformWindowingRGB  Lut;
      Lut lut( REGION_DETECTION_BARYCENTRE_LUT_MIN, REGION_DETECTION_BARYCENTRE_LUT_MAX, 256 );
      lut.createGreyscale();

      const core::vector3f centre = computeBarycentre( volume, lut );
      const core::vector3f centreVoxel = volume.positionToIndex( centre );
      core::vector3f mprCentre1 = volume.indexToPosition( core::vector3f( static_cast<f32>( volume.size()[ 0 ] ) / 2,
                                                                          centreVoxel[ 1 ],
                                                                          static_cast<f32>( volume.size()[ 2 ] ) / 2 ) );

      core::vector3f mprCentre2 = volume.indexToPosition( core::vector3f( centreVoxel[ 0 ],
                                                                          static_cast<f32>( volume.size()[ 1 ] ) / 2,
                                                                          static_cast<f32>( volume.size()[ 2 ] ) / 2 ) );

      const core::vector3f vector_x( volume.getPst()( 0, 0 ) / volume.getSpacing()[ 0 ],
                                     volume.getPst()( 1, 0 ) / volume.getSpacing()[ 0 ],
                                     volume.getPst()( 2, 0 ) / volume.getSpacing()[ 0 ]);
      const core::vector3f vector_y( volume.getPst()( 0, 1 ) / volume.getSpacing()[ 1 ],
                                     volume.getPst()( 1, 1 ) / volume.getSpacing()[ 1 ],
                                     volume.getPst()( 2, 1 ) / volume.getSpacing()[ 1 ]);
      const core::vector3f vector_z( volume.getPst()( 0, 2 ) / volume.getSpacing()[ 2 ],
                                     volume.getPst()( 1, 2 ) / volume.getSpacing()[ 2 ],
                                     volume.getPst()( 2, 2 ) / volume.getSpacing()[ 2 ]);

      core::Image<ui8> mprxz, mpryz;
      ui32 sx1 = static_cast<ui32>( volume.size()[ 0 ] * volume.getSpacing()[ 0 ] / volume.getSpacing()[ 2 ] );
      ui32 sx2 = static_cast<ui32>( volume.size()[ 1 ] * volume.getSpacing()[ 1 ] / volume.getSpacing()[ 2 ] );
      mpr1 = extractMpr( volume,
                         core::vector2ui( sx1, volume.size()[ 2 ] ),
                         vector_x,
                         vector_z,
                         mprCentre1,
                         core::vector2f( volume.getSpacing()[ 2 ], volume.getSpacing()[ 2 ] ),
                         lut );
      mpr2 = extractMpr( volume,
                         core::vector2ui( sx2, volume.size()[ 2 ] ),
                         vector_y,
                         vector_z,
                         mprCentre2,
                         core::vector2f( volume.getSpacing()[ 2 ], volume.getSpacing()[ 2 ] ),
                         lut );
   }

   /**
    @brief Normalize the image to a REGION_DETECTION_SOURCE_IMG_X*REGION_DETECTION_SOURCE_IMG_Y image
    */
   inline std::vector< core::Image<ui8> > normalizeImage( const Volumef& volume )
   {
      core::Image<ui8> mprxz, mpryz;

      extractMpr( volume, mprxz, mpryz );

      // crop th x part of the image
      cropVertical( mprxz, 0.05f, 20 );
      cropVertical( mpryz, 0.05f, 20 );


      // resample the image, so that all images have exactly the same dimension
      core::rescaleBilinear( mprxz, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y );
      core::rescaleBilinear( mpryz, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y );

      return core::make_vector< core::Image<ui8> >( mprxz, mpryz );
   }

   /**
    @brief Create a single buffer for feature
    */
   inline core::Buffer1D<double> createFeatures( const Volumef& volume )
   {
      std::vector< core::Image<ui8> > res = normalizeImage( volume );
      core::Buffer1D<double> input( REGION_DETECTION_SOURCE_IMG_X * REGION_DETECTION_SOURCE_IMG_Y * 2 );
      for ( ui32 y = 0; y < REGION_DETECTION_SOURCE_IMG_Y; ++y )
      {
         for ( ui32 x = 0; x < REGION_DETECTION_SOURCE_IMG_X; ++x )
         {
            input[ x + y * REGION_DETECTION_SOURCE_IMG_X ] = res[ 0 ]( x, y, 0 );
            input[ x + y * REGION_DETECTION_SOURCE_IMG_X +
                   REGION_DETECTION_SOURCE_IMG_X * REGION_DETECTION_SOURCE_IMG_Y ] = res[ 1 ]( x, y, 0 );
         }
      }
      return input;
   }
                                          
}
}

#endif