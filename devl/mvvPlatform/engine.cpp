#include "engine.h"
#include "resource.h"

namespace mvv
{
namespace platform
{
   
   void Engine::connect( impl::ResourceSharedData* r )
   {
      _addSimpleLink( r );
      r->_addSimpleLink( this );
   }

   void Engine::disconnect( impl::ResourceSharedData* r )
   {
      _eraseSimpleLink( r );
      r->_eraseSimpleLink( this );
   }

   bool Engine::isConnected( impl::Resource r ) const
   {
      LinkStorage::const_iterator it = _links.find( &r.getData() );
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