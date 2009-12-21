#ifndef MVV_PLATFORM_ORDER_CONSUMER_H_
# define MVV_PLATFORM_ORDER_CONSUMER_H_

# include "mvvPlatform.h"
# include "order.h"
# include "order-dispatcher.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Consume an order
    */
   class MVVPLATFORM_API OrderConsumer
   {
   public:
      typedef Order::OrderClassId   OrderClassId;

   public:
      /**
       @brief default constructor
       */
      OrderConsumer()
      {}

      /**
       @brief Consume an order, there is no order guaranteed for the execution order (except for predecessors)
       */
      virtual void consume( RefcountedTyped<Order> order ) = 0;

      /**
       @brief Returns the type of order the consumer will handle. (orders are dispatched according to this)
       */
      virtual const std::set<OrderClassId>& interestedOrder() const = 0;

   private:
      // copy disabled: we don't wan't to automatically register the new consumer to the dispatcher...
      OrderConsumer( const OrderConsumer& d );
      OrderConsumer& operator=( const OrderConsumer& o );
   };
}
}

#endif