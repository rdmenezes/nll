#ifndef MVV_PLATFORM_ORDER_CREATOR_H_
# define MVV_PLATFORM_ORDER_CREATOR_H_

# include "mvvPlatform.h"
# include "order-provider.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Defines an order creator.
    */
   class MVVPLATFORM_API OrderCreator
   {
   public:
      /**
       @brief Construct an order creator. The only requirement for this is that it can push an order on the
              order provider execution queue.
       */
      OrderCreator( OrderProvider& provider ) : _orderProvider( provider )
      {}

      virtual ~OrderCreator()
      {}

   private:
      // copy disabled
      OrderCreator( const OrderCreator& );
      OrderCreator& operator=( const OrderCreator& );

   protected:
      OrderProvider&  _orderProvider;
   };
}
}
#endif