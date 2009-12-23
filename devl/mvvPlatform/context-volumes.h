#ifndef MVV_PLATFORM_CONTEXT_VOLUMES_H_
# define MVV_PLATFORM_CONTEXT_VOLUMES_H_

# include "resource-storage-volumes.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Hold the volumes. They are referenced by a name and are refcounted.

    Each time 
    */
   struct MVVPLATFORM_API ContextVolume : public ContextInstance
   {
      ResourceStorageVolumes      volumes;
   };
}
}

#endif