#ifndef MVV_PLATFORM_SEGMENT_TOOL_CAMERA_H_
# define MVV_PLATFORM_SEGMENT_TOOL_CAMERA_H_

# include "segment-tool.h"
# include "segment-tool-pointer.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief This tool will center the attached segment when a bigger volume has just been loaded,
           it will also handle navigation (slice by slice, padding, zooming)
    */
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
         /*
         if ( _nbMaxVoxels > 0 )
         {
            // Todo: if several volumes, sometimes tool is not perfectly synchronized...
            return true;
         }*/

         // check if a volume has more voxel than previously
         ui32 maxVoxel = 0;
         bool updatePosition = false;
         nll::core::vector3f newPosition;
         for ( SegmentTool::LinkStorage::iterator it = SegmentTool::_links.begin(); it != SegmentTool::_links.end(); ++it )
         {
            for ( ResourceVolumes::Iterator volit = (**it).volumes.begin(); volit != (**it).volumes.end(); ++volit )
            {
               // check if the volume moved
               nll::core::vector3ui size = (**volit).size();
               const ui32 nbVoxels = (**volit).size()[ 0 ] * (**volit).size()[ 1 ] * (**volit).size()[ 2 ];
               if ( nbVoxels > _nbMaxVoxels )
               {
                  maxVoxel = nbVoxels;
                  updatePosition = true;
                  newPosition = (**volit).indexToPosition( nll::core::vector3f( static_cast<f32>( size[ 0 ] ) / 2,
                                                                                static_cast<f32>( size[ 1 ] ) / 2,
                                                                                static_cast<f32>( size[ 2 ] ) / 2 )  );
               }
            }
         }

         if ( !maxVoxel )
         {
            // empty so don't do anything
            return true;
         }

         // if true, update the segments connected on the _same_ point, even if the volume is not in the segment (else segments are not synchronized)
         if ( updatePosition )
         {
            for ( SegmentTool::LinkStorage::iterator it = SegmentTool::_links.begin(); it != SegmentTool::_links.end(); ++it )
            {
               // update segment position
               (**it).position.setValue( newPosition );

               // update the position of the pointer: all connected segments must point at the same position
               typedef std::set<SegmentToolPointer*> Pointers;
               Pointers pointers = (**it).getTools<SegmentToolPointer>();
               for ( Pointers::iterator it = pointers.begin(); it != pointers.end(); ++it )
               {
                  (*it)->setPosition( newPosition );
                  (*it)->refreshConnectedSegments();
               }
            }
         }

         _nbMaxVoxels = maxVoxel;
         return true;
      }

      virtual void receive( Segment& s, const EventMouse& e, const nll::core::vector2ui& )
      {
         if ( e.isMouseLeftButtonJustPressed )
         {
            _paddingLastPos = e.mousePosition;
         }
         if ( e.isMouseRightButtonJustPressed )
         {
            _sliceLastPos = e.mousePosition;
         }
         if ( e.isMouseLeftButtonPressed && !e.isMouseRightButtonPressed )
         {
            // we are panning the segment
            nll::core::vector2i diffMouse( - (int)e.mousePosition[ 0 ] + (int)_paddingLastPos[ 0 ],
                                           - (int)e.mousePosition[ 1 ] + (int)_paddingLastPos[ 1 ] );

            nll::core::vector3f position = s.panning.getValue();
            nll::core::vector3f directionx = s.directionx.getValue();
            nll::core::vector3f directiony = s.directiony.getValue();
            nll::core::vector2f zoom = s.zoom.getValue();

            nll::core::vector3f pos( position[ 0 ] + ( diffMouse[ 0 ] * directionx[ 0 ] / zoom[ 0 ] + diffMouse[ 1 ] * directiony[ 0 ] / zoom[ 1 ] ),
                                     position[ 1 ] + ( diffMouse[ 0 ] * directionx[ 1 ] / zoom[ 0 ] + diffMouse[ 1 ] * directiony[ 1 ] / zoom[ 1 ] ),
                                     position[ 2 ] + ( diffMouse[ 0 ] * directionx[ 2 ] / zoom[ 0 ] + diffMouse[ 1 ] * directiony[ 2 ] / zoom[ 1 ] ) );
            s.panning.setValue( pos );
            _paddingLastPos = e.mousePosition;
         } else if ( !e.isMouseLeftButtonPressed && e.isMouseRightButtonPressed )
         {
            // we are going through the slices
            nll::core::vector2i diffMouse( - (int)e.mousePosition[ 0 ] + (int)_sliceLastPos[ 0 ],
                                           - (int)e.mousePosition[ 1 ] + (int)_sliceLastPos[ 1 ] );
            float sign = ( _sliceLastPos[ 1 ] > (int)e.mousePosition[ 1 ] ) ? 1.0f : -1.0f;
            float d = static_cast<float>( fabs( (float)diffMouse[ 1 ] ) ) * sign / 10;

            nll::core::StaticVector<float, 3> cross = nll::core::cross( s.directionx.getValue(), s.directiony.getValue() );
            assert( nll::core::equal( cross.norm2(), 1.0, 1e-5 ) );  // the base vector1, vector2 must be normalized
            nll::core::vector3f pos( s.position.getValue()[ 0 ] + d * cross[ 0 ],
                                     s.position.getValue()[ 1 ] + d * cross[ 1 ],
                                     s.position.getValue()[ 2 ] + d * cross[ 2 ] );
            s.position.setValue( pos );

            _sliceLastPos = e.mousePosition;

            // if we have 2 segment with a colinear normal, then me must synchronize their position
            for ( SegmentTool::LinkStorage::iterator it = SegmentTool::_links.begin(); it != SegmentTool::_links.end(); ++it )
            {
               if ( *it != &s && !(*it)->segment.isEmpty() && nll::core::isCollinear( (*it)->segment.getValue().getNormal(), cross ) )
               {
                  (*it)->position.setValue( pos );
               }
            }

            // TODO: we need to synchronize the other slices that would not be orthogonals
         } else if ( e.isMouseLeftButtonPressed && e.isMouseRightButtonPressed )
         {
            // update the zoom factors of all the attached segments
            nll::core::vector2i diffMouse( - (int)e.mousePosition[ 0 ] + (int)_paddingLastPos[ 0 ],
                                           - (int)e.mousePosition[ 1 ] + (int)_paddingLastPos[ 1 ] );
            float d = static_cast<float>( fabs( (float)diffMouse[ 1 ] ) );
            nll::core::vector2f zoom;
            for ( SegmentTool::LinkStorage::iterator it = SegmentTool::_links.begin(); it != SegmentTool::_links.end(); ++it )
            {
               if ( (int)e.mousePosition[ 1 ] >= _paddingLastPos[ 1 ] )
               {
                  zoom[ 0 ] = (**it).zoom.getValue()[ 0 ] * (float)( 1 + d / 100 );
                  zoom[ 1 ] = (**it).zoom.getValue()[ 1 ] * (float)( 1 + d / 100 );
               } else {
                  zoom[ 0 ] = (**it).zoom.getValue()[ 0 ] / (float)( 1 + d / 100 );
                  zoom[ 1 ] = (**it).zoom.getValue()[ 1 ] / (float)( 1 + d / 100 );
               }
               (**it).zoom.setValue( zoom );
            }
            
            _paddingLastPos = e.mousePosition;
            _sliceLastPos = e.mousePosition;
            return;
         }
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
      ResourceStorageVolumes  _storage;
      ui32                    _nbMaxVoxels;
      nll::core::vector2ui    _paddingLastPos;
      nll::core::vector2ui    _sliceLastPos;
   };
}
}

#endif
