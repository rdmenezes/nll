#include "engine.h"
#include "resource.h"

namespace mvv
{
namespace platform
{
   void Engine::connect( impl::Resource r )
   {
      _resources.insert( r );
      r.connect( this );
   }

   void Engine::disconnect( impl::Resource r )
   {
      size_t nb = _resources.erase( r );
      if ( nb )
      {
         r.disconnect( this );
      }
   }

   void Engine::clearConnections()
   {
      while ( _resources.size() )
      {
         disconnect( *_resources.begin() );
      }
   }

   Engine::~Engine()
   {
      clearConnections();
      _handler.disconnect( *this );
   }
}
}