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
      /**
       @brief connect an engine. Meaning next time, the <code>run()</code> of the engine will be called
       */
      virtual void connect( Engine& e ) = 0;

      /**
       @brief disconnect the engine. the <code>run()</code> of the engine won't be called anymore
       */
      virtual void disconnect( Engine& e ) = 0;

      /**
       @brief this method will call the <code>run()</code> method of the engine, with an unspecified order
       */
      virtual void run() = 0;

      virtual ~EngineHandler()
      {
      }
   };
}
}

#endif