#include "thread-pool.h"
#include "thread-worker.h"

namespace mvv
{
namespace platform
{

   ThreadPool::ThreadPool( ui32 numberOfWorkers ) // Notifiable& mainThread  TODO CHECK : _mainThread( mainThread )
   {
      _workers = Workers( numberOfWorkers );
      _workerThreads = WorkerThreads( numberOfWorkers );
      for ( ui32 n = 0; n < numberOfWorkers; ++n )
      {
         _workers[ n ] = new ThreadWorker( this, n );
         _workersAvailable.push( _workers[ n ] );
         _workerThreads[ n ] = new boost::thread( boost::ref( *_workers[ n ] ) );
      }
   }

   void ThreadPool::notify()
   {
      // we don't lock it as we don't want to block the calling thread!
      _notified.notify_one();

      // TODO check 
      //if ( _notifiable )
      //{
      //   _notifiable->notify();
      //}
   }

   void ThreadPool::workerFinished( RefcountedTyped<Order> order, ui32 workerId )
   {
      boost::mutex::scoped_lock lock( _mutex );
      _ordersFinished.push_back( order );
      _workersAvailable.push( _workers[ workerId ] );
      notify();
   }
}
}