#ifndef MVV_PLATFORM_SEGMENT_TOOL_MANIPULATORS_H_
# define MVV_PLATFORM_SEGMENT_TOOL_MANIPULATORS_H_

# include "segment-tool.h"
# include "types.h"
# include <mvvPlatform/font.h>

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

    if the manipulator needs to be redrawn, use notify(), but only when it is not handled (e.g. checkEvent return true, do not use notify as it will redaw anyway!)
    Typically if the user modify one of its property(e.g. position) then use notify()
    */
   class MVVMPRPLUGIN_API ToolManipulatorsInterface : public ResourceBool
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
       @brief This method is called on before segments are drawn
       */
      virtual void dispatch( std::set<Segment*>& )
      {
         // do nothing
      }
   };

   /**
    @brief manipulates a single 3D point
    */
   class MVVMPRPLUGIN_API ToolManipulatorsPoint : public ToolManipulatorsInterface
   {
   public:
      ToolManipulatorsPoint( const nll::core::vector3f& position, const std::string& txt, Font& font, ui32 txtSize = 12, nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 255 ) ) : _position( position ), _color( color ), _txt( txt ), _font( font ), _txtSize( txtSize )
      {
         _colorInactif = nll::core::vector3uc( 0.7 * color[ 0 ],
                                               0.7 * color[ 1 ],
                                               0.7 * color[ 2 ] );
      }

      virtual bool checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e );

      virtual void draw( ResourceSliceuc& s, bool isActivated );

      const nll::core::vector3f& getPosition() const
      {
         return _position;
      }

      void setPosition( const nll::core::vector3f& p )
      {
         _position = p;
         notify();
      }

      const std::string& getTxt() const 
      {
         return _txt;
      }

      void setTxt( const std::string& txt )
      {
         _txt = txt;
         notify();
      }

   private:
      // disabled
      ToolManipulatorsPoint& operator=( const ToolManipulatorsPoint& );
      ToolManipulatorsPoint( const ToolManipulatorsPoint& );

   private:
      nll::core::vector3f  _position;
      nll::core::vector3uc _color;
      nll::core::vector3uc _colorInactif;
      std::string          _txt;
      ui32                 _txtSize;
      Font&                _font;
   };

   /**
    @brief a pointer to synchronize segment & manipulate them (pan, zoom, scrolling)
    */
   class MVVMPRPLUGIN_API ToolManipulatorsPointer : public ToolManipulatorsInterface
   {
      // disabled
      ToolManipulatorsPointer& operator=( const ToolManipulatorsPointer& );
      ToolManipulatorsPointer( const ToolManipulatorsPointer& );

   public:
      ToolManipulatorsPointer( Font& font, ui32 nbPixelForSelection = 5, float panningFactor = 10.0, float zoomingFactor = 100.0, nll::core::vector3uc color = nll::core::vector3uc( 255, 0, 0 ), ui32 fontSize = 12, nll::core::vector3uc fontColor = nll::core::vector3uc( 255, 255, 255 ) ) : _position( nll::core::vector3f( 0, 0, 0 ) ), _color( color ), _segmentIssuingDispatch( 0 ), _needToSynchronizePosition( false ), _needToSynchronizeZoom( false ), _needToSynchronizeZPos( false ), _wasPanning( false ), _panningFactor( panningFactor ), _zoomingFactor( zoomingFactor ), _nbPixelForSelection( nbPixelForSelection ), _fontSize( fontSize ), _fontColor( fontColor ), _font( font )
      {
         _colorInactif = nll::core::vector3uc( 0.7 * color[ 0 ],
                                               0.7 * color[ 1 ],
                                               0.7 * color[ 2 ] );
      }

      virtual bool checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e );

      virtual void draw( ResourceSliceuc& s, bool isActivated );

      virtual void dispatch( std::set<Segment*>& segments );

      void setPosition( const nll::core::vector3f& p )
      {
         _needToSynchronizePositionFull = true;
         _position = p;
         notify();
      }

      const nll::core::vector3f& getPosition() const
      {
         return _position;
      }

   private:
      nll::core::vector3f  _position;
      nll::core::vector3uc _color;
      nll::core::vector3uc _colorInactif;
      Segment*             _segmentIssuingDispatch;
      bool                 _needToSynchronizePosition;
      bool                 _needToSynchronizePositionFull;
      bool                 _needToSynchronizeZoom;
      bool                 _needToSynchronizeZPos;
      float                _zoomUpdate;
      bool                 _wasPanning;

      float                _panningFactor;
      float                _zoomingFactor;
      float                _nbPixelForSelection;
      nll::core::vector3f  _zmovementNormal;
      nll::core::vector3f  _zmovementPointer;

      ui32                 _fontSize;
      nll::core::vector3uc _fontColor;
      Font&                _font;
   };

   /**
    @brief manipulate a cuboid
    */
   class MVVMPRPLUGIN_API ToolManipulatorsCuboid : public ToolManipulatorsInterface
   {
   public:
      ToolManipulatorsCuboid( const nll::core::vector3f& min, const nll::core::vector3f& max, nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 255 ) ) : _min( min ), _max( max ), _color( color )
      {
         _colorInactif = nll::core::vector3uc( 0.7 * color[ 0 ],
                                               0.7 * color[ 1 ],
                                               0.7 * color[ 2 ] );
      }

      virtual bool checkEvent( Segment& s, const nll::core::vector2i& positionStartStroke, const nll::core::vector2i& positionEndStroke, const EventMouse& e );

      virtual void draw( ResourceSliceuc& s, bool isActivated );

      void setPoint1( const nll::core::vector3f& p )
      {
         notify();
         _min = p;
      }

      void setPoint2( const nll::core::vector3f& p )
      {
         notify();
         _max = p;
      }

      const nll::core::vector3f& getPoint1() const
      {
         return _min;
      }

      const nll::core::vector3f& getPoint2() const
      {
         return _max;
      }

   private:
      nll::core::vector3f  _min;
      nll::core::vector3f  _max;
      nll::core::vector3uc _color;
      nll::core::vector3uc _colorInactif;
   };

   /**
    @ingroup platform
    @brief This will handle a postprocessing effect that needs to be done after all other tools

    The pattern is: the manipulator is a resource, SegmentToolManipulators is an engine listening to all
    ToolManipulatorsInterface. In the _run() we just want to notify the SegmentToolManipulators resource to trigger a redraw
    */
   class MVVMPRPLUGIN_API SegmentToolManipulators : public SegmentTool, public Engine
   {
      typedef std::vector< RefcountedTyped<ToolManipulatorsInterface> > Manipulators;
   public:
      SegmentToolManipulators( EngineHandler& handler ) : SegmentTool( true ), Engine( handler ), _wasActivated( false ), _currentSegmentInteraction( 0 )
      {
      }

      void add( RefcountedTyped<ToolManipulatorsInterface> manip )
      {
         (*manip).connect( this );  // we want to be notified each time the resource is triggered
         _manipulators.push_back( manip );
      }

      void erase( RefcountedTyped<ToolManipulatorsInterface> manip )
      {
         Manipulators::iterator it = std::find( _manipulators.begin(), _manipulators.end(), manip );
         if ( it != _manipulators.end() )
         {
            (**it).disconnect( this );
            _manipulators.erase( it );
         }
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

      virtual void updateSegment( ResourceSliceuc& slice, Segment& s );

      virtual bool interceptEvent() const
      {
         return _wasActivated;
      }

      std::set<Segment*> getConnectedSegments()
      {
         return SegmentTool::_links;
      }

      void clear()
      {
         for ( size_t n = 0; n < _manipulators.size(); ++n )
         {
            erase( _manipulators[ n ] );
         }
         _manipulators.clear();
      }

      /**
       @brief Returns the first inserted manipulator of this type
       */
      template <class Manipulator> Manipulator* get()
      {
         for ( ui32 n = 0; n < _manipulators.size(); ++n )
         {
            Manipulator* m = dynamic_cast<Manipulator*>( &( *_manipulators[ n ] ) );
            if ( m )
               return m;
         }
         return 0;
      }

   protected:
      virtual bool _run()
      {
         std::cout << "NOTIFICATION" << std::endl;
         SegmentTool::notify();
         return true;
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
