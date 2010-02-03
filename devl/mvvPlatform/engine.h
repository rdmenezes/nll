#ifndef MVV_PLATFORM_ENGINE_H_
# define MVV_PLATFORM_ENGINE_H_

# include <set>
# include "mvvPlatform.h"
# include "notifiable.h"
# include "engine-handler.h"
# include "resource.h"
# include "linkable.h"

namespace mvv
{
namespace platform
{
   namespace impl
   {
      // forward declaration
      class Resource;
   }

   /**
    @ingroup platform
    @brief Implement an engine that compute output from a set of resources. If one resource change,
           the result must be recomputed.
    @note when destroyed, the engines will unregister all the resources
    */
   class MVVPLATFORM_API Engine : public LinkableDouble< impl::Resource, Engine* >
   {
   public:
      typedef LinkableDouble< impl::Resource, Engine* > Linkable;

   public:
      enum State
      {
         ENGINE_ENABLED,
         ENGINE_DISABLED
      };

   public:
      Engine( EngineHandler& handler ) : _needToRecompute( true ), _handler( handler ), _state( ENGINE_ENABLED )
      {
         _handler.connect( *this );
      }

      virtual void notify( impl::ResourceSharedData* r )
      {
         _notifiedResources.insert( r );
         _needToRecompute = true;
      }

      /**
       @brief In state disabled, the engine will receive notifications but won't run the actual _run() method
              until it is activated.
       */
      void setState( State s )
      {
         _state = s;
      }

      /**
       @brief Returns the current state of the engine
       */
      State getState() const
      {
         return _state;
      }

      /**
       @brief This method will be invoked if the engine must recompute an output
       @return false if the engine did not complete successfully and need to be recomputed
               next cycle
       */
      virtual bool _run() = 0;

      /**
       @brief This method will execute _run() if the engine is enabled and a connected resource has been notified
       @note if the engine has run successfully, the list of resources that triggered the engine update is cleared
       */
      virtual void run()
      {
         if ( _state == ENGINE_ENABLED && _needToRecompute )
         {
            _needToRecompute = !_run();
            if ( !_needToRecompute )
            {
               clearNotifiedResources();
            }
         }
      }

      virtual ~Engine();

      /**
       @brief Returns true if this resource is connected to this engine
       */
      bool isConnected( impl::Resource r ) const;

      /**
       @brief Returns true if this resource has triggered the engine
       */
      template <class T>
      bool hasTriggeredNotification( const Resource<T>& r ) const
      {
         return _notifiedResources.find( &r );
      }

   private:
      void clearNotifiedResources()
      {
         _notifiedResources.clear();
      }

      /**
       @brief connect the resource to the engine and add the resource to the resources (we need to hold a reference
              in case all references are lost externally, guaranteeing no resource used by the engine can be lost...)
       */
      virtual void connect( impl::Resource r );

      /**
       @brief Disconnect the resource from the engine and clear this resource for the list of used resources
       */
      virtual void disconnect( impl::Resource r );

   private:
      // disable copy operators
      Engine& operator=( const Engine& );
      Engine( const Engine& );

   protected:
      bool                       _needToRecompute;
      EngineHandler&             _handler;
      State                      _state;

   private:
      std::set<impl::ResourceSharedData*>  _notifiedResources;
   };
}
}

#endif
