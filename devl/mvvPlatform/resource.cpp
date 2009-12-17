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
            for ( ResourceSharedData::EngineStorage::iterator it = getData().links.begin(); it != getData().links.end(); ++it )
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
         size_t nb = getData().links.erase( e );
         if ( nb )
         {
            e->disconnect( *this );
         }
      }

      Resource::~Resource()
      {
         for ( impl::ResourceSharedData::EngineStorage::iterator it = getData().links.begin();
               it != getData().links.end();
               ++ it )
         {
            disconnect( *it );
         }
      }
   }
}
}
