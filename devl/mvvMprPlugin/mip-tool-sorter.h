#ifndef MVV_PLATFORM_MIP_TOOL_SORTER_H_
# define MVV_PLATFORM_MIP_TOOL_SORTER_H_

# include "mip-tool.h"

namespace mvv
{
namespace platform
{
   class MipTool;

   /**
    @ingroup platform
    @brief Sort the tools according to a specific order
    */
   class MVVMPRPLUGIN_API MipToolSorter
   {
   public:
      virtual void sort( std::vector<MipTool*>& tools ) = 0;
   };

   /**
    @ingroup platform
    @brief Sort the tools according to the priority of the tools, from the highest to the lowest
    */
   class MVVMPRPLUGIN_API MipToolSorterPriorityQueue : public MipToolSorter
   {
      // from smallest to biggest
      struct Compare
      {
         bool operator()( MipTool* s1, MipTool* s2 )
         {
            return s1->priority() < s2->priority();
         }
      };

   public:
      virtual void sort( std::vector<MipTool*>& tools )
      {
         Compare compare;
         std::sort( tools.begin(), tools.end(), compare );
      }
   };
}
}

#endif
