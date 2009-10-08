#include "stdafx.h"
#include "dynamic-resource.h"

namespace mvv
{
   EngineRunnable::~EngineRunnable()
   {
      for ( Resources::iterator it = _resources.begin(); it != _resources.end(); ++it )
         ( *it )->detach( this );
   }

   void EngineRunnable::attach( DynamicResource& r )
   {
      r.attach( this );
      _resources.insert( &r );
      notify();
   }
}
