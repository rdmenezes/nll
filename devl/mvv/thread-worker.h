#ifndef MVV_THREAD_WORKER_H_
# define MVV_THREAD_WORKER_H_

# include <boost/thread/mutex.hpp>
# include <boost/thread/condition.hpp>
# include "order.h"
# include "thread-pool.h"
# include "notifiable.h"


namespace mvv
{
   // forward declaration
   class ThreadPool;

   /**
    @ingroup mvv
    @brief Worker class to be run on a worker thread.
    */
   class Worker : public Notifiable
   {
   public:
      /**
       @param pool the pool that manages the current thread
       */
      Worker( ThreadPool* pool, ui32 workerId );

      virtual ~Worker()
      {}

      void notify()
      {
         // we don't lock it as we don't want to block the calling thread!
         _notified.notify_one();
      }

      /**
       @brief Specify the function we want to run when this thread is done!
       */
      void run( Order* order )
      {
         ensure( _hasFinished, "error: the thread must be idle!" );

         boost::mutex::scoped_lock lock( _mutex );
         _currentOrder = order;
         _hasFinished = false;
         notify();
      }

      void operator()()
      {
         boost::mutex::scoped_lock lock( _mutex );
         try
         {
            while ( 1 )
            {
               // wait for a new job
               while ( _hasFinished )
               {
                  std::cout << "worker=" << _workerId << " wait" << std::endl;
                  _notified.wait( lock );
               }

               ensure( _currentOrder, "something went wrong..." );
               std::cout << "worker=" << _workerId << " run, idorder=" << _currentOrder->getId() << std::endl;

               // run the job
               _run();
            }
         }
         catch ( boost::thread_interrupted )
         {
            std::cout << "interrupted" << std::endl;
            return;
         }
      }

   protected:
      void _run();

   protected:
      ThreadPool*       _pool;
      ui32              _workerId;
      boost::condition  _notified;
      boost::mutex      _mutex;
      bool              _hasFinished;
      Order*            _currentOrder;
   };
}

#endif