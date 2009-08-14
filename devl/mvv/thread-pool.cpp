#include "stdafx.h"
#include "thread-pool.h"
#include <boost/ref.hpp>

namespace mvv
{
   ThreadPool::ThreadPool( ui32 numberOfWorkers )
   {
      _workers = Workers( numberOfWorkers );
      _workerThreads = WorkerThreads( numberOfWorkers );
      for ( ui32 n = 0; n < numberOfWorkers; ++n )
      {
         _workers[ n ] = new Worker( this, n );
         _workersAvailable.push( _workers[ n ] );
         _workerThreads[ n ] = new boost::thread( boost::ref( *_workers[ n ] ) );
      }
   }

   void ThreadPool::notify()
   {
      // we don't lock it as we don't want to block the calling thread!
      _notified.notify_one();
   }

   void ThreadPool::workerFinished( Order* order, ui32 workerId )
   {
      boost::mutex::scoped_lock lock( _mutex );
      _ordersFinished.insert( order );
      _workersAvailable.push( _workers[ workerId ] );
      notify();
   }

   ThreadPool::Orders ThreadPool::getFinishedOrdersAndClearList()
   {
      boost::mutex::scoped_lock lock( _mutex );
      Orders o = _ordersFinished;
      _ordersFinished.clear();
      return o;
   }

   void ThreadPool::run( Order* order )
   {
      boost::mutex::scoped_lock lock( _mutex );

      // not idle treads, we have to wait
      while ( _workersAvailable.empty() )
         _notified.wait( lock );

      // we have a thread available
      Worker* worker = _workersAvailable.top();
      _workersAvailable.pop();
      worker->run( order );
   }

   void ThreadPool::kill()
   {
      for ( ui32 n = 0; n < _workers.size(); ++n )
      {
         _workerThreads[ n ]->interrupt();
         _workerThreads[ n ]->join();
      }
   }
}