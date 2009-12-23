#ifndef MVV_MPR_ENGINE_MPR_H_
# define MVV_MPR_ENGINE_MPR_H_

#include <mvvPlatform/engine-order.h>
#include <mvvPlatform/resource-vector.h>
#include <mvvPlatform/resource-volumes.h>
#include <mvvPlatform/resource-typedef.h>

namespace mvv
{
namespace platform
{
   namespace impl
   {
      class EngineMprSlice : EngineOrder
      {
      public:
         ResourceVolumes      volumes;       /// list of volume to display
         ResourceVector3f     position;      /// position in mm of the camera
         ResourceVector3f     directionx;    /// x-axis of the MPR
         ResourceVector3f     directiony;    /// y-axis of the MPR
         ResourceVector3f     panning;       /// panning position in mm
         ResourceVector2f     zoom;          /// zoom

      public:
         EngineMprSlice( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher ), volumes( storage )
         {
            position.connect( this );
            directionx.connect( this );
            directiony.connect( this );
            panning.connect( this );
            zoom.connect( this );
            volumes.connect( this );
         }
      };
   }

   class EngineMpr : EngineOrder
   {
   public:
      // public input slots, any change in a slot and the engine needs to be recomputed
      ResourceVector3f     position;      /// position in mm of the camera
      ResourceVector3f     directionx;    /// x-axis of the MPR
      ResourceVector3f     directiony;    /// y-axis of the MPR
      ResourceVector3f     panning;       /// panning position in mm
      ResourceVector2f     zoom;          /// zoom
      ResourceVolumes      volumes;       /// list of volume to display
      ResourceTransferFunction lut;       /// list of LUTs associated to the volume
      ResourceFloats       intensities;/// list of intensities associated to the volume

   protected:


   public:
      EngineMpr( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher ), volumes( storage )
      {
         position.connect( this );
         directionx.connect( this );
         directiony.connect( this );
         panning.connect( this );
         zoom.connect( this );
         volumes.connect( this );
         lut.connect( this );
         intensities.connect( this );
      }
   };
}
}

#endif