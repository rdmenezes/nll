#include "engine.h"
#include "resource.h"

namespace mvv
{
namespace platform
{
   
   void Engine::connect( impl::Resource r )
   {
      addSimpleLink( r );
      r.addSimpleLink( this );
   }

   void Engine::disconnect( impl::Resource r )
   {
      eraseSimpleLink( r );
      r.eraseSimpleLink( this );
   }

   bool Engine::isConnected( impl::Resource r ) const
   {
      LinkStorage::const_iterator it = _links.find( r );
      if ( it != _links.end() )
         return true;
      return false;
   }

   Engine::~Engine()
   {
      removeConnections();
      _handler.disconnect( *this );
   }
}
}