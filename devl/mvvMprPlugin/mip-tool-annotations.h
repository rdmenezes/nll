#ifndef MVV_PLATFORM_LAYOUT_TOOL_ANNOTATIONS_H_
# define MVV_PLATFORM_LAYOUT_TOOL_ANNOTATIONS_H_

# include "mvvMprPlugin.h"
# include "types.h"
# include "annotation.h"
# include "mip-tool.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Displays a set of points on the MPR. It is assumed points coordinate are in source space.
    */
   class MVVMPRPLUGIN_API MipToolAnnotations : public MipTool, public Engine
   {
   public:
      MipToolAnnotations( ResourceAnnotations& annotations, EngineHandler& handler ) : MipTool( true ), Engine( handler ), _annotations( annotations )
      {
         _annotations.connect( this );
      }

      virtual f32 priority() const
      {
         return 200;
      }

      virtual bool _run()
      {
         // if a point is modified, then we need to force the connected MPR to be refreshed
         std::cout << "refresh mip!" << std::endl;
         refreshConnectedMip();
         return true;
      }


      virtual void updateMip( ResourceSliceuc& s, Mip&  )
      {
         for ( ResourceAnnotations::Iterator it = _annotations.begin(); it != _annotations.end(); ++it )
         {
            const_cast<Annotation&>( (**it) ).updateSegment( s );
         }
      }

      virtual void receive( Mip& , const EventMouse& , const nll::core::vector2ui&  )
      {
         // nothing yet...
      }

   private:
      void refreshConnectedMip();

   private:
      ResourceAnnotations     _annotations;
   };
}
}

#endif
