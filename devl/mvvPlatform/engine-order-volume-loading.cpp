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
         boost::mutex   mutex;
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
         OrderVolumeLoader( const std::string& location, SymbolVolume name, boost::mutex& mutex ) : Order( MVV_ORDER_VOLUME_LOADING, Order::Predecessors(), true ), _location( location ), _name( name ), _mutex( mutex )
         {
         }

      protected:
         virtual OrderResult* _compute()
         {
            OrderVolumeLoaderResult* result = 0;

            {
               boost::mutex::scoped_lock lock( _mutex );
               RefcountedTyped<Volume> volume( new Volume() );
               bool loaded = nll::imaging::loadSimpleFlatFile( _location, *volume );
               if ( loaded )
               {
                  std::cout << "volume loaded: " << _location << " size=" << (*volume).getSize()[ 0 ] << ":" << (*volume).getSize()[ 1 ] << ":" << (*volume).getSize()[ 2 ] << std::endl;
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
         boost::mutex&           _mutex;
      };
   }


   void EngineOrderVolumeLoader::loadVolume( const std::string& location, SymbolVolume name )
   {
      impl::RecordOrder* record = new impl::RecordOrder();
      _records[ name ] = record;
      _orderProvider.pushOrder( new impl::OrderVolumeLoader( location, name, record->mutex ) );
   }

   void EngineOrderVolumeLoader::consume( Order* order )
   {
      impl::OrderVolumeLoaderResult* result = dynamic_cast<impl::OrderVolumeLoaderResult*>( (*order).getResult() );
      _resourceVolumes.insert( result->name, result->volume );
      _resourceVolumes.notify();
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
      Records::iterator it = _records.find( name );
      if ( it == _records.end() )
      {
         // in this case the volume is already loaded, or it doesn't exist!
         bool found = _resourceVolumes.find( name, vol );
         if ( !found )
         {
            // doesn't exist
            throw std::exception( ( std::string( "volume ID can't be found:" ) + name.getName() ).c_str() );
         } else return vol;
      }

      // we have a record, block the call until it is loaded
      boost::mutex::scoped_lock lock( it->second->mutex );
      bool found = _resourceVolumes.find( name, vol );
      assert( found );  // hmm how can we not find as it just finished loading?
      return vol;
   }


}
}