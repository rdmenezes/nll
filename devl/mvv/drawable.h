#ifndef MVV_DRAWABLE_H_
# define MVV_DRAWABLE_H_

# include "types.h"
# include <nll/nll.h>

namespace mvv
{
   /**
    @ingroup mvv
    @brief A drawable class
    */
   class Drawable
   {
   public:
      typedef nll::core::Image<ui8> Image;

   public:
      /**
       @brief compute an image of size (sx, sy)
       */
      virtual const Image& draw() = 0;

      /**
       @brief tells the size of the image that needs to be computed in pixel
       */
      virtual void setImageSize( ui32 sx, ui32 sy ) = 0;
   };
}

#endif
