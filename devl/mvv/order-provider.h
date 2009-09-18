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

      /**
       @brief push orders on a stack. <code>getOrdersAndClear</code> will return this stack and empty it
       */
      virtual void pushOrder( Order* order ) = 0;

      /**
       @brief Returns the a list of orders that need to be destroyed
       */
      virtual Orders getOrdersToDestroyAndClear() = 0;

      /**
       @brief Push an order that will be deallocated. It is assumed the order has finished executing, else this is not valid!
       */
      virtual void pushOrderToDestroy( Order* order ) = 0;
   };
}

#endif
