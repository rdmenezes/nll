#ifndef MVV_PLATFORM_SEGMENT_TOOL_MANIPULATORS_H_
# define MVV_PLATFORM_SEGMENT_TOOL_MANIPULATORS_H_

# include "segment-tool.h"
# include "types.h"

namespace mvv
{
namespace platform
{
   /**
    @brief A manipulator interface

    The process is simple:
     - checkEvent will be called everytime there is a mouse event on this MPR, this function
       must use notify() to trigger a visual modification of the manipulator (return true)
     - draw will be called each time the manipulator needs to be displayed
    */
   class MVVMPRPLUGIN_API ToolManipulatorsInterface
   {
   public:
      virtual ~ToolManipulatorsInterface(){}

      /**
       @brief Returns true if the tool is interacting (i.e. it needs to be redrawn). In this case, at the next engine check,
              a redaw will be issued, updating all connected segments.
       @note that only one manipulator can be interacting at the same time
       @param it is FORBIDEN to draw on the segment... this must be done only in the draw() method
       @param positionStartStroke the position (x, y) of the mouse in the segment taken at the begining of the mouse mouvement
       @param positionEndStroke the position (x, y) of the mouse in the segment taken at the end of the mouse mouvement
       */
      virtual bool checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e ) = 0;

      /**
       @brief actually draw on the slice
       @param isActivated if true, the manipulator must be displayed with 'activated' style
       */
      virtual void draw( ResourceSliceuc& slice, bool isActivated ) = 0;

      /**
       @brief This method is called on after before segments are drawn
       */
      virtual void dispatch( std::set<Segment*>& segments )
      {
         // do nothing
      }
   };

   class MVVMPRPLUGIN_API ToolManipulatorsPoint : public ToolManipulatorsInterface
   {
   public:
      ToolManipulatorsPoint( const nll::core::vector3f& position, nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 255 ) ) : _position( position ), _color( color )
      {
         _colorInactif = nll::core::vector3uc( 0.7 * color[ 0 ],
                                               0.7 * color[ 1 ],
                                               0.7 * color[ 2 ] );
      }

      virtual bool checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e );

      virtual void draw( ResourceSliceuc& s, bool isActivated );

   private:
      nll::core::vector3f  _position;
      nll::core::vector3uc _color;
      nll::core::vector3uc _colorInactif;
   };

   class MVVMPRPLUGIN_API ToolManipulatorsPointer : public ToolManipulatorsInterface
   {
   public:
      ToolManipulatorsPointer( nll::core::vector3uc color = nll::core::vector3uc( 255, 0, 0 ) ) : _position( nll::core::vector3f( 0, 0, 0 ) ), _color( color ), _segmentIssuingDispatch( 0 ), _needToSynchronizePosition( false )
      {
         _colorInactif = nll::core::vector3uc( 0.7 * color[ 0 ],
                                               0.7 * color[ 1 ],
                                               0.7 * color[ 2 ] );
      }

      virtual bool checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e );

      virtual void draw( ResourceSliceuc& s, bool isActivated );

      virtual void dispatch( std::set<Segment*>& segments );

   private:
      nll::core::vector3f  _position;
      nll::core::vector3uc _color;
      nll::core::vector3uc _colorInactif;
      Segment*             _segmentIssuingDispatch;
      bool                 _needToSynchronizePosition;
   };

   /**
    @ingroup platform
    @brief This will handle a postprocessing effect that needs to be done after all other tools
    */
   class MVVMPRPLUGIN_API SegmentToolManipulators : public SegmentTool
   {
      typedef std::vector< RefcountedTyped<ToolManipulatorsInterface> > Manipulators;
   public:
      SegmentToolManipulators() : SegmentTool( true ), _wasActivated( false ), _currentSegmentInteraction( 0 )
      {
      }

      void add( RefcountedTyped<ToolManipulatorsInterface> manip )
      {
         _manipulators.push_back( manip );
      }

      virtual void receive( Segment& s, const EventMouse& e, const nll::core::vector2ui& origin );

      virtual f32 priority() const
      {
         return 101;
      }

      virtual bool isSavingMprImage() const
      {
         return true;
      }

      virtual void updateSegment( ResourceSliceuc slice, Segment& s );

      virtual bool interceptEvent() const
      {
         return _wasActivated;
      }

   protected:
      Manipulators                                 _manipulators;
      nll::core::vector2i                          _leftMouseLastPos;
      RefcountedTyped<ToolManipulatorsInterface>   _lastActivated;      // store the manipulator that must be drawn the latest
      bool                                         _wasActivated;
      Segment*                                     _currentSegmentInteraction;
   };
}
}

#endif
