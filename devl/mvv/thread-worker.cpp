#include "stdafx.h"
#include "thread-worker.h"

#pragma warning( push )
#pragma warning( disable:4512 ) // assignment operator not generated

namespace mvv
{
   Worker::Worker( ThreadPool* pool, ui32 workerId ) : _pool( pool ), _workerId( workerId ), _hasFinished( true ), _currentOrder( 0 )
   {
      ensure( pool, "error: null pointer" );
   }

   void Worker::_run()
   {
      // locked by operator()
      OrderResult* result = _currentOrder->run();
      ensure( result, "result requires not to be null" );
      _currentOrder->setResult( result );
      _hasFinished = true;
      _pool->workerFinished( _currentOrder, _workerId );
   }
}

#pragma warning( pop )