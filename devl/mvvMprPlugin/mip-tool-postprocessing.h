#ifndef MVV_PLATFORM_MIP_TOOL_POSTPROCESSING_H_
# define MVV_PLATFORM_MIP_TOOL_POSTPROCESSING_H_

# include "mip-tool.h"
# include "segment-tool-postprocessing.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief This will handle a postprocessing effect that needs to be done after all other tools
    */
   class MVVMPRPLUGIN_API MipToolPostProcessing : public MipTool
   {
   public:
      MipToolPostProcessing( RefcountedTyped<ToolPostProcessingInterface> processor ) : MipTool( true ), _processor( processor )
      {
      }

      virtual void receive( Mip&, const EventMouse&, const nll::core::vector2ui& )
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

      virtual void updateMip( ResourceSliceuc& slice, Mip& )
      {
         (*_processor).run( slice.getValue() );
      }

   protected:
      RefcountedTyped<ToolPostProcessingInterface> _processor;
   };
}
}

#endif
