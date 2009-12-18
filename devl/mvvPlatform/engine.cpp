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

   Engine::~Engine()
   {
      ResourceStorage::iterator cur;
      for ( ResourceStorage::iterator it = _resources.begin(); it != _resources.end(); )
      {
         cur = it++;
         disconnect( *cur );
      }
      _handler.disconnect( *this );
   }
}
}