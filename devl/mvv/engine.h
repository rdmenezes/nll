#ifndef MVV_ENGINE_H_
# define MVV_ENGINE_H_

namespace mvv
{
   // forward declaration
   class DynamicResource;

   /**
    @ingroup mvv
    @brief Defines an engine. It is an object that computes things depending on DynamicResource class.
           It should use and publish DynamicResource so that engines can be plugged in
    */
   class MVV_API Engine
   {
      typedef std::set<DynamicResource*>  Resources;

   public:
      Engine() : _needToRecompute( true ), _activated( true )
      {}

      virtual ~Engine();

      /**
       @brief Notify the engine it needs to be recomputed.
       */
      void notify()
      {
         _needToRecompute = true;
      }

      /**
       @brief Attach the engine to a resource. The resource must be alive until this class is used.
              The resources are automatically released when this engine is destroyed.
       */
      void attach( DynamicResource& r );

      /**
       @brief recompute a result. If it is inactivated, nothing is done.
       */
      void run()
      {
         if ( _activated )
         {
            bool isSuccessfullyRun = false;
            if ( _needToRecompute )
               isSuccessfullyRun = _run();
            if ( isSuccessfullyRun )
               _needToRecompute = false;
         }
      }

      /**
       @brief Specify if this engine is activated or not.

       In the case it is inactivated, the engine doesn't compute anything even if the resources have changed. It does
       however consume orders.
       */
      void setActivated( bool activated )
      {
         _activated = activated;
      }

      /**
       @brief Returns true if the engine need to be run
       */
      bool isNotified() const
      {
         return _needToRecompute;
      }

   protected:
      /**
       @brief Compute a result.
       @return true is successfully run. Else return 0, the engine will try again to run it next time
       */
      virtual bool _run() = 0;

   protected:
      bool        _needToRecompute;
      Resources   _resources;
      bool        _activated;
   };

   /**
    @ingroup mvv
    @brief Defines an engine. It is an object that computes things depending on DynamicResource class and reacting to orders. 
    */
   class MVV_API EngineRunnable : public Engine
   {
   public:
      virtual void consume( Order* ) = 0;

      EngineRunnable() : Engine()
      {}
   };
}

#endif