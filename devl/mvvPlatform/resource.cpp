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
         if ( getData().state == STATE_ENABLED )
         {
            ResourceSharedData::EngineStorage& engines = getData().links;
            for ( ResourceSharedData::EngineStorage::iterator it = engines.begin(); it != engines.end(); ++it )
            {
               assert( *it );
               (*it)->notify();
            }
            getData().needNotification = false;
         } else {
            getData().needNotification = true;
         }
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
