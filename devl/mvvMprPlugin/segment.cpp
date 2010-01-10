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
         _wrappers.push_back( RefcountedTyped<SegmentToolWrapper>( new SegmentToolWrapper( *this, *it, _handler ) ) );
      }

      
      // connect the first tool to the segment
      Wrappers::iterator it = _wrappers.begin();
      if ( _wrappers.size() )
      {
         (**it).inputSegment = segment;
         ++it;
      }

      // connect tool output to next tool input
      for ( ; it != _wrappers.end(); )
      {
         Wrappers::iterator cur = it;
         Wrappers::iterator next = ++it;
         if ( next != _wrappers.end() )
         {
            (**next).outputSegment = (**cur).inputSegment;
         }
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

   void Segment::receive( const EventMouse& e )
   {
      for ( ToolsStorage::iterator it = _tools.begin(); it != _tools.end(); ++it )
      {
         (*it)->receive( *this, e );
      }
   }
}
}