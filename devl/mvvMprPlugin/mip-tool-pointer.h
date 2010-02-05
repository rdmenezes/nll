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
         if ( e.isMouseLeftButtonJustPressed )
         {
            _initialOrientation = e.mousePosition;
            float time = static_cast<float>( clock() - _lastLeftClick ) / CLOCKS_PER_SEC;
            if ( time < 0.2 )
            {
               sender.fps.setValue( 7 );
            } else sender.fps.setValue( 0 );
            _lastLeftClick = clock();
         }

         if ( e.isMouseLeftButtonPressed && !e.isMouseRightButtonPressed )
         {
            float newAngle = sender.anglex.getValue() + ( static_cast<float>( e.mousePosition[ 0 ] ) - static_cast<float>( _initialOrientation[ 0 ] ) ) / 100.0f;
            while ( newAngle >= nll::core::PI * 2 )
            {
               newAngle -= static_cast<float>( nll::core::PI * 2 );
            }
            sender.anglex.setValue( newAngle );
         } else if ( e.isMouseLeftButtonPressed && e.isMouseRightButtonPressed )
         {
            // update the zoom factors of all the attached segments
            nll::core::vector2i diffMouse( - (int)e.mousePosition[ 0 ] + (int)_initialOrientation[ 0 ],
                                           - (int)e.mousePosition[ 1 ] + (int)_initialOrientation[ 1 ] );
            float d = static_cast<float>( fabs( (float)diffMouse[ 1 ] ) );
            float zoom;
            if ( (int)e.mousePosition[ 1 ] >= _initialOrientation[ 1 ] )
            {
               zoom = sender.zoom.getValue() * (float)( 1 + d / 100 );
            } else {
               zoom = sender.zoom.getValue() / (float)( 1 + d / 100 );
            }
            sender.zoom.setValue( zoom );
         }

         // just reset the position to avoid moving artifact while LC and or RC
         _initialOrientation = e.mousePosition;
      }

   private:
      nll::core::vector2ui  _initialOrientation;
      ui32                  _mipFps;   // frame per second
      time_t                _lastLeftClick;
   };
}
}

#endif
