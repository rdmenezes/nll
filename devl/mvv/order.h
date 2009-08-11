#ifndef MVV_ORDER_H_
# define MVV_ORDER_H_

# include "mvv.h"
# include "types.h"
# include <nll/nll.h>

namespace mvv
{
   /**
    @ingroup mvv
    @brief defines an order type that will be used to recognize this type
    */
   enum MVV_API OrderId
   {
   };

   /**
    @ingroup mvv
    @brief Contain orders and their result
    */
   class MVV_API OrderInterface
   {
   public:
      /**
       @brief Construct an empty order
       @param orderId the id of the type of order
       */
      OrderInterface( OrderId orderId ) : _id( nll::core::IdMaker::instance().generateId() ), _orderId( orderId )
      {}

      ~OrderInterface()
      {}

      /**
       @return the order instance id
       */
      ui32 getId() const
      {
         return _id;
      }

      /**
       @return the id of the kind of order
       */
      OrderId getOrderId() const
      {
         return _orderId;
      }

   protected:
      ui32     _id;
      OrderId  _orderId;
   };
}

#endif
