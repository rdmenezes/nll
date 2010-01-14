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
      struct MVVPLATFORM_API ResourceSharedData
      {
         typedef std::set<Engine*>  EngineStorage;
         ResourceSharedData() : privateData( 0 ), own( false ), state( STATE_ENABLED ), needNotification( false )
         {}

         ResourceSharedData( void* d, bool o ) : privateData( d ), own( o ), state( STATE_ENABLED ), needNotification( false )
         {}

         void*             privateData;
         bool              own;
         EngineStorage     links;
         ResourceState     state;
         bool              needNotification; /// when the resource is asleep, we need to recompute when reactivated hence this flag!
      };

      /**
       @brief Resource. Should not be used in client code as it is not type safe
       */
      class MVVPLATFORM_API Resource : public RefcountedTyped<ResourceSharedData>, public Notifiable
      {
         typedef RefcountedTyped<ResourceSharedData>  Base;
      public:
         Resource( void* resourceData, bool own = true ) : Base( new ResourceSharedData( resourceData, own ), true )
         {
         }

         virtual void notify();

         ui32 getNbConnectedEngines() const
         {
            return static_cast<ui32>( getData().links.size() );
         }

         void connect( Engine* e );

         void disconnect( Engine* e );

         void setState( ResourceState s );

         bool needNotification() const;

         ResourceState getState() const;

         virtual ~Resource();

         Resource& operator=( const Resource& r )
         {
            if ( r._data != _data )
            {
               // in case the resource is different, we need to notify it has changed
               notify();
            }
            Base::operator=( r );
            return *this;
         }

         //
         // this should be only called internally
         //
         void _addSimpleLink( Engine* o )
         {
            assert( o );
            getData().links.insert( o );
         }

         //
         // this should be only called internally
         //
         void _eraseSimpleLink( Engine* o )
         {
            assert( o );
            impl::ResourceSharedData::EngineStorage::iterator it = getData().links.find( o );
            if ( it != getData().links.end() )
               getData().links.erase( it );
         }
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