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

      virtual bool _run()
      {
         bool biggerVolumeFound = false;
         nll::core::vector3f pos;
         for ( ResourceStorageVolumes::Iterator it = _storage.begin(); it != _storage.end(); ++it )
         {
            nll::core::vector3ui size = (**it).size();
            const ui32 s = (**it).size()[ 0 ] * (**it).size()[ 1 ] * (**it).size()[ 2 ];
            if ( s > _nbMaxVoxels )
            {
               _nbMaxVoxels = s;
               biggerVolumeFound = true;
               pos = (**it).indexToPosition( nll::core::vector3f( static_cast<f32>( size[ 0 ] ) / 2,
                                                                  static_cast<f32>( size[ 1 ] ) / 2,
                                                                  static_cast<f32>( size[ 2 ] ) / 2 )  );
            }
         }

         // if a bigger volume is found, center all the segments on it
         _position.setValue( pos );
         for ( LinkStorage::iterator it = _links.begin(); it != _links.end(); ++it )
         {
            _position.copyAndAddConnections( (**it).position );
         }

         std::cout << "camera=" << this << " pos=" << pos[ 0 ] << " " << pos[ 1 ] << " " << pos[ 2 ] << std::endl;
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
      ResourceVector3f       _position;
      ResourceStorageVolumes _storage;
      ui32 _nbMaxVoxels;
   };
}
}

#endif
