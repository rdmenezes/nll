#include "segment.h"

namespace mvv
{
namespace platform
{
   void Segment::updateToolsList()
   {
      // sort the tools
      (*_sorter).sort( _tools );

      // connect the first tool to the segment
      ToolsStorage::iterator it = _tools.begin();
      if ( _tools.size() )
      {
         (**it).inputSegment = segment;
         ++it;
      }

      // connect tool output to next tool input
      for ( ; it != _tools.end(); )
      {
         ToolsStorage::iterator cur = it;
         ToolsStorage::iterator next = ++it;
         if ( next != _tools.end() )
         {
            (**next).outputSegment = (**cur).inputSegment;
         }
      }

      // duplicate or not the 
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