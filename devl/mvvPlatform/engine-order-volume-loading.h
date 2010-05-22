#ifndef MVV_PLATFORM_ENGINE_ORDER_VOLUME_LOADING_H_
# define MVV_PLATFORM_ENGINE_ORDER_VOLUME_LOADING_H_

# include "order-manager-thread-pool.h"
# include "engine-order.h"
# include "context-volumes.h"
# include "order-definition.h"
# include <nll/nll.h>

namespace mvv
{
namespace platform
{
   namespace impl
   {
      class OrderVolumeLoader;
      struct RecordOrder;
   }

   /**
    @ingroup platform
    @brief Helper method to asynchronously load volumes and store them in a ResourceVolumes object

    All volumes must be loaded using this helper
    */
   class MVVPLATFORM_API EngineOrderVolumeLoader : public EngineOrder
   {
      typedef std::map<SymbolVolume, impl::RecordOrder*> Records;

   public:
      EngineOrderVolumeLoader( ResourceStorageVolumes& resourceVolumes, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher ), _resourceVolumes( resourceVolumes )
      {
         construct();
         dispatcher.connect( this );
      }

      EngineOrderVolumeLoader( ResourceStorageVolumes& resourceVolumes, EngineHandler& handler, OrderManagerThreadPool& manager ) : EngineOrder( handler, manager, manager ), _resourceVolumes( resourceVolumes )
      {
         construct();
      }

      ~EngineOrderVolumeLoader();

      // asynchronously load a volume
      void loadVolume( const std::string& location, SymbolVolume name );

      // get a volume. If it is currently being loaded, the call to this method will be blocked, until the volume is fully loaded
      RefcountedTyped<Volume> getVolume( SymbolVolume name );

      virtual void consume( Order* order );

      virtual const std::set<OrderClassId>& interestedOrder() const
      {
         return _interested;
      }

      virtual bool _run()
      {
         // we don't use any resources...
         return true;
      }

   protected:
      void construct()
      {
         std::set<OrderClassId> i;
         i.insert( MVV_ORDER_VOLUME_LOADING );
         _interested = i;
      }

   protected:
      ResourceStorageVolumes  _resourceVolumes;
      std::set<OrderClassId>  _interested;
      Records _records;     // the record stores the volumes currently being loaded. A mutex is aquired, so that we a blocking function can be implemented, waiting for the volume to be fully loaded
   };
}
}

#endif