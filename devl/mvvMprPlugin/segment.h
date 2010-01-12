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

      class SegmentToolWrapper : public Engine
      {
      private:
         // output of one tool is connected to the input of the following output
         ResourceSliceuc   inputSegment;     /// must never be modified directly

      public:
         ResourceSliceuc   outputSegment;

      public:
         SegmentToolWrapper( ResourceSliceuc vinputSegment,
                             Segment& segment,
                             SegmentTool* tool,
                             EngineHandler& handler ) : Engine( handler ), inputSegment( vinputSegment ), _tool( tool ), _segment( segment )
         {
            ensure( tool, "must not be zero" );

            connect( inputSegment );
            handler.connect( *this );

            outputSegment = inputSegment;
         }

      private:
         // update the segment as soon as it is changed
         virtual bool _run()
         {
            //std::cout << "size=" << inputSegment.getValue().size()[ 0 ] << " " << inputSegment.getValue().size()[ 1 ] << std::endl;

            if ( !inputSegment.getValue().size()[ 0 ] ||
                 !inputSegment.getValue().size()[ 1 ] ||
                 !inputSegment.getValue().size()[ 2 ] )
            {
               // we have no segment to display, then just do nothing
               return true;
            }


            // update the geometry in case it is different
            outputSegment.getValue().setGeometry( inputSegment.getValue().getAxisX(),
                                                  inputSegment.getValue().getAxisY(),
                                                  inputSegment.getValue().getOrigin(),
                                                  inputSegment.getValue().getSpacing() );
            if ( _tool->isModifyingMprImage() )
            {
               // if we modify the image & the dimenstion are not good, just deep copy the input
               if ( inputSegment.getValue().getStorage().size() != outputSegment.getValue().getStorage().size() )
               {
                  outputSegment.getValue().getStorage().clone( inputSegment.getValue().getStorage() );
               } else {
                  // quick copy, we know memory is continuous
                  Sliceuc::iterator inputBegin = inputSegment.getValue().getStorage().begin();
                  Sliceuc::iterator outputBegin = outputSegment.getValue().getStorage().begin();
                  memcpy( &*outputBegin, &*inputBegin, inputSegment.getValue().getStorage().size() );
               }

               // notify the changes
               outputSegment.notify();
            }

            //do the changes on the output
            _tool->updateSegment( outputSegment, _segment );
            return true;
         }

      private:
         SegmentTool*      _tool;
         Segment&          _segment;
      };

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

   private:
      ResourceSliceuc               _segment;

   public:
      Segment( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : volumes( storage ), _slicer( volumes, position, directionx, directiony, panning, zoom, size, luts, intensities, isInteracting, interpolation, handler, provider, dispatcher, false ), _handler( handler )
      {
         position.setValue( nll::core::vector3f( 0, 0, 0 ) );
         directionx.setValue( nll::core::vector3f( 1, 0, 0 ) );
         directiony.setValue( nll::core::vector3f( 0, 1, 0 ) );
         panning.setValue( nll::core::vector3f( 0, 0, 0 ) );
         zoom.setValue( nll::core::vector2f( 2, 2 ) );
         isInteracting.setValue( false );
         interpolation.setValue( LINEAR );

         // point to the correct resource..
         _segment = _slicer.blendedSlice;
         segment = _segment;

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
      typedef RefcountedTyped<SegmentToolWrapper>  Wrapper;
      typedef std::vector<Wrapper>                 Wrappers;

      EngineMpr                           _slicer;
      ToolsStorage                        _tools;
      RefcountedTyped<SegmentToolSorter>  _sorter;
      Wrappers                            _wrappers;
      EngineHandler&                      _handler;
   };
}
}

#endif