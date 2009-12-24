#ifndef MVV_PLATFORM_ENGINE_H_
# define MVV_PLATFORM_ENGINE_H_

# include <set>
# include "mvvPlatform.h"
# include "notifiable.h"
# include "engine-handler.h"
# include "resource.h"

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
    @note the engine when destroyed will destroy its resource reference and will initiate a disconnect() on all the resources
    */
   class MVVPLATFORM_API Engine : public Notifiable
   {
      typedef std::set<impl::Resource>   ResourceStorage;

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

      virtual void notify()
      {
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
       */
      virtual void run()
      {
         if ( _state == ENGINE_ENABLED && _needToRecompute )
         {
            _needToRecompute = !_run();
         }
      }

      virtual ~Engine();

   protected:
      /**
       @brief connect the resource to the engine and add the resource to the resources (we need to hold a reference
              in case all references are lost externally, guaranteeing no resource used by the engine can be lost...)
       */
      void connect( impl::Resource r );

      /**
       @brief Disconnect the resource from the engine and clear this resource for the list of used resources
       */
      void disconnect( impl::Resource r );

   private:
      // disable copy operators
      Engine& operator=( const Engine& );
      Engine( const Engine& );

   protected:
      bool              _needToRecompute;
      ResourceStorage   _resources;
      EngineHandler&    _handler;
      State             _state;
   };
}
}

#endif
