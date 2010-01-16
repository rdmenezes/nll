#ifndef MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_
# define MVV_PLATFORM_SEGMENT_TOOL_POINTER_H_

# include "segment-tool-pointer.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API SegmentToolPointer : public SegmentTool
   {
   public:
      SegmentToolPointer() : SegmentTool( true )
      {
      }

      virtual void updateSegment( ResourceSliceuc segment, Segment&  )
      {
         ResourceSliceuc::value_type slice = segment.getValue();
         if ( !slice.size()[ 0 ] || !slice.size()[ 1 ] || !slice.size()[ 2 ] )
            return;

         nll::core::vector3f p = slice.getOrthogonalProjection( _position );
         nll::core::vector2f pplane = slice.worldToSliceCoordinate( p );
         std::cout << "val=" << pplane[ 0 ] + slice.size()[ 0 ] / 2 << " " << pplane[ 1 ] + slice.size()[ 1 ] / 2 << std::endl;
         if ( !slice.contains( pplane ) )
         {
            // out of the slice, don't display anything
            return;
         }

         ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( static_cast<ui32>( pplane[ 0 ] + slice.size()[ 0 ] / 2 ),
                                                                                            0 );
         for ( ui32 n = 0; n < slice.size()[ 1 ]; ++n, it.addy() )
         {
            *it = 255;
         }

         it = slice.getIterator( 0, static_cast<ui32>( pplane[ 1 ] + slice.size()[ 1 ] / 2 ) );
         for ( ui32 n = 0; n < slice.size()[ 0 ]; ++n, ++it )
         {
            *it = 255;
         }
      }

      virtual void receive( Segment& , const EventMouse&  )
      {

      }

   protected:
      nll::core::vector3f  _position;
   };
}
}

#endif