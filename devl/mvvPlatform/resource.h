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

         ResourceSharedData() : privateData( 0 ), own( false ), state( STATE_ENABLED ), needNotification( false ), simple( false )
         {}

         ResourceSharedData( void* d, bool o, bool s ) : privateData( d ), own( o ), state( STATE_ENABLED ), needNotification( false ), simple( s )
         {}

         void*             privateData;
         bool              own;
         EngineStorage     links;
         ResourceStorage   resources;        /// the connected resources
         ResourceHolder    resourceHolder;   /// the link to all resources pointing to this ResourceSharedData
         ResourceStorage   resourcesLinks;   /// when destroyed, and it is a resource connected to another resource, we must remove the of the other resource from 'this'
         ResourceState     state;
         bool              needNotification; /// when the resource is asleep, we need to recompute when reactivated hence this flag!
         bool              simple;           /// if true, we are using a simple resource model: b = d, a = b, then a = c, then b still points to d until it is alive

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

         ~ResourceSharedData();

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
         void forceNeedNotification( bool val )
         {
            needNotification = val;
         }
      };

      /**
       @brief Resource. Should not be used in client code as it is not type safe

       A resource can be connected to other resources, with 2 models:
       - simple: resource are refcounted, connected resources & engines are not modified when operator=
       - not simple: all connected resources, resource holders & engines will be updated upon operator=
       */
      class MVVPLATFORM_API Resource : public RefcountedTyped<ResourceSharedData>, public Notifiable
      {
         typedef RefcountedTyped<ResourceSharedData>  Base;
      public:
         Resource( void* resourceData, bool own = true, bool simple = false ) : Base( new ResourceSharedData( resourceData, own, simple ), true )
         {
            assert( resourceData );
            //std::cout << "_data=" << _data << " data=" << _data->data << " HOLDER=" << this << std::endl;
            if ( !getData().simple )
               getData()._addSimpleLink( this );
         }

         Resource( const Resource& r )
         {
            //std::cout << "cpy _data=" << _data << " data=" << _data->data << " HOLDER=" << this << std::endl;
            operator=( r );

            if ( !getData().simple )
               getData()._addSimpleLink( this );
         }

         /**
          @brief notifies all the connected resources & engines
          */
         virtual void notify();

         ui32 getNbConnectedEngines() const
         {
            return static_cast<ui32>( getData().links.size() );
         }

         /**
          @brief connect an engine. An engine cannot be 'moved', it should always be at the same position in memory,
                 as we only take a reference on it
          */
         void connect( Engine* e );

         void disconnect( Engine* e );

         /**
          @brief connect a resource, meaming if the resource is notified, the connected resources
                 will also be notified
          */
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

         void forceNeedNotification( bool val )
         {
            getData().forceNeedNotification( val );
         }

         Resource& operator=( const Resource& r );
      };
   }

   /**
    @brief Resource that notifies engines when its value changes...
    @note this version is the type safe version of impl::Engine
    @note simple: if true, the resource, if replaced, doesn't replace the other connected resources.
          when a simple resource is copied/replaced, the target is transformed into a simple resource
          if simple = false, if a resource is replaced _all_ the connections will be replaced (resource dependencies, resource holders & engines)
    */
   template <class T>
   class Resource : public impl::Resource
   {
   public:
      typedef T   value_type;

   public:
      Resource( T* resourceData, bool own = true, bool simple = false ) : impl::Resource( resourceData, own, simple )
      {
         assert( resourceData );
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