#ifndef MVV_MPR_ENGINE_MPR_H_
# define MVV_MPR_ENGINE_MPR_H_

#include <mvvPlatform/engine-order.h>

namespace mvv
{
namespace platform
{
   class EngineMpr : EngineOrder
   {
   public:
      EngineMpr( EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher )
      {}
   };
}
}

#endif