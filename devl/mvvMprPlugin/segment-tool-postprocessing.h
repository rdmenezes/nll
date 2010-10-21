#ifndef MVV_PLATFORM_SEGMENT_TOOL_POSTPROCESSING_H_
# define MVV_PLATFORM_SEGMENT_TOOL_POSTPROCESSING_H_

# include "segment-tool.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API ToolPostProcessingInterface
   {
   public:
      /**
       @brief run a method doing some posprocessing.
       @note only pixel data are allowed to be changed, all other properties should not!
       */
      virtual void run( Sliceuc& s ) = 0;

      virtual ~ToolPostProcessingInterface(){}
   };

   /**
    @ingroup platform
    @brief This will handle a postprocessing effect that needs to be done after all other tools
    */
   class MVVMPRPLUGIN_API SegmentToolPostProcessing : public SegmentTool
   {
   public:
      SegmentToolPostProcessing( RefcountedTyped<ToolPostProcessingInterface> processor ) : SegmentTool( true ), _processor( processor )
      {
      }

      virtual void receive( Segment&, const EventMouse&, const nll::core::vector2ui& )
      {
         // do nothing
      }

      virtual f32 priority() const
      {
         return 9999;
      }

      virtual bool isSavingMprImage() const
      {
         return false;
      }

      virtual void updateSegment( ResourceSliceuc& slice, Segment& )
      {
         (*_processor).run( slice.getValue() );
      }

   protected:
      RefcountedTyped<ToolPostProcessingInterface> _processor;
   };
}
}

#endif
