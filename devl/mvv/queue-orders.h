#ifndef MVV_QUEUE_ORDERS_H_
# define MVV_QUEUE_ORDERS_H_

# include "mvv.h"
# include "order.h"
# include <queue>

namespace mvv
{
   /**
    @brief QueueOrders interface is responsible for holding orders and their result
    */
   class MVV_API QueueOrdersInterface
   {
   public:
      typedef std::queue<QueueOrdersInterface*>   Orders;

   public:
      virtual ~QueueOrdersInterface()
      {}
   };
}

#endif
