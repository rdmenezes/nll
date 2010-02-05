#ifndef MVV_PLATFORM_MIP_TOOL_POINTER_H_
# define MVV_PLATFORM_MIP_TOOL_POINTER_H_

# include "mvvMprPlugin.h"
# include "types.h"
# include "mip-tool.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Allows to manipulate the MIP
    */
   class MVVMPRPLUGIN_API MipToolPointer : public MipTool
   {
   public:
      MipToolPointer( ui32 mipFps = 7 ) : MipTool( false ), _mipFps( mipFps )
      {}

      virtual void updateMip( ResourceSliceuc, Mip& )
      {
      }

      virtual void receive( Mip& sender, const EventMouse& e, const nll::core::vector2ui& )
      {
         if ( e.isMouseRightButtonJustPressed )
         {
            _initialOrientation = e.mousePosition;
            sender.fps.setValue( 0 );
         } else if ( e.isMouseLeftButtonJustPressed )
         {
            float time = static_cast<float>( clock() - _lastLeftClick ) / CLOCKS_PER_SEC;
            if ( time < 0.2 )
            {
               sender.fps.setValue( 7 );
            }
            _lastLeftClick = clock();
         }



         if ( !e.isMouseLeftButtonPressed && e.isMouseRightButtonPressed )
         {
            float newAngle = sender.anglex.getValue() + ( static_cast<float>( e.mousePosition[ 0 ] ) - static_cast<float>( _initialOrientation[ 0 ] ) ) / 100.0f;
            while ( newAngle >= nll::core::PI * 2 )
            {
               newAngle -= static_cast<float>( nll::core::PI * 2 );
            }
            sender.anglex.setValue( newAngle );
            _initialOrientation = e.mousePosition;
         }
         
      }

   private:
      nll::core::vector2ui  _initialOrientation;
      ui32                  _mipFps;   // frame per second
      time_t                _lastLeftClick;
   };
}
}

#endif
