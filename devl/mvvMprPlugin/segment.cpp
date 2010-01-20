#include "segment.h"

namespace mvv
{
namespace platform
{
   void Segment::updateToolsList()
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
            _wrappers.push_back( RefcountedTyped<SegmentToolWrapper>( new SegmentToolWrapper( _segment, *this, *it, _handler ) ) );
         } else {
            // just take a reference on the last tools as input
            RefcountedTyped<SegmentToolWrapper> wrapper( new SegmentToolWrapper( (**_wrappers.rbegin()).outputSegment, *this, *it, _handler ) );
            _wrappers.push_back( wrapper );            
         }
      }

      if ( _wrappers.size() )
      {
         segment = (**_wrappers.rbegin()).outputSegment;
         std::cout << " connected out=" << (void*)segment.getValue().getStorage().begin() << std::endl;
      }
   }

   void Segment::connect( SegmentTool* tool )
   {
      _tools.push_back( tool );
      (*tool).connect( this );

      updateToolsList();
   }

   void Segment::disconnect( SegmentTool* tool )
   {
      ToolsStorage::iterator it = std::find( _tools.begin(), _tools.end(), tool );
      if ( it != _tools.end() )
      {
         _tools.erase( it );
         (*tool).disconnect( this );
      }

      updateToolsList();
   }

   void Segment::_remove( SegmentTool* tool )
   {
      ToolsStorage::iterator it = std::find( _tools.begin(), _tools.end(), tool );
      if ( it != _tools.end() )
      {
         _tools.erase( it );
      }
   }

   Segment::~Segment()
   {
      removeConnections();
   }

   void Segment::receive( const EventMouse& e, const nll::core::vector2ui& windowOrigin )
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

   const nll::core::vector3f Segment::UNINITIALIZED_POSITION = nll::core::vector3f( -10000, -10000, -10000 );
}
}