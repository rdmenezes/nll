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
      {/*
         handler.connect( *this );
         _nbMaxVoxels = 0;*/

      }

      virtual bool _run()
      {
         /*
         for ( ResourceStorageVolumes::Iterator it = _storage.begin(); it != _storage.end(); ++it )
         {
            const ui32 s = (**it).size()[ 0 ] * (**it).size()[ 1 ] * (**it).size()[ 2 ];
            if ( s > _nbMaxVoxels )
            {
               std::cout << "Haha bigger=" << s << std::endl;
               _nbMaxVoxels = s;
            }
         }*/
         return true;
      }

      virtual void receive( Segment& , const EventMouse&  )
      {
      }

      virtual void updateSegment( ResourceSliceuc, Segment& )
      {
         // do nothing
      }

   protected:
      ResourceStorageVolumes _storage;
      ui32 _nbMaxVoxels;
   };
}
}

#endif
