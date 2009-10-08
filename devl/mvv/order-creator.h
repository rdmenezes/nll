#ifndef MVV_ORDER_CREATOR_H_
# define MVV_ORDER_CREATOR_H_

# include "order.h"

namespace mvv
{
   class OrderCreator
   {
   public:
      virtual void consume( Order* ) = 0;

      virtual void run() = 0;
   };
}

#endif