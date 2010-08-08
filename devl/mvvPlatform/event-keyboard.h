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
         KEY_SUPR = 127,
         KEY_ESC = 27,
         KEY_ENTER = 13,
         KEY_BACKSPACE = 8,
         KEY_LEFT = 256,
         KEY_UP,
         KEY_RIGHT,
         KEY_DOWN,
         KEY_HOME,
         KEY_END,
         KEY_TAB = '\t',
         KEY_F1 = 0x0001,
         KEY_F2 = 0x0002,
         KEY_F3 = 0x0003,
         KEY_F4 = 0x0004,
         KEY_F5 = 0x0005,
         KEY_F6 = 0x0006,
         KEY_F7 = 0x0007,
         KEY_F8 = 0x0008,
         KEY_F9 = 0x0009
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
