#pragma warning( push )
#pragma warning( disable:4127 ) // conditional expression is constant
#pragma warning( disable:4512 ) // assignment operator not generated

#include "thread-pool.h"
#include "thread-worker.h"

namespace mvv
{
namespace platform
{

   ThreadPool::ThreadPool( ui32 numberOfWorkers )
   {
      // init the pool, create the workers...
      _workers = Workers( numberOfWorkers );
      _workerThreads = WorkerThreads( numberOfWorkers );
      for ( ui32 n = 0; n < numberOfWorkers; ++n )
      {
         _workers[ n ] = new ThreadWorker( this, n );
         _workersAvailable.push( _workers[ n ] );
         _workerThreads[ n ] = new boost::thread( boost::ref( *_workers[ n ] ) );
      }
      _notified2 = false;
   }

   void ThreadPool::notify()
   {
      // we don't lock it as we don't want to block the calling thread!
      _notified.notify_one();
      _notified2 = true;

      //std::cout << "notify()" << std::endl;
      //std::cout << clock() / (double)CLOCKS_PER_SEC << "Notified pool" << std::endl;
   }

   void ThreadPool::workerFinished( RefcountedTyped<Order> order, ui32 workerId )
   {
      // we need to synchronize as we update internal data
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

   void ThreadPool::push( RefcountedTyped<Order> order )
   {
      // we need to synchronize as we update internal data
      boost::mutex::scoped_lock lock( _mutex );
      _ordersToRun.push_back( order );

      // we need to wake up the manager thread: an order arrived
      //std::cout << "pool.push=" << (*order).getClassId().getName() << std::endl;
      notify();
   }

   void ThreadPool::dispatchToWorker( RefcountedTyped<Order> order )
   {
      boost::mutex::scoped_lock lock( _mutex );

      // we have a thread available
      ThreadWorker* worker = _workersAvailable.top();
      _workersAvailable.pop();
      worker->run( order );
      //std::cout << "pool.dispatchworker=" << (*order).getClassId().getName() << std::endl;
   }

   void ThreadPool::kill()
   {
      std::cout << "kill" << std::endl;
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

   void ThreadPool::operator()()
   {
      // we need to lock our waiting mutex
      boost::mutex::scoped_lock lock( _mutexWait );
      try
      {
         std::cout << ( (float)clock() / CLOCKS_PER_SEC ) << " Pool is in main loop" << std::endl;
         while (1)
         {
            while ( !_notified2 || _workersAvailable.empty() )
            {
               _notified.wait( _mutexWait );

               boost::mutex::scoped_lock lock( _mutex );
               if ( _notified2 && !_workersAvailable.empty() )
               {
                  break;
               }
               //std::cout << "checkcheck=" << _notified2 << std::endl;
            }

            {
               boost::mutex::scoped_lock lock( _mutex );
               _notified2 = false;
            }
            
            // a new order arrived, or an older one has finished
            // we now need to run the orders that need to be
            _check();
         }
      }
      catch ( boost::thread_interrupted )
      {
         std::cout << "interrupted" << std::endl;
         return;
      }
   }

   void ThreadPool::_check()
   {
      //std::cout << clock() / (double)CLOCKS_PER_SEC << "check" << std::endl;
      Orders orders;
      {
         boost::mutex::scoped_lock lock( _mutex );
         orders = _ordersToRun;
      }

      for ( Orders::iterator it = orders.begin(); it != orders.end(); ++it )
      {
         // check predecessors
         bool skip = false;
         Order::Predecessors p = ( **it ).getPredecessors();
         for ( Order::Predecessors::const_iterator pred = p.begin(); pred != p.end(); ++pred )
         {
            if (! ( **pred ).getResult() )
            {
               // if no result, it means one of its predecessors is not run, skip it!
               skip = true;
               //std::cout << "skip" << std::endl;
               break;
            }
         }
         if ( !skip )
         {
            if ( ( **it ).toBeMultithreaded() )
            {
               //std::cout << "dispatch" << std::endl;
               dispatchToWorker( *it );
            } else {
               // run it on the manager thread
               (**it).compute();

               // update result
               boost::mutex::scoped_lock lock( _mutex );
               _ordersFinished.push_back( *it );
            }

            // clean the order, we don't need it anymore. We need to synchronize
            boost::mutex::scoped_lock lock( _mutex );
            Orders::iterator toErase = std::find( _ordersToRun.begin(), _ordersToRun.end(), *it );
            assert( toErase != _ordersToRun.end() );
            _ordersToRun.erase( toErase );
         } else {
            continue;
         }

         // if no worker available, just wait
         if ( _workersAvailable.empty() )
         {
            break;
         }
      }
      //std::cout << "pool.check=" << orders.size() << std::endl;
   }
}
}

#pragma warning( pop )