#include "stdafx.h"
#include "thread-pool.h"
#include <boost/ref.hpp>

namespace mvv
{
   ThreadPool::ThreadPool( ui32 numberOfWorkers, Notifiable* notifiable )
   {
      _notifiable = notifiable;
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

      if ( _notifiable )
      {
         std::cout << "notify queue" << std::endl;
         _notifiable->notify();
      }
   }

   void ThreadPool::workerFinished( Order* order, ui32 workerId )
   {
      boost::mutex::scoped_lock lock( _mutex );
      _ordersFinished.push_back( order );
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

      // no idle threads, we have to wait
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
         std::cout << "pool intrrupt" << std::endl;
         _workerThreads[ n ]->interrupt();
         std::cout << "pool join" << std::endl;
         _workerThreads[ n ]->join();
         std::cout << "pool joined" << std::endl;
         delete _workers[ n ];
         delete _workerThreads[ n ];
      }

      _workers.clear();
      _workerThreads.clear();
      _workersAvailable = AvailableWorkers();
      std::cout << "end pool kill" << std::endl;
   }
}