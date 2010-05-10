#ifndef MVV_PLATFORM_SEGMENT_TOOL_AUTOCENTER_H_
# define MVV_PLATFORM_SEGMENT_TOOL_AUTOCENTER_H_

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief This tool will center the attached segment when a bigger volume has just been loaded
    @note when this tool is added to a segment, we are watching it 
    */
   class MVVMPRPLUGIN_API SegmentToolAutocenter : public SegmentTool, public Engine
   {
   public:
      SegmentToolAutocenter( EngineHandler& handler ) : SegmentTool( false ), Engine( handler )
      {
         _nbMaxVoxels = 0;
      }

      virtual void receive( Segment&, const EventMouse&, const nll::core::vector2ui& )
      {
         // do nothing
      }

      virtual void updateSegment( ResourceSliceuc, Segment& )
      {
         // do nothing
      }

      // we need to override, as we need to register the volume storage
      virtual void connect( Segment* o )
      {
         SegmentTool::_addSimpleLink( o );
         o->_addSimpleLink( this );
         _storages.insert( o->volumes.getStorage() );
         o->volumes.getStorage().connect( this );
      }

      // we need to override, as we need to unregister the volume storage
      virtual void disconnect( Segment* o )
      {
         SegmentTool::_eraseSimpleLink( o );
         o->_eraseSimpleLink( this );
         // TODO: how do we remove safely from the list? (i.e. all segments points to the same storage)
      }

      /**
       @brief on a volume loaded
       */
      virtual bool _run()
      {
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

   private:
      ui32                    _nbMaxVoxels;
      std::set<ResourceStorageVolumes> _storages;
   };
}
}

#endif