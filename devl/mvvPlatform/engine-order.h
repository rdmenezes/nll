#ifndef MVV_PLATFORM_ENGINE_ORDER_H_
# define MVV_PLATFORM_ENGINE_ORDER_H_

# include "mvvPlatform.h"
# include "engine.h"
# include "order-creator.h"
# include "order-consumer.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief It is quite common for an order to use resource as data provider, send order for asynchronous computations
           and update its output according to the order's result.

           Typically, the engine acquires resources using <code>register</code>
    */
   class EngineOrder : public Engine, OrderCreator, OrderConsumer
   {
   public:
      /**
       @brief Instanciate an EngineOrder. The engine will be connected to the necessary components, and disconnected when
              destroyed.
       */
      EngineOrder( EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : Engine( handler ), OrderCreator( provider ), OrderConsumer( dispatcher )
      {
         _dispatcher.connect( *this );
      }

      virtual ~EngineOrder()
      {
         _dispatcher.disconnect( *this );
      }
   };
}
}

#endif