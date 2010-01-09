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

           All engines must be disconnected before the handler is destroyed
    */
   class MVVPLATFORM_API EngineOrder : public Engine, public OrderCreator, public OrderConsumer
   {
   public:
      /**
       @brief Instanciate an EngineOrder. The engine will be connected to the necessary components, and disconnected when
              destroyed.
       */
      EngineOrder( EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : Engine( handler ), OrderCreator( provider ), _dispatcher( dispatcher )
      {
         //_dispatcher.connect( *this );  // We can't do that: the base is initialized before the 'child', and seen as pure virtual
      }

      virtual ~EngineOrder()
      {
         //_dispatcher.disconnect( *this ); // We can't do that: the base is initialized before the 'child', and seen as pure virtual
      }

   protected:
      OrderDispatcher& _dispatcher;
   };
}
}

#endif