#ifndef MVV_DRAWABLE_ENGINE_MPR_H_
# define MVV_DRAWABLE_ENGINE_MPR_H_

# include "engine-mpr.h"

namespace mvv
{
public:
   class DrawableEngineMpr : public EngineMprImpl, public Drawable
   {
   public:
      DrawableEngineMpr( OrderProvider& orderProvider,
                         ResourceVolumes& volumes,
                         ResourceVector3d& origin,
                         ResourceVector3d& vector1,
                         ResourceVector3d& vector2,
                         ResourceVector2d& zoom,
                         ResourceVector2ui& renderingSize,
                         ResourceVolumeIntensities& intensities,
                         ResourceLuts& luts) : EngineMprImpl( orderProvider, volumes, origin, vector1, vector2, zoom, _renderingSize, intensities, luts )
      {
      }

      virtual const Image& draw()
      {
         if ( outFusedMpr.image.sizex() != _renderingSize[ 0 ] || outFusedMpr.image.sizey() != _renderingSize[ 1 ] )
         {
            // we need to rescale for this frame the current MPR as the size is different (asynchrone results)
            // it will be correctly updated later...
            nll::core::rescaleBilinear( outFusedMpr.image, _renderingSize[ 0 ], _renderingSize[ 1 ] );
         }
         return outFusedMpr.image;
      }

      virtual void setImageSize( ui32 sx, ui32 sy )
      {
         _renderingSize[ 0 ] = sx;
         _renderingSize[ 1 ] = sy;
      }


   protected:
      ResourceVector2ui  _renderingSize;
   };
}

#endif