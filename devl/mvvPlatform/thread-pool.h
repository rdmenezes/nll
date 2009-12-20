#ifndef MVV_PLATFORM_THREAD_POOL_H_
# define MVV_PLATFORM_THREAD_POOL_H_

# include <stack>

# pragma warning( push )
# pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

# include <boost/thread/mutex.hpp>
# include <boost/thread/condition.hpp>

# pragma warning( pop )

# include "types.h"
# include "mvvPlatform.h"
# include "notifiable.h"
# include "order.h"
# include "refcounted.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Run orders on a pool of thread. This class must be run on the manager thread
    */
   class MVVPLATFORM_API ThreadPool : public Notifiable
   {
      friend class ThreadWorker;

   public:
      typedef std::vector< RefcountedTyped<Order> >   Orders;
      typedef std::vector<ThreadWorker*>              Workers;
      typedef std::vector<boost::thread*>             WorkerThreads;
      typedef std::stack<ThreadWorker*>               AvailableWorkers;

      /**
       @brief Construct the pool.
       @param nbWorkers the number of orders that can be run simultaneaously
      // @param mainThread the class on main thread that must be notified when an order has been run
       */
      ThreadPool( ui32 nbWorkers ); // TODO CHECK , Notifiable& mainThread

      ~ThreadPool()
      {
         kill();
      }

      /**
       @brief Kills the workers and discard the current queue orders. Orders being run by a worker are executed, after
              this, the order is destroyed. The call will block until the pool is entirely destroyed.
       */
      void kill();

      /**
       @brief Return the finished orders and clear the list.
       */
      Orders getFinishedOrdersAndClearList();

      /**
       @brief Push an order on the execution queue ( run on main thread )
       */
      void push( RefcountedTyped<Order> order );

      /**
       @brief run the infiny loop of the manager thread
       */
      void operator()();

      ui32 getNumberOfOrdersToRun() const
      {
         return static_cast<ui32>( _ordersToRun.size() );
      }

 //  protected:
      /**
       @brief Notify the thread that it requires to do something
       */
      void notify();

      /**
       @brief run the order on a worker thread
       */
      void dispatchToWorker( RefcountedTyped<Order> order );

      /**
       @brief notify the pool that a worker is idle
       */
      void workerFinished( RefcountedTyped<Order> order, ui32 workerId );

      void _check();

   private:
      // copy disabled
      ThreadPool& operator=( const ThreadPool& );
      ThreadPool( const ThreadPool& );

   private:
      boost::condition  _notified;
      bool              _notified2;
      boost::mutex      _mutex;
      boost::mutex      _mutexWait;
      Orders            _ordersFinished;
      Orders            _ordersToRun;
      Workers           _workers;
      WorkerThreads     _workerThreads;
      AvailableWorkers  _workersAvailable;
     // Notifiable&       _mainThread;
   };
}
}

#endif
