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
      _addSimpleLink( segment );
      segment->_addSimpleLink( this );
   }

   void SegmentTool::disconnect( Segment* segment )
   {
      _eraseSimpleLink( segment );
      segment->_eraseSimpleLink( this );
   }
}
}