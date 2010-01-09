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
    @brief
    */
   class MVVMPRPLUGIN_API SegmentTool : public EventMouseReceiver, public EngineOrder, public LinkableDouble< Segment*, SegmentTool* >
   {
   public:
      // output of one tool is connected to the input of the following output
      ResourceSliceuc   inputSegment;
      ResourceSliceuc   outputSegment;

   public:
      SegmentTool( ResourceSliceuc inputSegment, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher )
      {
         EngineOrder::connect( inputSegment );
         handler.connect( *this );

         // by default, copy the input to the output as it is not modyfing the segment. Else a new resource should be
         // linked to the output
         outputSegment = inputSegment;
      }

      // virtual class!!
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
       */
      virtual int priority() const
      {
         return 0;
      }

      virtual bool _run()
      {
         if ( _canModify )
         {
            // copy the current segment
         }

         //do the changes on the output
         updateSegment();
         return true;
      }

      virtual void updateSegment() = 0;

      /**
       @brief if true, the event will not be propagated to other tools
       */
      virtual bool interceptEvent() const
      {
         return false;
      }

      virtual void connect( Segment* segment );

      virtual void disconnect( Segment* segment );

      virtual void receive( Segment& sender, const EventMouse& event ) = 0;

   private:
      // disabled copy
      SegmentTool& operator=( const SegmentTool& );
      SegmentTool( const SegmentTool& );

   protected:
      bool     _canModify;
   };
}
}

#endif
