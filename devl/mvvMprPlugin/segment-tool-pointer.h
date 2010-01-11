#ifndef MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_
# define MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_

# include "segment-tool-pointer.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API SegmentToolPointer : public SegmentTool
   {
   public:
      virtual void updateSegment( ResourceSliceuc segment, Segment&  )
      {
         if ( segment.getValue().getStorage().sizex() < 5 )
            return;

         std::cout << "segment - computed" << std::endl;
         ResourceSliceuc::value_type::Storage::DirectionalIterator  it = segment.getValue().getIterator( 5, 5 );
         for ( int n = 0; n < 15; ++n, ++it )
         {
            *it = 255;
         }
      }

      virtual void receive( Segment& , const EventMouse&  )
      {
      }

   protected:
      ResourceVector3f  _position;
   };
}
}

#endif