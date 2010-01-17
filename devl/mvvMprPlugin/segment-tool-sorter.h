#ifndef MVV_PLATFORM_SEGMENT_TOOL_SORTER_H_
# define MVV_PLATFORM_SEGMENT_TOOL_SORTER_H_

# include "segment-tool.h"

namespace mvv
{
namespace platform
{
   class SegmentTool;

   /**
    @ingroup platform
    @brief Sort the tools according to a specific order
    */
   class MVVMPRPLUGIN_API SegmentToolSorter
   {
   public:
      virtual void sort( std::vector<SegmentTool*>& tools ) = 0;
   };

   /**
    @ingroup platform
    @brief Sort the tools according to the priority of the tools, from the highest to the lowest
    */
   class MVVMPRPLUGIN_API SegmentToolSorterPriorityQueue : public SegmentToolSorter
   {
      // from smallest to biggest
      struct Compare
      {
         bool operator()( SegmentTool* s1, SegmentTool* s2 )
         {
            return s1->priority() < s2->priority();
         }
      };

   public:
      virtual void sort( std::vector<SegmentTool*>& tools )
      {
         Compare compare;
         std::sort( tools.begin(), tools.end(), compare );
      }
   };
}
}

#endif
