#ifndef MVV_PLATFORM_SEGMENT_TOOL_MANIPULATORS_H_
# define MVV_PLATFORM_SEGMENT_TOOL_MANIPULATORS_H_

# include "segment-tool.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API ToolManipulatorsInterface
   {
   public:
      virtual void draw( Sliceuc& s ) = 0;

      virtual ~ToolManipulatorsInterface(){}

      /**
       @brief Returns true if the tool is interacting. In this case, all the manipulators not interacting will
              be redisplayed, and the last one will be the interacting tool
       @param it is FORBIDEN to draw on the segment... this must be done only in the draw() method
       @param positionStartStroke the position (x, y) of the mouse in the segment taken at the begining of the mouse mouvement
       @param positionEndStroke the position (x, y) of the mouse in the segment taken at the end of the mouse mouvement
       */
      virtual bool isInteracting( Segment&, const nll::core::vector2ui& positionStartStroke, const nll::core::vector2ui& positionEndStroke  ) = 0;

      /**
       @brief
       */
      virtual void draw( ResourceSliceuc slice, Segment& s ) = 0;
   };

   /**
    @ingroup platform
    @brief This will handle a postprocessing effect that needs to be done after all other tools
    */
   class MVVMPRPLUGIN_API SegmentToolManipulators : public SegmentTool
   {
      typedef std::vector< RefcountedTyped<ToolManipulatorsInterface> > Manipulators;
   public:
      SegmentToolManipulators() : SegmentTool( true )
      {
      }

      void add( RefcountedTyped<ToolManipulatorsInterface> manip )
      {
         _manipulators.push_back( manip );
      }

      virtual void receive( Segment& s, const EventMouse& e, const nll::core::vector2ui& origin )
      {
         if ( e.isMouseLeftButtonJustPressed )
         {
            _leftMouseLastPos[ 0 ] = e.mousePosition[ 0 ] - origin[ 0 ];
            _leftMouseLastPos[ 1 ] = e.mousePosition[ 1 ] - origin[ 1 ];
         }

         // while the left button is pressed,
         if ( e.isMouseLeftButtonPressed )
         {
            nll::core::vector2ui strokeEnd( e.mousePosition[ 0 ] - origin[ 0 ],
                                            e.mousePosition[ 1 ] - origin[ 1 ] );
            for ( ui32 n = 0; n < _manipulators.size(); ++n )
            {
               if ( (*_manipulators[ n ]).isInteracting( s, _leftMouseLastPos, strokeEnd ) )
               {
                  // TODO force redraw
               }
            }
         }
      }

      virtual f32 priority() const
      {
         return 101;
      }

      virtual bool isSavingMprImage() const
      {
         return true;
      }

      virtual void updateSegment( ResourceSliceuc slice, Segment& s )
      {
      }

      virtual bool interceptEvent() const
      {
         return true;
      }

   protected:
      Manipulators            _manipulators;
      nll::core::vector2ui    _leftMouseLastPos;
   };
}
}

#endif
