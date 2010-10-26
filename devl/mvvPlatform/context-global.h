#ifndef MVV_PLATFORN_CONTEXT_GLOBAL_H_
# define MVV_PLATFORN_CONTEXT_GLOBAL_H_

# include "mvvPlatform.h"
# include "context.h"
# include "engine-handler-impl.h"
# include "order-manager-thread-pool.h"
# include "font.h"
# include "layout-pane.h"
# include "mouse-pointer.h"
# include <mvvScript/completion-interface.h>

namespace mvv
{
namespace platform
{
   /**
    @brief Global context holding an engine handler & order manager
    */
   class MVVPLATFORM_API ContextGlobal : public ContextInstance
   {
   public:
      ContextGlobal( EngineHandlerImpl& e, OrderManagerThreadPool& o, Font& font, nll::core::Image<ui8>& sscreen ) : engineHandler( e ), orderManager( o ), commonFont( font ), screen( sscreen )
      {
      }

      // we need copy: we can't actually store these in the context: they need to be alive while all resources are being deallocated
      EngineHandlerImpl&                   engineHandler;
      OrderManagerThreadPool&              orderManager;
      Font&                                commonFont;
      RefcountedTyped<Pane>                layout;
      RefcountedTyped<parser::CompletionInterface> completion;
      RefcountedTyped<MousePointer>        mousePointer;
      nll::core::Image<ui8>&               screen;

      RefcountedTyped<MouseSegmentCallback> onSegmentLeftMouseClick;    // callback to run in a segment on a left click
      RefcountedTyped<MouseSegmentCallback> onSegmentRightMouseClick;   // callback to run in a segment on a right click
      RefcountedTyped<MouseSegmentCallback> onSegmentLeftMouseRelease;
      RefcountedTyped<MouseSegmentCallback> onSegmentRightMouseRelease;
      RefcountedTyped<MouseSegmentCallback> onSegmentLeftMousePressed;  // run while the mouse move + left pressed
      RefcountedTyped<MouseSegmentCallback> onSegmentRightMousePressed; // run while the mouse move + right pressed

   private:
      // disabled copy
      ContextGlobal& operator=( const ContextGlobal& );
      ContextGlobal( const ContextGlobal& );
   };
}
}

#endif