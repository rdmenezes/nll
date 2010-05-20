#ifndef MVV_PLATFORM_ORDER_DISPATCHER_IMPL_H_
# define MVV_PLATFORM_ORDER_DISPATCHER_IMPL_H_

# include "mvvPlatform.h"
# include "order-dispatcher.h"
# include "order-consumer.h"
# include <map>

namespace mvv
{
namespace platform
{
   /**
    @brief Implementation of an OrderDispatcher interface.
    */
   class MVVPLATFORM_API OrderDispatcherImpl : public OrderDispatcher
   {
      typedef std::set<OrderConsumer*>                Storage;
      typedef std::map<Order::OrderClassId, Storage>  Consumers;
      typedef std::set<Order::OrderClassId>           Buckets;

   public:
      /**
       @brief Distach an order to some consumers. The implementation is taking advantage of the
              <code>interestedOrder</code> of the consumer to not dispatch an order to all consumers
       */
      virtual void dispatch( Order* order )
      {
         Consumers::iterator it = _consumers.find( ( *order ).getClassId() );
         if ( it != _consumers.end() )
         {
            for ( Storage::iterator sit = it->second.begin(); sit != it->second.end(); ++sit )
            {
               ( *sit )->consume( order );
            }
         }

         // the order has been dispatched, if another thread is waiting for the result we need to unlock the mutex
         //order->getMutex().unlock();
      }

      /**
       @brief Register a consumer that will consume the orders
       */
      virtual void connect( OrderConsumer* consumer )
      {
         Buckets::const_iterator it = consumer->interestedOrder().begin();
         for ( ; it != consumer->interestedOrder().end(); ++it )
         {
            _consumers[ *it ].insert( consumer );
         }
      }

      /**
       @brief Remove a consumer from the distacher list
       */
      virtual void disconnect( OrderConsumer* consumer )
      {
         for ( Buckets::const_iterator it = consumer->interestedOrder().begin(); it != consumer->interestedOrder().end(); ++it )
         {
            Consumers::iterator c = _consumers.find( *it );
            if ( c != _consumers.end() )
            {
               c->second.erase( consumer );
            }
         }
      }

      virtual ~OrderDispatcherImpl()
      {
      }

   protected:
      Consumers  _consumers;
   };
}
}

#endif
