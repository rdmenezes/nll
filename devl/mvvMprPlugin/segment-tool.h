#ifndef MVV_PLATFORM_SEGMENT_TOOL_H_
# define MVV_PLATFORM_SEGMENT_TOOL_H_

# include "segment.h"
# include <mvvPlatform/event-mouse-receiver.h>
# include "mvvMprPlugin.h"

namespace mvv
{
namespace platform
{
   // forward declaration
   class Segment;

   class MVVMPRPLUGIN_API SegmentTool : public EventMouseReceiver, public EngineOrder
   {
      typedef std::set< RefcountedTyped<Segment> > StorageSegments;
   public:
      SegmentTool( EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher )
      {}

      // virtual class!!
      virtual ~SegmentTool();

      virtual bool isModifyingMprImage() const
      {
         return false;
      }

      void connect( Segment* segment )
      {
         _segments.insert( segment );
      }

      void disconnect( Segment* segment )
      {
         StorageSegments::iterator it = _segments.find( segment );
         if ( it != _segments.end() )
         {
            _segments.erase( it );
         }
      }

      virtual void receive( Segment& sender, const EventMouse& event ) = 0;

   private:
      // disabled copy
      SegmentTool& operator=( const SegmentTool& );
      SegmentTool( const SegmentTool& );

   protected:
      StorageSegments _segments;
   };
}
}

#endif
