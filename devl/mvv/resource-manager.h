#ifndef MVV_RESOURCE_MANAGER_H_
# define MVV_RESOURCE_MANAGER_H_

# include <boost/thread/mutex.hpp>
# include "resource.h"
# include "order-provider.h"
# include "notifiable.h"

namespace mvv
{
   /**
    @ingroup 
    @brief Holds all the resources for the viewer
    */
   class _ResourceManager : public OrderProvider
   {
   public:
      /**
       @brief return the current orders to be handled. This method is synchronized.
              Use case: pushOrder push an order on the queue, then the dispatcher thread (queue order) pick the orders
       */
      virtual Orders getOrdersAndClear()
      {
         boost::mutex::scoped_lock lock( _mutex );
         Orders o = _orders;
         _orders.clear();
         return o;
      }

      /**
       @brief Push an order to the queue. This order must be picked by the dispatcher thread (queue-order)
              That will handle its lifecycle.
       */
      virtual void pushOrder( Order* order )
      {
         boost::mutex::scoped_lock lock( _mutex );
         _orders.push_back( order );
         if ( _queue )
            _queue->notify();
      }

      /**
       @brief hold a notifiable object that will be notified when an order has been pushed on the current stack of orders.
       */
      void setQueueOrder( Notifiable* queue )
      {
         _queue = queue;
      }

   private:
      boost::mutex      _mutex;
      Orders            _orders;
      Notifiable*       _queue;
   }; 

   typedef nll::core::Singleton<_ResourceManager>  ResourceManager;
}

#endif