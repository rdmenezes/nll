#ifndef MVV_QUEUE_ORDERS_H_
# define MVV_QUEUE_ORDERS_H_

# include "mvv.h"
# include "order.h"
# include <map>
# include <boost/signals2/signal.hpp>
# include <boost/thread/mutex.hpp>

namespace mvv
{
   /**
    @ingroup mvv
    @brief QueueOrders interface is responsible for holding orders and their result
    */
   class MVV_API QueueOrdersInterface
   {
   public:
      typedef boost::signals2::signal< void( OrderInterface* ) >  OnOrderAdded;
      typedef OnOrderAdded::slot_type                             OnOrderAddedSlot;
      typedef boost::signals2::signal< void( OrderInterface* ) >  OnOrderToBeProcessed;
      typedef OnOrderAdded::slot_type                             OnOrderToBeProcessedSlot;

   public:
      virtual ~QueueOrdersInterface()
      {}

      /**
       @brief Add an order on the queue to be handled
       */
      virtual void registerOrder( OrderInterface* order ) = 0;

      /**
       @brief registers the functor to be notified for this event
       */
      void doOnActionAdded( const OnOrderAddedSlot& slot )
      {
         _onOrderAdded.connect( slot );
      }

      /**
       @brief on order finished, send it to the postprocessor manager
       */
      virtual void doOnOrderFinished( const OnOrderToBeProcessedSlot& slot )
      {
         _onOrderToBeProcessed.connect( slot );
      }

      virtual void onOrderFinished( OrderInterface* order ) = 0;

   protected:
      OnOrderAdded         _onOrderAdded;
      OnOrderToBeProcessed _onOrderToBeProcessed;
   };

   /**
    @ingroup mvv
    @brief An implementation of <code>QueueOrdersInterface</code>
    @note orders are sorted by id and not by creation time
    */
   class MVV_API QueueOrders : public QueueOrdersInterface
   {
      typedef std::map< ui32, OrderInterface* >  Orders;

   public:
      virtual void registerOrder( OrderInterface* order )
      {
         boost::mutex::scoped_lock lock( _mutex );
         _orders[ order->getId() ] = order;
         _onOrderAdded( order );
      }

      virtual void onOrderFinished( OrderInterface* order )
      {
         _onOrderToBeProcessed( order );
      }

   protected:
      Orders         _orders;
      boost::mutex   _mutex;
   };
}

#endif
