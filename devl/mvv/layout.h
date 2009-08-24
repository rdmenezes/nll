#ifndef MVV_LAYOUT_H_
# define MVV_LAYOUT_H_

# include "types.h"
# include <nll/nll.h>

namespace mvv
{
   class Pane
   {
   protected:
      typedef nll::core::Image<ui8> Image;

   public:
      /**
       @brief Construct a Pane
       @brief size the size in pixel of the pane
       @brief origin the origin in pixel of the pane. (0, 0) is top left of the screen
       */
      Pane( const nll::core::vector2ui& size, const nll::core::vector2ui& origin ) : _size( size ), _origin( origin )
      {}

      virtual ~Pane()
      {}

      /**
       @brief Returns the size of the pane in pixel
       */
      const nll::core::vector2ui& getSize() const
      {
         return _size;
      }

      /**
       @brief Returns the origin of the pane in pixel
       */
      const nll::core::vector2ui& getOrigin() const
      {
         return _origin;
      }

      /**
       @brief Set the current pane pixel buffer
       */
      void setPixel( ui32 x, ui32 y, ui8 value )
      {
      }

   protected:
      nll::core::vector2ui size;
      nll::core::vector2ui origin;
   };

   class Layout
   {
   public:
      Layout( ui32 sx, ui32 sy );
   };
}

#endif
