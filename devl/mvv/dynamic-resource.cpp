#include "stdafx.h"
#include "dynamic-resource.h"

namespace mvv
{
   Engine::~Engine()
   {
      for ( Resources::iterator it = _resources.begin(); it != _resources.end(); ++it )
         ( *it )->detach( this );
   }

   void Engine::attach( DynamicResource& r )
   {
      r.attach( this );
      _resources.insert( &r );
   }
}
