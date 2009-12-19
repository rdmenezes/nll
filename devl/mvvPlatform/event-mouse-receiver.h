#ifndef MVV_PLATFORM_EVENT_MOUSE_RECEIVER_H_
# define MVV_PLATFORM_EVENT_MOUSE_RECEIVER_H_

# include "mvvPlatform.h"
# include "event-mouse.h"

namespace mvv
{
namespace platform
{
   class MVVPLATFORM_API EventMouseReceiver
   {
   public:
      virtual void receive( const EventMouse& event ) = 0;
   };
}
}

#endif