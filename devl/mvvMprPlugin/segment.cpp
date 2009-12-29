#include "segment.h"

namespace mvv
{
namespace platform
{
   void Segment::connect( SegmentTool* tool )
   {
      _tools.insert( tool );
      (*tool).connect( this );
   }

   void Segment::disconnect( SegmentTool* tool )
   {
      ToolsStorage::iterator it = _tools.find( tool );
      if ( it != _tools.end() )
      {
         _tools.erase( tool );
         (*tool).disconnect( this );
      }
   }

   void Segment::_remove( SegmentTool* tool )
   {
      ToolsStorage::iterator it = _tools.find( tool );
      if ( it != _tools.end() )
      {
         _tools.erase( it );
      }
   }

   Segment::~Segment()
   {
      std::cout << "segment destructor" << std::endl;
      for ( ToolsStorage::iterator it = _tools.begin(); it != _tools.end(); ++it )
         disconnect( *it );
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