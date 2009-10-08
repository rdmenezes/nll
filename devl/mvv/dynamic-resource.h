#ifndef MVV_DYNAMIC_RESOURCE_H_
# define MVV_DYNAMIC_RESOURCE_H_

# include <set>
# include "order.h"
# include "engine.h"

namespace mvv
{
   /**
    @ingroup mvv
    @brief A class that holds dynamic resource. Each time a resource is modified, it notifies all the listeners
    */
   class DynamicResource
   {
      typedef std::set<EngineRunnable*>  Engines;

   public:
      /**
       @param father if father uis not null, it means that all notification of the child, will also affect the
              father. It is used for example in MPR->volume->TransferFunction : a modification on the transfer
              function must force a notification
       */
      DynamicResource( DynamicResource* father = 0 ) : _father( father )
      {}

      void setFather( DynamicResource* father )
      {
         _father = father;
      }

      virtual ~DynamicResource()
      {}

      /**
       @brief Call this method to notify that the resource has changed. Engines need to recalculate
       */
      void notifyChanges()
      {
         for ( Engines::iterator it = _engines.begin(); it != _engines.end(); ++it )
            ( *it )->notify();
         if ( _father )
            _father->notifyChanges();
      }

      /**
       @brief Attach a resource to an engine. It means each time a resource is changed, the engine needs
              to recalculate everything.
       */
      void attach( EngineRunnable* resource )
      {
         _engines.insert( resource );
      }

      /**
       @brief must be called when an engine doesn't use the resource anymore
       */
      void detach( EngineRunnable* resource )
      {
         _engines.erase( resource );
      }

   protected:
      Engines           _engines;
      DynamicResource*  _father;
   };
}

#endif
