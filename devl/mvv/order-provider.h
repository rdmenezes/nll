#ifndef MVV_ORDER_PROVIDER_H_
# define MVV_ORDER_PROVIDER_H_

# include "order.h"
# include <vector>

namespace mvv
{
   /**
    @ingroup mvv
    @brief Provide the orders for the queue
    */
   class OrderProvider
   {
   public:
      typedef std::vector<Order*>  Orders;

   public:
      /**
       @brief get the orders to be executed and clear them. It must return allocated orders only.
              The list of orders is erased after this call and must be synchronized.
       */
      virtual Orders getOrdersAndClear() = 0;
   };
}

#endif
