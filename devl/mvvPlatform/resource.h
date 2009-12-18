#ifndef MVV_PLATFORM_RESOURCE_H_
# define MVV_PLATFORM_RESOURCE_H_

# include <set>
# include "mvvPlatform.h"
# include "refcounted.h"
# include "notifiable.h"

namespace mvv
{
namespace platform
{
   // forward declaration
   class Engine;

   namespace impl
   {
      struct MVVPLATFORM_API ResourceSharedData
      {
         typedef std::set<Engine*>  EngineStorage;
         ResourceSharedData() : privateData( 0 ), own( false )
         {}

         ResourceSharedData( void* d, bool o ) : privateData( d ), own( o )
         {}

         void*             privateData;
         bool              own;
         EngineStorage     links;
      };

      /**
       @brief Resource. Should not be used in client code as it is not type safe
       */
      class MVVPLATFORM_API Resource : public RefcountedTyped<ResourceSharedData>, public Notifiable
      {
         typedef RefcountedTyped<ResourceSharedData>  Base;

      public:
         enum State
         {
            ENABLED, /// the resource is enabled
            DISABLED /// the resource is disabled and won't trigger a change
         };
      public:
         Resource( void* resourceData, bool own = true ) : Base( new ResourceSharedData( resourceData, own ), true )
         {
            init();
         }

         virtual void notify();

         void connect( Engine* e );

         void disconnect( Engine* e );

         void setState( const State s )
         {
            if ( _state == DISABLED && s == ENABLED )
            {
               _state = s;
               notify();
            } else {
               _state = s;
            }
         }

         State getState() const
         {
            return _state;
         }

         virtual ~Resource();

      protected:
         void init()
         {
            _state = ENABLED;
         }
      protected:
         State    _state;
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