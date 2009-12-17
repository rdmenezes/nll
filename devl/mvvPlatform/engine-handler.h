#ifndef MVV_PLATFORM_ENGINE_HANDLER_H_
# define MVV_PLATFORM_ENGINE_HANDLER_H_

# include "mvvPlatform.h"

namespace mvv
{
namespace platform
{
   // forward declaration
   class Engine;

   /**
    @ingroup platform
    @brief Handle a set of engines. They are stored and called through run() method
    */
   class MVVPLATFORM_API EngineHandler
   {
   public:
      virtual void connect( Engine* e ) = 0;

      virtual void disconnect( Engine* e ) = 0;

      virtual void run() = 0;

      virtual ~EngineHandler()
      {
      }
   };
}
}

#endif