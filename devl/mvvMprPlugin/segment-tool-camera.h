#ifndef MVV_PLATFORM_SEGMENT_TOOL_CAMERA_H_
# define MVV_PLATFORM_SEGMENT_TOOL_CAMERA_H_

# include "segment-tool.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API SegmentToolCamera : public SegmentTool, public Engine
   {
   public:
      /**
       @param storage has a new volume added, and this added volume is bigger than all others, the camera is centering this volume
       */
      SegmentToolCamera( ResourceStorageVolumes storage, EngineHandler& handler ) : Engine( handler ), _storage( storage ), SegmentTool( false )
      {
         handler.connect( *this );
         storage.connect( this );
         _nbMaxVoxels = 0;
      }

      /**
       @brief on a volume loaded
       */
      virtual bool _run()
      {
         for ( SegmentTool::LinkStorage::iterator it = SegmentTool::_links.begin(); it != SegmentTool::_links.end(); ++it )
         {
            if ( (**it).getAuthorizeRecenteringOnLoading() )
            {
               bool biggerVolumeFound = false;
               nll::core::vector3f pos;
               for ( ResourceVolumes::Iterator volit = (**it).volumes.begin(); volit != (**it).volumes.end(); ++volit )
               {
                  nll::core::vector3ui size = (**volit).size();
                  const ui32 s = (**volit).size()[ 0 ] * (**volit).size()[ 1 ] * (**volit).size()[ 2 ];
                  if ( s > _nbMaxVoxels )
                  {
                     _nbMaxVoxels = s;
                     biggerVolumeFound = true;
                     pos = (**volit).indexToPosition( nll::core::vector3f( static_cast<f32>( size[ 0 ] ) / 2,
                                                                           static_cast<f32>( size[ 1 ] ) / 2,
                                                                           static_cast<f32>( size[ 2 ] ) / 2 )  );
                  }
               }
               if ( biggerVolumeFound )
               {
                  (**it).position.setValue( pos );
               }
            }
         }
         return true;
      }

      virtual void receive( Segment& , const EventMouse&  )
      {
      }

      virtual void updateSegment( ResourceSliceuc, Segment& )
      {
         // do nothing
      }

      /**
       @brief update the segment's location
       */
      void setPosition( const nll::core::vector3f& pos )
      {
         for ( SegmentTool::LinkStorage::iterator it = SegmentTool::_links.begin(); it != SegmentTool::_links.end(); ++it )
         {
            (**it).position.setValue( pos );
         }
      }

   protected:
      ResourceStorageVolumes _storage;
      ui32 _nbMaxVoxels;
   };
}
}

#endif
