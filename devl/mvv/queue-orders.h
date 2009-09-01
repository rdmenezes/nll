#ifndef MVV_QUEUE_ORDERS_H_
# define MVV_QUEUE_ORDERS_H_

# include "mvv.h"
# include "order.h"
# include "order-provider.h"
# include "thread-pool.h"
# include "notifiable.h"

namespace mvv
{
   /**
    @brief This class is run on the dispatcher thread. It will pick orders from an OrderProvider and
           send it to a pool to be executed. All the orders are stored here and will be deleted at the
           end of this object.
    */
   class QueueOrder : public Notifiable
   {
   public:
      typedef std::map<ui32, Order*>   OrderStorage;
      typedef std::vector<Order*>      OrderBuffer;

   public:
      /**
       @brief Construct the queue with a pool.

       Each time an order has finished, this thread is notified.

       Note: "this" pointer is used in constructor, however it is guaranteed that the thread pool won't call
             any method of the queue until it is fully constructed, so it is valid to do so.
       */
      QueueOrder( OrderProvider& orderProvider, ui32 poolSize ) : _orderProvider( orderProvider ), _pool( poolSize, this )
      {
         _notified2 = false;
      }

      virtual ~QueueOrder()
      {
         kill();
      }

      /**
       @brief notify the thread to pick the orders and send them to the pool.
       */
      void notify()
      {
         _notified.notify_one();
         _notified2 = true;
      }

      /**
       @brief Threading method. Each time it is notified, it will pick the orders and send them
              to the pool
       */
      void operator()()
      {
         boost::mutex::scoped_lock lock( _mutex );
         try
         {
            while (1)
            {
               std::cout << "queue:queue waiting" << std::endl;
               if ( !_notified2 )
                  _notified.wait( _mutex );
               _notified2 = false;
               std::cout << "queue:check orders" << std::endl;

               // get the orders, and take them to a buffer
               OrderProvider::Orders newOrders = _orderProvider.getOrdersAndClear();
               for ( OrderProvider::Orders::iterator it = newOrders.begin(); it != newOrders.end(); ++it )
               {
                  _orders[ (*it)->getId() ] = *it;
                  _buffer.push_back( *it );
               }

               // for each order test if successors have been run, else skip the order
               for ( OrderBuffer::iterator it = _buffer.begin(); it != _buffer.end(); )
               {
                  std::cout << "queue:start run check=" << (*it)->getId() << std::endl;
                  // check predecessors
                  bool skip = false;
                  const Order::Predecessors& p = ( *it )->getPredecessors();
                  for ( Order::Predecessors::const_iterator pred = p.begin(); pred != p.end(); ++pred )
                     if (! ( _orders[ *pred ] )->getResult() )
                     {
                        skip = true;
                        break;
                     }
                  // a predecessor has finished
                  if ( skip )
                  {
                     ++it;
                     continue;
                  }

                  // we can now run them
                  std::cout << "queue:run" << (*it)->getId() << std::endl;
                  if ( ( *it )->toBeMultithreaded() )
                     _pool.run( *it );
                  else
                  {
                     // run it on the same thread
                     ( *it )->setResult( ( *it )->run() );
                  }
                  std::cout << "queue:run launched" << std::endl;
                  OrderBuffer::iterator toErase = it;
                  it = _buffer.erase( toErase );
               }
            }
         }
         catch ( boost::thread_interrupted )
         {
            std::cout << "interrupted" << std::endl;
            return;
         }
      }

      /**
       @brief destroy all the current orders and release memory
              Destroy the pool. After this call the Queue won't work anymore.
       */
      void kill()
      {
         ensure( !_buffer.size(), "orders still in queue..." );
         std::cout << "queue: try kill" << std::endl;
         boost::mutex::scoped_lock lock( _mutex );
         std::cout << "queue: kill" << std::endl;

         std::cout << "queue: pool kill" << std::endl;
         _pool.kill();

         for ( OrderStorage::iterator it = _orders.begin(); it != _orders.end(); ++it )
            delete it->second;
         _orders.clear();
      }

      /**
       @brief get the orders that have been executed. It is a sequence, and it is guaranteed the predecessors
              have been placed before in the list.
       */
      OrderBuffer getFinishedOrdersAndClear()
      {
         boost::mutex::scoped_lock lock( _mutex );
         return _pool.getFinishedOrdersAndClearList();
      }

   protected:
      OrderProvider&     _orderProvider;
      ThreadPool         _pool;
      boost::mutex       _mutex;
      boost::condition   _notified;
      bool               _notified2;
      OrderStorage       _orders;
      OrderBuffer        _buffer;
      OrderBuffer        _executed;
   };
}

#endif
