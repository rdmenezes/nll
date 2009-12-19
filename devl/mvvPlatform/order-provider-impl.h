#ifndef MVV_PLATFORM_ORDER_PROVIDER_IMPL_H_
# define MVV_PLATFORM_ORDER_PROVIDER_IMPL_H_

# include "mvvPlatform.h"
# include "order-provider.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief This class manage orders: orders must be pushed on the execution queue
    */
   class MVVPLATFORM_API OrderProviderImpl : public OrderProvider
   {
   public:
      /**
       @brief Returns all stacked orders and clear the queue
       */
      virtual Orders getOrdersAndClear()
      {
         Orders o = _execWaitList;
         _execWaitList.clear();
         return o;
      }

      /**
       @brief The client will push orders on a stack to be run by the thread pool. <code>getOrdersAndClear</code> will return this stack and empty it
       */
      virtual void pushOrder( RefcountedTyped<Order> order )
      {
         _execWaitList.push_back( order );
      }

   protected:
      Orders   _execWaitList;
   };
}
}

#endif