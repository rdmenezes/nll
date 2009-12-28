#include "segment-tool.h"

namespace mvv
{
namespace platform
{
   SegmentTool::~SegmentTool()
   {
      for ( StorageSegments::iterator it = _segments.begin(); it != _segments.end(); ++it )
      {
         (**it)._remove( this );
      }
   }
}
}