#ifndef MVV_ORDER_MANAGER_H_
# define MVV_ORDER_MANAGER_H_

# include <set>
# include "order.h"

namespace mvv
{
   /**
    @ingroup mvv
    @brief Manage and digest orders
    */
   class OrderManagerInterface
   {
   public:
      /**
       @brief Construct an order manager
       @param interstedInOrders a set of order in which the manager is able to handle
       */
      OrderManagerInterface( const std::set<OrderId>& interstedInOrders ) : _interested( interstedInOrders )
      {}

      virtual ~OrderManagerInterface()
      {}

      /**
       @brief Consume a command.
       @return true if the manager has correctly handled this order
       */
      virtual bool consume( const OrderInterface* order ) const = 0;

   protected:
      std::set<OrderId>    _interested;
   };
}

#endif
