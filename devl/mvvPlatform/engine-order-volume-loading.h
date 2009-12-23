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
      struct OrderVolumeLoaderResult : OrderResult
      {
         OrderVolumeLoaderResult( RefcountedTyped<Volume> v, SymbolVolume name ) : volume( v ), name( name )
         {
         }

         SymbolVolume name;
         RefcountedTyped<Volume> volume;
      };

      class OrderVolumeLoader : public Order
      {
      public:
         OrderVolumeLoader( const std::string& location, SymbolVolume name ) : Order( MVV_ORDER_VOLUME_LOADING, Order::Predecessors(), true ), _location( location ), _name( name )
         {
         }

      protected:
         virtual OrderResult* _compute()
         {
            RefcountedTyped<Volume> volume( new Volume() );
            bool loaded = nll::imaging::loadSimpleFlatFile( _location, *volume );
            if ( !loaded )
            {
               throw std::exception( "Volume was not loaded correctly" );
            }
            return new OrderVolumeLoaderResult( volume, _name );
         }

      private:
         std::string             _location;
         SymbolVolume            _name;
      };
   }

   /**
    @ingroup platform
    @brief Helper method to asynchronously load volumes and store them in a ResourceVolumes object
    */
   class MVVPLATFORM_API EngineOrderVolumeLoader : public EngineOrder
   {
   public:
      EngineOrderVolumeLoader( ResourceStorageVolumes resourceVolumes, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : EngineOrder( handler, provider, dispatcher ), _resourceVolumes( resourceVolumes )
      {
         construct();
      }

      EngineOrderVolumeLoader( ResourceStorageVolumes resourceVolumes, EngineHandler& handler, OrderManagerThreadPool& manager ) : EngineOrder( handler, manager, manager ), _resourceVolumes( resourceVolumes )
      {
         construct();
      }

      void loadVolume( const std::string& location, SymbolVolume name )
      {
         _orderProvider.pushOrder( new impl::OrderVolumeLoader( location, name ) );
      }

      virtual void consume( RefcountedTyped<Order> order )
      {
         impl::OrderVolumeLoaderResult* result = dynamic_cast<impl::OrderVolumeLoaderResult*>( (*order).getResult() );
         _resourceVolumes.insert( result->name, result->volume );
      }

      virtual const std::set<OrderClassId>& interestedOrder()
      {
         return _interested;
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
   };
}
}

#endif