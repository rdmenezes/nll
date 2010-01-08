#pragma warning( push )
#pragma warning( disable:4512 ) // assignment operator not generated

#include "order-manager-thread-pool.h"
#include "thread-pool.h"
#include <boost/thread/thread.hpp>

namespace mvv
{
namespace platform
{
   namespace impl
   {
      struct OrderManagerThreadPoolImpl
      {
         OrderManagerThreadPoolImpl( ui32 nbThread ) : pool( new ThreadPool( nbThread ) )
         {
         }

         boost::thread                 managerThread;
         RefcountedTyped<ThreadPool>   pool;
      };
   }

   OrderManagerThreadPool::OrderManagerThreadPool( ui32 nbWorkerThread ) : _impl( new impl::OrderManagerThreadPoolImpl( nbWorkerThread ) )
   {
      ( *_impl ).managerThread = boost::thread( boost::ref( *( *_impl ).pool ) );
   }

   OrderManagerThreadPool::~OrderManagerThreadPool()
   {
      kill();
   }

   void OrderManagerThreadPool::run()
   {
      // execute the orders
      Orders orders = getOrdersAndClear();
      for ( Orders::iterator it = orders.begin(); it != orders.end(); ++it )
      {
         //std::cout << "run order:" << (**it).getClassId().getName() << std::endl;
         ( *( *_impl ).pool ).push( *it );
      }

      // dispatch the orders already run
      orders = ( *( *_impl ).pool ).getFinishedOrdersAndClearList();
      for ( Orders::iterator it = orders.begin(); it != orders.end(); ++it )
      {
         std::cout << "dispatch order:" << (**it).getId() << std::endl;
         dispatch( *it );
      }
   }

   void OrderManagerThreadPool::kill()
   {
      if ( !_impl.isEmpty() )
      {
         ( *( *_impl ).pool ).kill();
         _impl.unref();
      }
   }

   ui32 OrderManagerThreadPool::getNumberOfOrdersToRun() const
   {
      return ( *( *_impl ).pool ).getNumberOfOrdersToRun();
   }

   void OrderManagerThreadPool::notify()
   {
      return ( *( *_impl ).pool ).notify();
   }
}
}

#pragma warning( pop )