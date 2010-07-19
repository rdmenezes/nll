#ifndef MVV_PLATFORM_EVENT_KEYBOARD_RECEIVER_H_
# define MVV_PLATFORM_EVENT_KEYBOARD_RECEIVER_H_

# include "mvvPlatform.h"
# include "event-keyboard.h"

namespace mvv
{
namespace platform
{
   class MVVPLATFORM_API EventKeyboardReceiver
   {
   public:
      virtual void receive( const EventKeyboard& event ) = 0;
   };
}
}

#endif