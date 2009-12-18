#include "resource.h"
#include "engine.h"

namespace mvv
{
namespace platform
{
   namespace impl
   {
      void Resource::notify()
      {
         if ( _state == ENABLED )
         {
            ResourceSharedData::EngineStorage& engines = getData().links;
            for ( ResourceSharedData::EngineStorage::iterator it = engines.begin(); it != engines.end(); ++it )
            {
               assert( *it );
               (*it)->notify();
            }
         }
      }

      void Resource::connect( Engine* e )
      {
         assert( e );
         getData().links.insert( e );
      }

      void Resource::disconnect( Engine* e )
      {
         assert( e );
         if ( !_data )
            return;
         getData().links.erase( e );
      }

      Resource::~Resource()
      {
      }
   }
}
}
