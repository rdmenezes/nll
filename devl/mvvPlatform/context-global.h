#ifndef MVV_PLATFORN_CONTEXT_GLOBAL_H_
# define MVV_PLATFORN_CONTEXT_GLOBAL_H_

# include "mvvPlatform.h"
# include "context.h"
# include "engine-handler-impl.h"
# include "order-manager-thread-pool.h"
# include "font.h"

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
      ContextGlobal( EngineHandlerImpl& e, OrderManagerThreadPool& o, Font& font ) : engineHandler( e ), orderManager( o ), commonFont( font )
      {}

      // we need copy: we can't actually store these in the context: they need to be alive while all resources are being deallocated
      EngineHandlerImpl&                   engineHandler;
      OrderManagerThreadPool&              orderManager;
      Font&                                commonFont;

   private:
      // disabled copy
      ContextGlobal& operator=( const ContextGlobal& );
      ContextGlobal( const ContextGlobal& );
   };
}
}

#endif