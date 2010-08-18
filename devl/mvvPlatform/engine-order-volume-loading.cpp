#include "engine-order-volume-loading.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace mvv
{
namespace platform
{
   namespace impl
   {
      struct RecordOrder
      {
         RecordOrder() : order( 0 )
         {}

         RecordOrder( Order* o ) : order( o )
         {}

         Order* order;
      };

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
            std::cout << "----volume loading order=" << name.getName() << std::endl;
         }

      protected:
         virtual OrderResult* _compute()
         {
            OrderVolumeLoaderResult* result = 0;

            {
               std::cout << "order start loading=" << _location << std::endl;
               RefcountedTyped<Volume> volume( new Volume() );
               bool loaded = nll::imaging::loadSimpleFlatFile( _location, *volume );
               if ( loaded )
               {
                  std::cout << "volume loaded: " << _location << " size=" << (*volume).getSize()[ 0 ] << ":" << (*volume).getSize()[ 1 ] << ":" << (*volume).getSize()[ 2 ] << std::endl;
                  std::cout << " spacing:" << (*volume).getSpacing()[ 0 ] << " " << (*volume).getSpacing()[ 1 ] << " " << (*volume).getSpacing()[ 2 ] << std::endl;
                  std::cout << " position:" << (*volume).getOrigin()[ 0 ] << " " << (*volume).getOrigin()[ 1 ] << " " << (*volume).getOrigin()[ 2 ] << std::endl;
               } else {
                  std::cout << "error: cant' load volume!!! " << _location << std::endl;
               }
               result = new OrderVolumeLoaderResult( volume, _name );
            }
            return result;
         }

      private:
         std::string             _location;
         SymbolVolume            _name;
      };
   }


   void EngineOrderVolumeLoader::loadVolume( const std::string& location, SymbolVolume name )
   {
      Order* order = new impl::OrderVolumeLoader( location, name );
      impl::RecordOrder* record = new impl::RecordOrder( order );
      _records[ name ] = record;
      _orderProvider.pushOrder( order );
   }

   void EngineOrderVolumeLoader::consume( Order* order )
   {
      impl::OrderVolumeLoaderResult* result = dynamic_cast<impl::OrderVolumeLoaderResult*>( (*order).getResult() );
      _resourceVolumes.insert( result->name, result->volume );
      _resourceVolumes.notify();

      Records::iterator it = _records.find( result->name );
      assert( it != _records.end() );  // it HAS to be here!
      _records.erase( it );

      // remove the result as there is currently a memleak with the orders
      delete (*order).getResult();
      (*order).setResult( 0 );
   }

   EngineOrderVolumeLoader::~EngineOrderVolumeLoader()
   {
      _dispatcher.disconnect( this );

      for ( Records::iterator it = _records.begin(); it != _records.end(); ++it )
      {
         delete it->second;
      }
   }

   RefcountedTyped<Volume> EngineOrderVolumeLoader::getVolume( SymbolVolume name )
   {
      RefcountedTyped<Volume> vol;
      //Records::iterator it = _records.find( name );
      //if ( it == _records.end() )
      {
         // in this case the volume is already loaded, or it doesn't exist!
         bool found = _resourceVolumes.find( name, vol );
         if ( found )
         {
            return vol;
            // doesn't exist
            //throw std::exception( ( std::string( "volume ID can't be found:" ) + name.getName() ).c_str() );
         } //else return vol;
      }

      Records::iterator it = _records.find( name );
      if ( it == _records.end() )
      {
         throw std::exception( ( std::string( "volume ID can't be found and no Future can be found:" ) + name.getName() ).c_str() );
      }

      // be sure the orders of the pool have been flushed... [problem if create asynchronous & wait in the same cyle...]
      _pool.run();

      std::cout << "----GET volume =" << name.getName() << std::endl;
      std::cout << "wating..." << std::endl;
      // lock the thread while the volume is being loaded
      ensure( it->second->order, "can't be null" );
      std::cout << "wating2..." << std::endl;
      Future* future = &(*it->second->order->getFuture());
      ensure( future, "can't be null" );
      //while ( !future )
      //   future = &(*it->second->order->getFuture());

      std::cout << "access...=" << &future << std::endl;
      future->wait();
      std::cout << "stop waiting" << std::endl;
      impl::OrderVolumeLoaderResult* result = dynamic_cast<impl::OrderVolumeLoaderResult*>( it->second->order->getResult() );
      return result->volume;
   }


}
}