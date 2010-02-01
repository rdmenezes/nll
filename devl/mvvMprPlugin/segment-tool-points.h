#ifndef MVV_PLATFORM_SEGMENT_TOOL_POINTS_H_
# define MVV_PLATFORM_SEGMENT_TOOL_POINTS_H_

# include "segment-tool-pointer.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Displays a set of points on the MPR. It is assumed points coordinate are in source space.
    */
   class MVVMPRPLUGIN_API SegmentToolPoints : public SegmentTool, public Engine
   {
   public:
      SegmentToolPoints( ResourceVector3fs points, EngineHandler& handler, nll::core::vector3uc color = nll::core::vector3uc( 255, 0, 0 ) ) : SegmentTool( true ), Engine( handler ), _points( points ), _color( color )
      {
         _points.connect( this );
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


      virtual void updateSegment( ResourceSliceuc , Segment&  )
      {

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
      ResourceVector3fs       _points;
      nll::core::vector3uc    _color;
   };
}
}

#endif
