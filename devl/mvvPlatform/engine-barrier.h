#ifndef MVV_PLATFORM_ENGINE_BARRIER_H_
# define MVV_PLATFORM_ENGINE_BARRIER_H_

# include "engine.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief This barrier prevents any engine to be _run() while active
    */
   class MVVPLATFORM_API EngineScopedBarrier
   {
   public:
      EngineScopedBarrier( Engine& engine ) : _engine( engine ), _initialState( engine.getState() )
      {
         _engine.setState( Engine::ENGINE_DISABLED );
      }

      ~EngineScopedBarrier()
      {
         _engine.setState( _initialState );
      }

   private:
      // disabled copy
      EngineScopedBarrier& operator=( const EngineScopedBarrier& );
      EngineScopedBarrier( const EngineScopedBarrier& );

   private:
      Engine&        _engine;
      Engine::State  _initialState;
   };

   /**
    @ingroup platform
    @brief This barrier prevents any engine to be _run() while active
    */
   class MVVPLATFORM_API EngineBarrier
   {
   public:
      EngineBarrier() : _engine( 0 )
      {
      }

      void connect( Engine& engine )
      {
         disconnect();

         _engine = &engine;
         _initialState = engine.getState();
         _engine->setState( Engine::ENGINE_DISABLED );
      }

      void disconnect()
      {
         if ( _engine )
         {
            _engine->setState( _initialState );
            _engine = 0;
         }
      }

      ~EngineBarrier()
      {
         disconnect();
      }

   private:
      // disabled copy
      EngineBarrier& operator=( const EngineBarrier& );
      EngineBarrier( const EngineBarrier& );

   private:
      Engine*        _engine;
      Engine::State  _initialState;
   };
}
}

#endif