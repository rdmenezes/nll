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
