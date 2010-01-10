#ifndef MVV_PLATFORM_SEGMENT_TOOL_H_
# define MVV_PLATFORM_SEGMENT_TOOL_H_

//# include "segment.h"
# include <mvvPlatform/event-mouse-receiver.h>
# include <mvvPlatform/linkable.h>
# include <mvvPlatform/engine-order.h>
# include "mvvMprPlugin.h"
# include "types.h"

namespace mvv
{
namespace platform
{
   // forward declaration
   class Segment;

   /**
    @ingroup platform
    @brief Tools that can be plugged on a segment
    @note don't forget to call notify() on outputSegment if the slice is modified in updateSegment
    */
   class MVVMPRPLUGIN_API SegmentTool : public LinkableDouble< Segment*, SegmentTool* >
   {
   public:
      SegmentTool()
      {}

      virtual ~SegmentTool();

      /**
       @brief Returns true is this tool is modifying the segment (add layouts...), means we need to make a copy of
              the Segment, so in case the tool depends on another resource and this resource is notified we just update
              the saved segment
       */
      bool isModifyingMprImage() const
      {
         return _canModify;
      }

      void setModifyingMprImage( bool modify )
      {
         _canModify = modify;
      }

      /**
       @brief Tools are sorted according to their priority from highest to lowest
              they are displayed from lowest->highest
              received events from highest->lowest
       */
      virtual int priority() const
      {
         return 0;
      }

      /**
       @brief action need to be run when the input slice changed.
       */
      virtual void updateSegment( ResourceSliceuc segment, Segment& holder ) = 0;

      virtual void receive( Segment& sender, const EventMouse& event ) = 0;

      /**
       @brief if true, the event will not be propagated to other tools
       */
      virtual bool interceptEvent() const
      {
         return false;
      }

      virtual void connect( Segment* segment );

      virtual void disconnect( Segment* segment );

   private:
      // disabled copy
      SegmentTool& operator=( const SegmentTool& );
      SegmentTool( const SegmentTool& );

   protected:
      bool                    _canModify;
   };
}
}

#endif
