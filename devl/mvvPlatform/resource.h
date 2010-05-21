#ifndef MVV_PLATFORM_RESOURCE_H_
# define MVV_PLATFORM_RESOURCE_H_

# include <set>
# include "mvvPlatform.h"
# include "refcounted.h"
# include "notifiable.h"
# include "types.h"

namespace mvv
{
namespace platform
{
   // forward declaration
   class Engine;

   enum ResourceState
   {
      STATE_ENABLED, /// the resource is enabled
      STATE_DISABLED /// the resource is disabled and won't trigger a change
   };

   namespace impl
   {
      class Resource;

      struct MVVPLATFORM_API ResourceSharedData
      {
         typedef std::set<Engine*>              EngineStorage;
         typedef std::set<ResourceSharedData*>  ResourceStorage;
         typedef std::set<Resource*>            ResourceHolder;

         ResourceSharedData() : privateData( 0 ), own( false ), state( STATE_ENABLED ), needNotification( false )
         {}

         ResourceSharedData( void* d, bool o ) : privateData( d ), own( o ), state( STATE_ENABLED ), needNotification( false )
         {}

         void*             privateData;
         bool              own;
         EngineStorage     links;
         ResourceStorage   resources;        /// the connected resources
         ResourceHolder    resourceHolder;   /// the link to all resources pointing to this ResourceSharedData
         ResourceStorage   resourcesLinks;   /// when destroyed, and it is a resource connected to another resource, we must remove the of the other resource from 'this'
         ResourceState     state;
         bool              needNotification; /// when the resource is asleep, we need to recompute when reactivated hence this flag!

         void _addSimpleLink( ResourceSharedData* r )
         {
            if ( r != this )
            {
               resources.insert( r );
            }
         }

         void _addSimpleLink( Resource* r )
         {
            resourceHolder.insert( r );
         }

         void _eraseSimpleLink( Resource* r )
         {
            ResourceHolder::iterator it = resourceHolder.find( r );
            if ( it != resourceHolder.end() )
               resourceHolder.erase( it );
         }

         void _addSimpleLinkConnection( ResourceSharedData* r )
         {
            if ( r != this )
            {
               resourcesLinks.insert( r );
            }
         }

         void _eraseSimpleLink( ResourceSharedData* r )
         {
            ResourceStorage::iterator it = resources.find( r );
            if ( it != resources.end() )
               resources.erase( it );
            it = resourcesLinks.find( r );
            if ( it != resourcesLinks.end() )
               resourcesLinks.erase( it );
         }

         ~ResourceSharedData()
         {
            for ( ResourceStorage::iterator it = resourcesLinks.begin(); it != resourcesLinks.end(); ++it )
            {
               (*it)->_eraseSimpleLink( this );
            }
         }

          void _addSimpleLink( Engine* o )
         {
            assert( o );
            links.insert( o );
         }

         //
         // this should be only called internally
         //
         void _eraseSimpleLink( Engine* o )
         {
            assert( o );
            impl::ResourceSharedData::EngineStorage::iterator it = links.find( o );
            if ( it != links.end() )
               links.erase( it );
         }

         void notify();
      };

      /**
       @brief Resource. Should not be used in client code as it is not type safe

       A resource can be connected to other resources, in this case
       */
      class MVVPLATFORM_API Resource : public RefcountedTyped<ResourceSharedData>, public Notifiable
      {
         typedef RefcountedTyped<ResourceSharedData>  Base;
      public:
         Resource( void* resourceData, bool own = true ) : Base( new ResourceSharedData( resourceData, own ), true )
         {
            getData()._addSimpleLink( this );
         }

         Resource( const Resource& r )
         {
            operator=( r );
         }

         virtual void notify();

         ui32 getNbConnectedEngines() const
         {
            return static_cast<ui32>( getData().links.size() );
         }

         void connect( Engine* e );

         void disconnect( Engine* e );

         void connect( Resource& e )
         {
            e.getData()._addSimpleLinkConnection( &getData() );
            getData()._addSimpleLinkConnection( &e.getData() );
            getData()._addSimpleLink( &e.getData() );
         }

         void disconnect( Resource& e )
         {
            e.getData()._eraseSimpleLink( &getData() );
            getData()._eraseSimpleLink( &e.getData() );
         }

         void setState( ResourceState s );

         bool needNotification() const;

         ResourceState getState() const;

         virtual ~Resource();

         Resource& operator=( const Resource& r );
      };
   }

   /**
    @brief Resource that notifies engines when its value changes...
    @note this version is the type safe version of impl::Engine
    */
   template <class T>
   class Resource : public impl::Resource
   {
   public:
      typedef T   value_type;

   public:
      Resource( T* resourceData, bool own = true ) : impl::Resource( resourceData, own )
      {
      };

      virtual void destroy()
      {
         if ( getData().own )
         {
            T* val = reinterpret_cast<T*>( getData().privateData );
            delete val;
         }
      }

      T& getValue()
      {
         return *reinterpret_cast<T*>( getData().privateData );
      }

      const T& getValue() const
      {
         return *reinterpret_cast<const T*>( getData().privateData );
      }
   };
}
}

#endif