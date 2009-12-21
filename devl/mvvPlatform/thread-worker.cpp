#include "thread-worker.h"
#include "thread-pool.h"

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression constant

namespace mvv
{
namespace platform
{
   void ThreadWorker::run( RefcountedTyped<Order> order )
   {
      boost::mutex::scoped_lock lock( _mutex );

      ensure( &order, "must not be null" );
      ensure( _hasFinished, "error: the thread must be idle!" );

      _currentOrder = order;
      _hasFinished = false;
      notify();
   }

   void ThreadWorker::operator()()
   {
      boost::mutex::scoped_lock lock( _mutex );
      try
      {
         while ( 1 )
         {
            // wait for a new job
            _pool->notify();
            while ( _hasFinished )
            {
               _notified.wait( lock );
            }

            ensure( &_currentOrder, "something went wrong..." );
            std::cout << clock() / (double)CLOCKS_PER_SEC << " worker=" << _workerId << " run, idorder=" << (*_currentOrder).getId() << std::endl;

            // run the job
            nll::core::Timer todoDebug;
            _run();
            std::cout << clock() / (double)CLOCKS_PER_SEC << " worker=" << _workerId << " end, time=" << todoDebug.getCurrentTime() << " classId=" << (*_currentOrder).getClassId().getName() << std::endl;
            _currentOrder.unref();
         }
      }
      catch ( boost::thread_interrupted )
      {
         std::cout << "interrupted" << std::endl;
         return;
      }
   }

   ThreadWorker::ThreadWorker( ThreadPool* pool, ui32 workerId ) : _pool( pool ), _workerId( workerId ), _hasFinished( true ), _currentOrder( 0 )
   {
      ensure( pool, "error: null pointer" );
   }

   void ThreadWorker::_run()
   {
      // locked by operator()
      nll::core::Timer timerDebug;
      (*_currentOrder).compute();
      std::cout << "Job only=" << timerDebug.getCurrentTime() << std::endl;
      ensure( (*_currentOrder).getResult(), "result requires not to be null" );
      _hasFinished = true;
      _pool->workerFinished( _currentOrder, _workerId );
   }
}
}

# pragma warning( pop )