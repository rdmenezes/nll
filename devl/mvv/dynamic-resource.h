#ifndef MVV_DYNAMIC_RESOURCE_H_
# define MVV_DYNAMIC_RESOURCE_H_

# include <set>
# include "order.h"

namespace mvv
{
   // forward declaration
   class DynamicResource;

   /**
    @ingroup mvv
    @brief Defines an engine. It is an object that computes things depending on DynamicResource class. 
    */
   class MVV_API Engine
   {
      typedef std::set<DynamicResource*>  Resources;

   public:
      Engine() : _needToRecompute( true ), _activated( true )
      {}

      virtual ~Engine();

      /**
       @brief Consume an order
       */
      virtual void consume( Order* ) = 0;

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
            if ( _needToRecompute )
               _run();
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

   protected:
      /**
       @brief Compute a result.
       */
      virtual void _run() = 0;

   protected:
      bool        _needToRecompute;
      Resources   _resources;
      bool        _activated;
   };

   /**
    @brief A class that holds dynamic resource. Each time a resource is modified, it notifies all the listeners
    */
   class DynamicResource
   {
      typedef std::set<Engine*>  Engines;

   public:
      virtual ~DynamicResource()
      {}

      /**
       @brief Call this method to notify that the resource has changed. Engines need to recalculate
       */
      void notifyChanges()
      {
         for ( Engines::iterator it = _engines.begin(); it != _engines.end(); ++it )
            ( *it )->notify();
      }

      /**
       @brief Attach a resource to an engine. It means each time a resource is changed, the engine needs
              to recalculate everything.
       */
      void attach( Engine* resource )
      {
         _engines.insert( resource );
      }

      /**
       @brief must be called when an engine doesn't use the resource anymore
       */
      void detach( Engine* resource )
      {
         _engines.erase( resource );
      }

   protected:
      Engines _engines;
   };
}

#endif
