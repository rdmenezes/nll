#ifndef MVV_PLATFORM_ENGINE_H_
# define MVV_PLATFORM_ENGINE_H_

# include "mvvPlatform.h"
# include "notifiable.h"
# include "engine-handler.h"
# include <set>

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
    */
   class MVVPLATFORM_API Engine : public Notifiable
   {
      typedef std::set<impl::Resource>   ResourceStorage;

   public:
      Engine( EngineHandler& handler ) : _needToRecompute( true ), _handler( handler )
      {
         _handler.connect( this );
      }

      void connect( impl::Resource r );

      void disconnect( impl::Resource r );

      virtual void notify()
      {
         _needToRecompute = true;
      }

      /**
       @brief This method will be invoked if the engine must recompute an output
       @return false if the engine did not complete successfully and need to be recomputed
               next cycle
       */
      virtual bool _run() = 0;

      void run()
      {
         if ( _needToRecompute )
         {
            _needToRecompute = !_run();
         }
      }

      virtual ~Engine();

   private:
      // disable copy operators
      Engine& operator=( const Engine& );
      Engine( const Engine& );

   protected:
      bool              _needToRecompute;
      ResourceStorage   _resources;
      EngineHandler&    _handler;
   };
}
}

#endif
