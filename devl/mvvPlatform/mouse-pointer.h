#ifndef MVV_PLATFORM_MOUSE_POINTER_H_
# define MVV_PLATFORM_MOUSE_POINTER_H_

# include "mvvPlatform.h"
# include "event-mouse.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Defines an interface to display mouse pointer graphics
    */
   class MVVPLATFORM_API MousePointer
   {
   public:
      enum MouseGraphic
      {
         MOUSE_DESTROY   = 0x0003,
         MOUSE_NORMAL    = 0x0001,
         MOUSE_CYCLE     = 0x0005,
         MOUSE_WAIT      = 0x0007,
         MOUSE_CROSSHAIR = 0x0009
      };

      MousePointer()
      {}

      virtual ~MousePointer()
      {}

      /**
       @brief set the mouse pointer
       */
      virtual void setMousePointer( MouseGraphic graphic ) = 0;

      /**
       @brief get the mouse pointer
       */
      virtual ui32 getMousePointer() const = 0;

   private:
      // copy disabled
      MousePointer& operator=( const MousePointer& );
      MousePointer( const MousePointer& );
   };

   /**
    @brief Defines a callback for running an user action for a specific mouse event
    */
   class MVVPLATFORM_API MouseSegmentCallback
   {
   public:
      MouseSegmentCallback()
      {}

      virtual ~MouseSegmentCallback()
      {}

      virtual void run( const nll::imaging::Slice<nll::ui8>& s, const EventMouse& event, const nll::core::vector2ui& windowOrigin ) = 0;
   };
}
}

#endif
