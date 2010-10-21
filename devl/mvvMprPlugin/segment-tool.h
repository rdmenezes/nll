#ifndef MVV_PLATFORM_SEGMENT_TOOL_H_
# define MVV_PLATFORM_SEGMENT_TOOL_H_

//# include "segment.h"
# include <mvvPlatform/event-mouse-receiver.h>
# include <mvvPlatform/linkable.h>
# include <mvvPlatform/engine-order.h>
# include <mvvPlatform/resource-typedef.h>
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
    @note if ResourceBool::notify() is used it will force the connected segments to be redrawn
    */
   class MVVMPRPLUGIN_API SegmentTool : public LinkableDouble< Segment*, SegmentTool* >, public ResourceBool
   {
   public:
      SegmentTool( bool canModifyImage ) : _canModify( canModifyImage )
      {
      }

      virtual ~SegmentTool();

      /**
       @brief Returns true is this tool is modifying the segment (add layouts...). Else modification
              on the MPR are not allowed and <code>updateSegment</code> won't be called
       */
      bool isModifyingMprImage() const
      {
         return _canModify;
      }

      /**
       @brief Returns true is this tool need to save the current segment: when the tool is updated,
              and if no other resources have change, then we don't need to recompute all the segment,
              just start at the point we saved previously
       */
      virtual bool isSavingMprImage() const
      {
         return false;
      }

      /**
       @brief Tools are sorted according to their priority from highest to lowest
              they are displayed from lowest->highest
              received events from highest->lowest
       */
      virtual f32 priority() const
      {
         return 0;
      }

      /**
       @brief action need to be run when the input slice changed.
       */
      virtual void updateSegment( ResourceSliceuc& segment, Segment& holder ) = 0;

      /**
       @param sender the segment where the signal originate's from
       @param event the event
       @paran windowOrigin the origin of the layout so we can compute the actual position of the mouse relative to the layout
       */
      virtual void receive( Segment& sender, const EventMouse& event, const nll::core::vector2ui& windowOrigin ) = 0;

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
