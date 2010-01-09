#include "segment-tool.h"
#include "segment.h"

namespace mvv
{
namespace platform
{
   SegmentTool::~SegmentTool()
   {
      removeConnections();
   }

   void SegmentTool::connect( Segment* segment )
   {
      addSimpleLink( segment );
      segment->addSimpleLink( this );
   }

   void SegmentTool::disconnect( Segment* segment )
   {
      eraseSimpleLink( segment );
      segment->eraseSimpleLink( this );
   }
}
}