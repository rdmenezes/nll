#ifndef MVV_PLATFORM_ORDER_DISPATCHER_H_
# define MVV_PLATFORM_ORDER_DISPATCHER_H_

# include "mvvPlatform.h"
# include "order.h"
//# include "order-consumer.h"

namespace mvv
{
namespace platform
{
   /// forward declaration
   class OrderConsumer;

   /**
    @brief When an order have been run, the result must be dispatched to the consummer
    @note the client must guarantee all registered orders are valid or released if not!
    */
   class MVVPLATFORM_API OrderDispatcher
   {
   public:
      /**
       @brief Distach an order to some consumers. The implementation should take advantage of the
              <code>interestedOrder</code> of the consumer to not dispatch an order to all consumers
       */
      virtual void dispatch( RefcountedTyped<Order> order ) = 0;

      /**
       @brief Register a consumer that will consume the orders
       */
      virtual void connect( OrderConsumer& consumer ) = 0;

      /**
       @brief Remove a consumer from the distacher list
       */
      virtual void disconnect( OrderConsumer& consumer ) = 0;

      virtual ~OrderDispatcher()
      {
      }
   };
}
}

#endif