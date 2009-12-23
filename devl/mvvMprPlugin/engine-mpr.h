#ifndef MVV_MPR_ENGINE_MPR_H_
# define MVV_MPR_ENGINE_MPR_H_

#include <mvvPlatform/engine-order.h>
#include <mvvPlatform/resource-vector.h>
#include <mvvPlatform/resource-volumes.h>

namespace mvv
{
namespace platform
{
   class EngineMpr : EngineOrder
   {
   public:
      // public slots
      ResourceVector3f     position;
      ResourceVector3f     directionx;
      ResourceVector3f     directiony;
      ResourceVector3f     panning;
      ResourceVector2f     zoom;
      ResourceVolumes      volumes;

   public:
      EngineMpr( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher ), volumes( storage )
      {}
   };
}
}

#endif