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
      typedef std::vector< Order* >  Orders;

   public:
      /**
       @brief Returns all stacked orders and clear the queue
       */
      virtual Orders getOrdersAndClear() = 0;

      /**
       @brief The client will push orders on a stack to be run by the thread pool. <code>getOrdersAndClear</code> will return this stack and empty it
       @param order must be a valid pointer for its whole lifecycle
       */
      virtual void pushOrder( Order* order ) = 0;

      /**
       @brief Returns true if orders have not yet started
       */
      virtual bool hasOrdersWaiting() = 0;
   };
}
}

#endif