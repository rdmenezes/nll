#ifndef MVV_PLATFORM_ENGINE_HANDLER_IMPL_H_
# define MVV_PLATFORM_ENGINE_HANDLER_IMPL_H_

# include "engine-handler.h"

namespace mvv
{
namespace platform
{
   class MVVPLATFORM_API EngineHandlerImpl : public EngineHandler
   {
      typedef std::set<Engine*> Storage;

   public:
      virtual void connect( Engine& e )
      {
         engines.insert( &e );
      }

      virtual void disconnect( Engine& e )
      {
         engines.erase( &e );
      }

      virtual void run()
      {
         for ( Storage::iterator it = engines.begin(); it != engines.end(); ++it )
         {
            (*it)->run();
         }
      }

   private:
      Storage engines;
   };
}
}

# endif