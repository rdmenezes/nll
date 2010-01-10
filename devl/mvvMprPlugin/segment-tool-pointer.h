#ifndef MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_
# define MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_

# include "segment-tool-pointer.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API SegmentToolPointer : public SegmentTool
   {
   public:
      SegmentToolPointer( ResourceSliceuc inputSegment, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : SegmentTool( inputSegment, handler, provider, dispatcher )
      {
      }

      virtual void updateSegment()
      {
         
      }

      virtual void receive( Segment& sender, const EventMouse& event )
      {
      }

   protected:
      ResourceVector3f  _position;
   };
}
}

#endif