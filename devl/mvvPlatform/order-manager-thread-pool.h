#ifndef MVV_PLATFORM_ORDER_MANAGER_THREAD_POOL_H_
# define MVV_PLATFORM_ORDER_MANAGER_THREAD_POOL_H_

# include "types.h"
# include "mvvPlatform.h"
# include "order-provider-impl.h"
# include "order-dispatcher-impl.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Receive, execute and dispatch orders to consumers using a pool of threads

    The typical process is:
    1- the main thread push orders on a queue
    2- a manager thread read the queue, and copy the orders to its internal queue. The manager thread will
      then allocate a worker thread
    3- the worker thread run the order. When it is done, it notifies the pool
    4- the manager thread will copy the orders run to the main thread
    5- the main thread will dispatch the orders

    2-5 is done in the implementation of OrderManagerThreadPool
    */
   class MVVPLATFORM_API OrderManagerThreadPool : public OrderProviderImpl, public OrderDispatcherImpl
   {
   public:
      /**
       @brief Create a pool of threads
       @param nbWorkerThread the number of worker thread. Typically, about the same than the number of CPU
       */
      OrderManagerThreadPool( ui32 nbWorkerThread ) : _nbWorkerThread( nbWorkerThread )
      {}

      /**
       @brief Kill all the workers, the current queued orders are discarded.
       @note orders currently executed are still run, only the new ones won't be launched. After the last worker is done,
             the object is destroyed.

             The call will block until the pool is entirely destroyed.
       */
      void kill();

      /**
       @brief Start a new cycle: first order queue will be send to the order manager thread. The order manager thread
              will be notified and run orders if resources are available. Finally orders that have been computed
              will be dispatched to the consumers interested.
       */
      void run();

   protected:
      ui32     _nbWorkerThread;
   };
}
}

#endif