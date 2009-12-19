#ifndef MVV_PLATFORM_MOUSE_EVENT_H_
# define MVV_PLATFORM_MOUSE_EVENT_H_

# include "mvvPlatform.h"
# include <limits>
# include <nll/nll.h>

namespace mvv
{
namespace platform
{
   /**
    @brief Event for mouse actions
    */
   struct MVVPLATFORM_API EventMouse
   {
      /// the current mouse position
      nll::core::vector2ui    mousePosition;

      /// the position where the first left clicked occured
      nll::core::vector2ui    mouseLeftClickedPosition;

      /// the position where first unreleased
      nll::core::vector2ui    mouseLeftReleasedPosition;

      /// set if left button is pressed
      bool                    isMouseLeftButtonPressed;

      /// set if right button is pressed
      bool                    isMouseRightButtonPressed;

      /// true if the right mouse button has just been released
      bool                    isMouseRightButtonJustReleased;

      /// true if the left mouse button has just been released
      bool                    isMouseLeftButtonJustReleased;

      /// true if the right mouse button has just been pressed
      bool                    isMouseRightButtonJustPressed;

      /// true if the left mouse button has just been pressed
      bool                    isMouseLeftButtonJustPressed;
   };
}
}

#endif
