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
   class MVVMPRPLUGIN_API SegmentTool : public EventMouseReceiver, public EngineOrder, public LinkableDouble< Segment*, SegmentTool* >
   {
   public:
      // output of one tool is connected to the input of the following output
      ResourceSliceuc   inputSegment;     /// must never be modified directly
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
              they are displayed from lowest->highest
              received events from highest->lowest
       */
      virtual int priority() const
      {
         return 0;
      }

      virtual bool _run()
      {
         // update the geometry in case it is different
         inputSegment.getValue().setGeometry( outputSegment.getValue().getAxisX(),
                                              outputSegment.getValue().getAxisY(),
                                              outputSegment.getValue().getOrigin(),
                                              outputSegment.getValue().getSpacing() );
         if ( _canModify )
         {
            if ( inputSegment.getValue().getStorage().size() != outputSegment.getValue().getStorage().size() )
            {
               outputSegment.getValue().getStorage().clone( inputSegment.getValue().getStorage() );
            }

            // copy the content
            ResourceSliceuc::value_type::Storage::const_iterator in = inputSegment.getValue().getStorage().begin();
            for ( ResourceSliceuc::value_type::Storage::iterator it = outputSegment.getValue().getStorage().begin(); it != outputSegment.getValue().getStorage().end(); ++it, ++in )
            {
               *it = *in;
            }

            // notify the changes
            outputSegment.notify();
         }

         //do the changes on the output
         updateSegment();
         return true;
      }

      /**
       @brief action need to be run when the input slice changed. only <code>inputSegment</code> must not be modified
       */
      virtual void updateSegment() = 0;

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
      bool     _canModify;
   };
}
}

#endif
