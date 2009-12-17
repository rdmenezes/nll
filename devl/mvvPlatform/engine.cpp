#include "engine.h"
#include "resource.h"

namespace mvv
{
namespace platform
{
   void Engine::connect( impl::Resource r )
   {
      _resources.insert( r );
   }

   void Engine::disconnect( impl::Resource r )
   {
      size_t nb = _resources.erase( r );
      if ( nb )
      {
         r.disconnect( this );
      }
   }

   Engine::~Engine()
   {
      for ( ResourceStorage::iterator it = _resources.begin(); it != _resources.end(); ++it )
      {
         disconnect( *it );
      }
      _handler.disconnect( this );
   }
}
}