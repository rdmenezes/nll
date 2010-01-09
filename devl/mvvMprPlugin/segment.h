#ifndef MVV_PLATFORM_SEGMENT_H_
# define MVV_PLATFORM_SEGMENT_H_

# include "mvvMprPlugin.h"
# include "engine-mpr.h"
# include "segment-tool.h"
# include "segment-tool-sorter.h"
# include <mvvPlatform/event-mouse-receiver.h>
# include <mvvPlatform/linkable.h>

namespace mvv
{
namespace platform
{
   // forward declaration
   class SegmentTool;

   /**
    @ingroup platform
    @brief Display a MPR and all supporting information
    */
   class MVVMPRPLUGIN_API Segment : public EventMouseReceiver, LinkableDouble<SegmentTool*, Segment*>
   {
      friend class SegmentTool;

      typedef std::vector< SegmentTool* > ToolsStorage;

   public:
      // input slots
      ResourceVolumes               volumes;
      ResourceVector3f              position;
      ResourceVector3f              directionx;
      ResourceVector3f              directiony;
      ResourceVector3f              panning;
      ResourceVector2f              zoom;
      ResourceVector2ui             size;
      ResourceMapTransferFunction   luts;
      ResourceFloats                intensities;
      ResourceBool                  isInteracting;
      ResourceInterpolationMode     interpolation;

   public:
      // output slots
      ResourceSliceuc               segment;

   public:
      Segment( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : volumes( storage ), _slicer( volumes, position, directionx, directiony, panning, zoom, size, luts, intensities, isInteracting, interpolation, handler, provider, dispatcher, false )
      {
         position.setValue( nll::core::vector3f( 0, 0, 0 ) );
         directionx.setValue( nll::core::vector3f( 1, 0, 0 ) );
         directiony.setValue( nll::core::vector3f( 0, 1, 0 ) );
         panning.setValue( nll::core::vector3f( 0, 0, 0 ) );
         zoom.setValue( nll::core::vector2f( 2, 2 ) );
         isInteracting.setValue( false );
         interpolation.setValue( LINEAR );

         // point to the correct resource..
         segment = _slicer.blendedSlice;

         // use a default priority sorter
         _sorter = RefcountedTyped<SegmentToolSorter>( new SegmentToolSorterPriorityQueue() );
      }

      ~Segment();

      virtual void connect( SegmentTool* tool );     

      virtual void disconnect( SegmentTool* tool );

      virtual void receive( const EventMouse& event );

      void setToolSorter( RefcountedTyped<SegmentToolSorter> sorter )
      {
         _sorter = sorter;
      }

   protected:
      void updateToolsList();

   private:
      void _remove( SegmentTool* tool );

   protected:
      EngineMpr                           _slicer;
      ToolsStorage                        _tools;
      RefcountedTyped<SegmentToolSorter>  _sorter;
   };
}
}

#endif