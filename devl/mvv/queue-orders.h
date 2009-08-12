#ifndef MVV_QUEUE_ORDERS_H_
# define MVV_QUEUE_ORDERS_H_

# include "mvv.h"
# include "order.h"
# include <map>

namespace mvv
{
   /**
    @ingroup mvv
    @brief QueueOrders interface is responsible for holding orders and their result
    */
   class MVV_API QueueOrdersInterface
   {
   public:
      typedef std::map<ui32, OrderInterface*>   Orders;

   public:
      virtual ~QueueOrdersInterface()
      {}

      /**
       @brief register a new Order. It is inserted in the list of orders according to its id.
              The smallest indices are computed first.
       */
      virtual void registerOrder( OrderInterface* order ) = 0;
   };
}

#endif
