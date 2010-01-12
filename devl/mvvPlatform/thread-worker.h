#ifndef MVV_PLATFORM_THREAD_WORKER_H_
# define MVV_PLATFORM_THREAD_WORKER_H_

# pragma warning( push )
# pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data
# include <boost/thread/mutex.hpp>
# include <boost/thread/condition.hpp>
# pragma warning( pop )

# include "mvvPlatform.h"
# include "refcounted.h"
# include "order.h"
# include "types.h"
# include <nll/nll.h>


namespace mvv
{
namespace platform
{
   // forward declaration
   class ThreadPool;

   class MVVPLATFORM_API ThreadWorker
   {
   public:
      ThreadWorker( ThreadPool* pool, ui32 workerId );

      virtual ~ThreadWorker()
      {}

      /**
       @brief notify the thread. Run on manager thread
       */
      void notify()
      {
         // we don't lock it as we don't want to block the calling thread!
         _notified.notify_one();
      }

      /**
       @brief Specify the function we want to run when this thread is done! run on manager thread
       */
      void run( Order* order );

      /**
       @brief infiny loop run on the worker thread.
       */
      void operator()();

   private:
      // copy disabled
      ThreadWorker& operator=( const ThreadWorker& );
      ThreadWorker( const ThreadWorker& );

   private:
      ThreadPool*       _pool;
      ui32              _workerId;
      boost::condition  _notified;
      boost::mutex      _mutex;
      bool              _isReady;
      bool              _hasFinished;
      Order*            _currentOrder;
   };
}
}

#endif
