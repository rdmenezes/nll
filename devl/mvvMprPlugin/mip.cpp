#include "mip.h"

namespace mvv
{
namespace platform
{
   void Mip::updateToolsList()
   {
      // sort the tools
      (*_sorter).sort( _tools );

      // TODO: if overhead is too big, we can simply update instead of recreating everything...
      _wrappers.clear();
      for ( ToolsStorage::iterator it = _tools.begin(); it != _tools.end(); ++it )
      {
         if ( _wrappers.size() == 0 )
         {
            // init with the initial segment
            _wrappers.push_back( RefcountedTyped<MipToolWrapper>( new MipToolWrapper( _outImage, *this, *it, _handler ) ) );
         } else {
            // just take a reference on the last tools as input
            RefcountedTyped<MipToolWrapper> wrapper( new MipToolWrapper( (**_wrappers.rbegin()).outputMip, *this, *it, _handler ) );
            _wrappers.push_back( wrapper );            
         }
      }

      if ( _wrappers.size() )
      {
         outImage = (**_wrappers.rbegin()).outputMip;
      }
   }

   void Mip::connect( MipTool* tool )
   {
      _tools.push_back( tool );
      (*tool).connect( this );

      updateToolsList();
   }

   void Mip::disconnect( MipTool* tool )
   {
      ToolsStorage::iterator it = std::find( _tools.begin(), _tools.end(), tool );
      if ( it != _tools.end() )
      {
         _tools.erase( it );
         (*tool).disconnect( this );
      }

      updateToolsList();
   }

   void Mip::receive( const EventMouse& e, const nll::core::vector2ui& windowOrigin )
   {
      for ( ToolsStorage::reverse_iterator it = _tools.rbegin(); it != _tools.rend(); ++it )
      {
         (*it)->receive( *this, e, windowOrigin );
         if ( (*it)->interceptEvent() )
         {
            // we stop the events
            break;
         }
      }
   }

}
}