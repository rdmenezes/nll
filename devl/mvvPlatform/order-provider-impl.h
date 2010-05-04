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
         Orders o;
         o = _execWaitList;
         _execWaitList.clear();
         return o;
      }

      /**
       @brief The client will push orders on a stack to be run by the thread pool. <code>getOrdersAndClear</code> will return this stack and empty it
       @param order must be a valid pointer for its whole lifecycle
       */
      virtual void pushOrder( Order* order )
      {
         _execWaitList.push_back( order );
      }

      /**
       @brief Returns true if orders have not yet started
       */
      virtual bool hasOrdersWaiting()
      {
         return _execWaitList.size() != 0;
      }

   protected:
      Orders   _execWaitList;
   };
}
}

#endif