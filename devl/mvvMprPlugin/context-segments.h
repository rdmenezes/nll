#ifndef MVV_PLATFORM_CONTEXT_SEGMENTS_H_
# define MVV_PLATFORM_CONTEXT_SEGMENTS_H_

# include "segment.h"

namespace mvv
{
namespace platform
{
   typedef SymbolTyped<Segment>  SymbolSegment;
   typedef ResourceMap<SymbolSegment, RefcountedTyped<Segment> >  ResourceMapSegments;

   class MVVMPRPLUGIN_API ContextSegments : public ContextInstance
   {
   public:
      ResourceMapSegments  segments;
   };
}
}

#endif