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
         getData()._addSimpleLink( e );
         e->_addSimpleLink( &getData() );
      }

      void Resource::disconnect( Engine* e )
      {
         assert( e );
         if ( !_data )
            return;
         getData()._eraseSimpleLink( e );
         e->_eraseSimpleLink( &getData() );
      }

      Resource::~Resource()
      {
         getData()._eraseSimpleLink( this );
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

      Resource& Resource::operator=( const Resource& r )
      {
         if ( r._data != _data )
         {
            Refcounted::Internals* internals = _data;

            // we need to update engine connections
            if ( _data && _data->data )   // else it is a copy constructor call...
            {
               std::cout << "update resource=" <<  r._data << " old=" << _data << std::endl;
               // destroy the now unused resource
               if ( getData().own )
               {
                  delete getData().privateData;
                  getData().privateData = 0;
               }

               // point to the same data and update ref count & links
               ResourceSharedData& old = getData();

               _data = r._data;
              _data->ref += internals->ref;

              // update engines & 
              while ( old.links.size() )
              {
                 // disconnect and reconnect the engine
                 Engine* e = *old.links.begin();
                 e->disconnect( &old );
                 e->connect( &getData() );

                 // merge links to engine
                 getData().links.insert( e );
              }

              // merge the links
              for ( ResourceSharedData::ResourceStorage::iterator it = old.resources.begin(); it != old.resources.end(); ++it )
              {
                 getData().resources.insert( *it );
              }

              // merge the links
              for ( ResourceSharedData::ResourceStorage::iterator it = old.resourcesLinks.begin(); it != old.resourcesLinks.end(); ++it )
              {
                 // update the link of the linked resource
                 (*it)->_eraseSimpleLink( &old );
                 (*it)->_addSimpleLinkConnection( &getData() );

                 // merge the linked resource
                 getData().resourcesLinks.insert( *it );
              }

              // update the resource holders and merge them
              for ( ResourceSharedData::ResourceHolder::iterator it = old.resourceHolder.begin(); it != old.resourceHolder.end(); ++it )
              {
                 (*it)->_data = _data;
                 std::cout << "resource holder change:" << *it << " to=" << _data << std::endl;
                 getData().resourceHolder.insert( *it );
              }

              notify();
            }

            // clean up memory
            internals->data = 0;
            delete internals->data;

            // in case the resource is different, we need to notify it has changed
            Base::operator=( r );
         }
         return *this;
      }
   }
}
}
