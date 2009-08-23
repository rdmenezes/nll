#ifndef MVV_THREAD_POOL_H_
# define MVV_THREAD_POOL_H_

# include <stack>
# include "order.h"
# include "thread-worker.h"
# include "notifiable.h"

namespace mvv
{
   // forward declaration
   class Worker;

   /**
    @ingroup mvv
    @brief Manages a pool of threads
    */
   class MVV_API ThreadPool : public Notifiable
   {
   public:
      typedef std::vector<Order*>         Orders;
      typedef std::vector<Worker*>        Workers;
      typedef std::vector<boost::thread*> WorkerThreads;
      typedef std::stack<Worker*>         AvailableWorkers;

   public:
      /**
       @param numberOfWorkers the number of threads to create
       @param notifiable when an order is finished, and this variable is not null, then it is notified
                         (i.e. in the case of order dependencies the calling thread needs to be notified
                          when an order is done so that dependencies are checked again)
       */
      ThreadPool( ui32 numberOfWorkers, Notifiable* notifiable = 0 );

      ~ThreadPool()
      {
         kill();
      }

      /**
       @brief Notify the thread that it requires to do something
       */
      void notify();

      /**
       @brief notify the pool that a worker is idle
       */
      void workerFinished( Order* order, ui32 workerId );

      /**
       @brief Return the finished orders and clear the list. It is guaranteed predecessors have been set in the list
              before.
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
      Notifiable*       _notifiable;
   };
}

#endif