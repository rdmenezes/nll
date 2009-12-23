#ifndef MVV_MPR_ENGINE_MPR_H_
# define MVV_MPR_ENGINE_MPR_H_

#include <mvvPlatform/engine-order.h>
#include <mvvPlatform/resource-vector.h>
#include <mvvPlatform/resource-volumes.h>

namespace mvv
{
namespace platform
{
   namespace impl
   {
   }

   class EngineMpr : EngineOrder
   {
   public:
      // public input slots
      ResourceVector3f     position;      /// position in mm of the camera
      ResourceVector3f     directionx;    /// x-axis of the MPR
      ResourceVector3f     directiony;    /// y-axis of the MPR
      ResourceVector3f     panning;       /// panning position in mm
      ResourceVector2f     zoom;          /// zoom
      ResourceVolumes      volumes;       /// list of volume to display

   public:
      EngineMpr( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher ), volumes( storage )
      {}
   };
}
}

#endif