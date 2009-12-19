#ifndef MVV_PLATFORM_ORDER_PROVIDER_H_
# define MVV_PLATFORM_ORDER_PROVIDER_H_

# include <vector>
# include "mvvPlatform.h"
# include "order.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup mvv
    @brief Define a base where orders are pushed to be executed asynchronously
    */
   class MVVPLATFORM_API OrderProvider
   {
   public:
      typedef std::vector< RefcountedTyped<Order> >  Orders;

   public:
      /**
       @brief Returns all stacked orders and clear the queue
       */
      virtual Orders getOrdersAndClear() = 0;

      /**
       @brief The client will push orders on a stack to be run by the thread pool. <code>getOrdersAndClear</code> will return this stack and empty it
       */
      virtual void pushOrder( RefcountedTyped<Order> order ) = 0;
   };
}
}

#endif