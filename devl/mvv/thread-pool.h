#ifndef MVV_THREAD_POOL_H_
# define MVV_THREAD_POOL_H_

# include <stack>
# include "order.h"
# include "thread-worker.h"

namespace mvv
{
   // forward declaration
   class Worker;

   /**
    @ingroup mvv
    @brief Manages a pool of threads
    */
   class MVV_API ThreadPool
   {
   public:
      typedef std::set<Order*>            Orders;
      typedef std::vector<Worker*>        Workers;
      typedef std::vector<boost::thread*> WorkerThreads;
      typedef std::stack<Worker*>         AvailableWorkers;

   public:
      ThreadPool( ui32 numberOfWorkers );

      ~ThreadPool()
      {
      }

      /**
       @brief Notify the thread that it requires to do something
       */
      void notify();

      /**
       @notify the pool that a worker is idle
       */
      void workerFinished( Order* order, ui32 workerId );

      /**
       @brief Return the finished orders and clear the list
       */
      Orders getFinishedOrdersAndClearList();

      /**
       @brief run the order on a worker thread
       */
      void run( Order* order );

      /**
       @brief wait the thread to finish their last order and kill them
       */
      void kill();

   protected:
      boost::condition  _notified;
      boost::mutex      _mutex;
      Orders            _ordersFinished;
      Workers           _workers;
      WorkerThreads     _workerThreads;
      AvailableWorkers  _workersAvailable;
   };
}

#endif