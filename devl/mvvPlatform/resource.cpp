#include "resource.h"
#include "engine.h"

namespace mvv
{
namespace platform
{
   namespace impl
   {
      void ResourceSharedData::notify()
      {
         bool isNotifying = false;
         if ( state == STATE_ENABLED && !isNotifying )
         {
            isNotifying = true;

            // notify engines
            for ( EngineStorage::iterator it = links.begin(); it != links.end(); ++it )
            {
               assert( *it );
               (*it)->notify( this );
            }
            // notify resources
            for ( ResourceStorage::iterator it = resources.begin(); it != resources.end(); ++it )
            {
               (*it)->notify();
            }
            needNotification = false;
         } else {
            needNotification = true;
         }
      }

      void Resource::notify()
      {
         getData().notify();
      }

      void Resource::connect( Engine* e )
      {
         assert( e );
         _addSimpleLink( e );
         e->_addSimpleLink( *this );
      }

      void Resource::disconnect( Engine* e )
      {
         assert( e );
         if ( !_data )
            return;
         _eraseSimpleLink( e );
         e->_eraseSimpleLink( *this );
      }

      Resource::~Resource()
      {
      }

      void Resource::setState( ResourceState s )
      {
         if ( getData().state == STATE_DISABLED && s == STATE_ENABLED )
         {
            getData().state = s;

            // we will notify the connected engines if needed
            if ( getData().needNotification )
               notify();
         } else {
            getData().state = s;
         }
      }

      bool Resource::needNotification() const
      {
         return getData().needNotification;
      }

      ResourceState Resource::getState() const
      {
         return getData().state;
      }
   }
}
}
