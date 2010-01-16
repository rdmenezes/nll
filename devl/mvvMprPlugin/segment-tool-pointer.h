#ifndef MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_
# define MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_

# include "segment-tool-pointer.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API SegmentToolPointer : public SegmentTool
   {
      typedef std::map<Segment*, bool> MapSegments;

   public:
      SegmentToolPointer() : SegmentTool( true )
      {
      }

      virtual void updateSegment( ResourceSliceuc segment, Segment& s )
      {
         ResourceSliceuc::value_type slice = segment.getValue();
         if ( !slice.size()[ 0 ] || !slice.size()[ 1 ] || !slice.size()[ 2 ] )
            return;

         bool isHighlighted = false;
         MapSegments::iterator ii = _active.find( &s );
         if ( ii != _active.end() && ii->second )
            isHighlighted = true;

         nll::core::vector3f p = slice.getOrthogonalProjection( _position );
         nll::core::vector2f pplane = slice.worldToSliceCoordinate( p );
         try
         {
            if ( !slice.contains( pplane ) )
            {
               // out of the slice, don't display anything
               return;
            }
         }
         catch( ... )
         {
            return;
         }

         ui8 val = isHighlighted ? 255 : 180;
         ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( static_cast<ui32>( pplane[ 0 ] + slice.size()[ 0 ] / 2 ),                                                                       0 );
         for ( ui32 n = 0; n < slice.size()[ 1 ]; ++n, it.addy() )
         {
            *it = val;
         }

         it = slice.getIterator( 0, static_cast<ui32>( pplane[ 1 ] + slice.size()[ 1 ] / 2 ) );
         for ( ui32 n = 0; n < slice.size()[ 0 ]; ++n, ++it )
         {
            *it = val;
         }
      }

      virtual void receive( Segment& segment, const EventMouse& e, const nll::core::vector2ui& windowOrigin )
      {
         // project the point on the plane
         ResourceSliceuc::value_type slice = segment.segment.getValue();
         if ( !slice.size()[ 0 ] || !slice.size()[ 1 ] || !slice.size()[ 2 ] )
            return;

         nll::core::vector3f p = slice.getOrthogonalProjection( _position );
         nll::core::vector2f pplane = slice.worldToSliceCoordinate( p );
         pplane[ 0 ] += static_cast<f32>( slice.size()[ 0 ] ) / 2;
         pplane[ 1 ] += static_cast<f32>( slice.size()[ 1 ] ) / 2;

         MapSegments::iterator it = _active.find( &segment );
         bool justMakeItActive = false;
         if ( it == _active.end() || !it->second )
         {
            // the crosshair may possibly needs to be highlighted
            if ( abs( pplane[ 0 ] - ( e.mousePosition[ 0 ] - windowOrigin[ 0 ] ) ) < 10 ||
                 abs( pplane[ 1 ] - ( e.mousePosition[ 1 ] - windowOrigin[ 1 ] ) ) < 10 )
            {
               justMakeItActive = true;
               _active[ &segment ] = true;
               segment.refreshTools();
            }
         }

         if ( !justMakeItActive )
         {
            // we know none should be highlighted
            for ( MapSegments::iterator it = _active.begin(); it != _active.end(); ++it )
            {
               bool isOutside = abs( pplane[ 0 ] - ( e.mousePosition[ 0 ] - windowOrigin[ 0 ] ) ) >= 10 &&
                                abs( pplane[ 1 ] - ( e.mousePosition[ 1 ] - windowOrigin[ 1 ] ) ) >= 10;
               if ( ( isOutside || it->first != &segment ) && it->second )
               {
                  SegmentTool::LinkStorage::iterator ii = _links.find( it->first );
                  it->second = false;
                  if ( ii != _links.end() )
                  {
                     (*ii)->refreshTools();
                  }
                  break;
               }
            }
         }
         
      }

      virtual void _eraseSimpleLink( Segment* o )
      {
         MapSegments::iterator ii = _active.find( o );
         if ( ii != _active.end() )
            _active.erase( ii );
         LinkStorage::iterator it = _links.find( o );
         if ( it != _links.end() )
            _links.erase( it );
      }

   protected:
      nll::core::vector3f  _position;
      MapSegments          _active;
   };
}
}

#endif