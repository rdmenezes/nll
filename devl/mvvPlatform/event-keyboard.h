#ifndef MVV_PLATFORM_KEYBOARD_EVENT_H_
# define MVV_PLATFORM_KEYBOARD_EVENT_H_

# include "mvvPlatform.h"
# include <limits>
# include <nll/nll.h>

namespace mvv
{
namespace platform
{
   /**
    @brief Event for keyboard actions
    */
   struct MVVPLATFORM_API     EventKeyboard
   {
      enum
      {
         KEY_ESC = 27,
         KEY_ENTER = 13,
         KEY_BACKSPACE = 8,
         KEY_LEFT = 256,
         KEY_UP,
         KEY_RIGHT,
         KEY_DOWN,
         KEY_HOME,
         KEY_END
      };

      EventKeyboard()
      {
         isShift = false;
         isCtrl = false;
         isAlt = false;
      }

      /// the current mouse position
      nll::core::vector2ui    mousePosition;

      /// the current key
      int                     key;

      /// modifiers for 'shift'
      bool                    isShift;

      /// modifiers for 'control'
      bool                    isCtrl;

      /// modifiers for 'alt'
      bool                    isAlt;
   };
}
}

#endif
