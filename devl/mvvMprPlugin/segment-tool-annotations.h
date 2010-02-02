#ifndef MVV_PLATFORM_SEGMENT_TOOL_ANNOTATIONS_H_
# define MVV_PLATFORM_SEGMENT_TOOL_ANNOTATIONS_H_

# include "segment-tool-pointer.h"
# include "annotation.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Displays a set of points on the MPR. It is assumed points coordinate are in source space.
    */
   class MVVMPRPLUGIN_API SegmentToolAnnotations : public SegmentTool, public Engine
   {
   public:
      SegmentToolAnnotations( ResourceAnnotations annotations, EngineHandler& handler ) : SegmentTool( true ), Engine( handler ), _annotations( annotations )
      {
         _annotations.connect( this );
      }

      virtual f32 priority() const
      {
         return 200;
      }

      virtual bool _run()
      {
         // if a point is modified, then we need to force the connected MPR to be refreshed
         refreshConnectedSegments();
         return true;
      }


      virtual void updateSegment( ResourceSliceuc s, Segment&  )
      {
         for ( ResourceAnnotations::Iterator it = _annotations.begin(); it != _annotations.end(); ++it )
         {
            (**it).updateSegment( s );
         }
      }

      virtual void receive( Segment& , const EventMouse& , const nll::core::vector2ui&  )
      {
         // nothing yet...
      }

   private:
      void refreshConnectedSegments()
      {
         for ( SegmentTool::LinkStorage::iterator it = SegmentTool::_links.begin(); it != SegmentTool::_links.end(); ++it )
         {
            (*it)->refreshTools();
         }
      }

   private:
      ResourceAnnotations     _annotations;
   };
}
}

#endif
