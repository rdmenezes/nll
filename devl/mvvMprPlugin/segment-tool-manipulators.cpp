# include "segment-tool-manipulators.h"
# include "segment.h"

namespace mvv
{
namespace platform
{
   void SegmentToolManipulators::receive( Segment& s, const EventMouse& e, const nll::core::vector2ui& origin )
   {
      nll::core::vector2ui strokeEnd( e.mousePosition[ 0 ] - origin[ 0 ],
                                      e.mousePosition[ 1 ] - origin[ 1 ] );
      nll::core::vector2ui strokeStart = _leftMouseLastPos;

      _leftMouseLastPos = strokeEnd;
      bool wasActivated = _wasActivated;

      for ( ui32 n = 0; n < _manipulators.size(); ++n )
      {
         if ( (*_manipulators[ n ]).checkEvent( s.segment, strokeStart, strokeEnd, e ) )
         {
            _lastActivated = _manipulators[ n ];
            _wasActivated = true;
            notify();   // we must redaw all connected segment
            return;
         }
      }
/*
      // while the left button is pressed,
      if ( e.isMouseLeftButtonPressed )
      {
         for ( ui32 n = 0; n < _manipulators.size(); ++n )
         {
            if ( (*_manipulators[ n ]).checkEvent( s.segment, strokeStart, strokeEnd ) )
            {
               _lastActivated = _manipulators[ n ];
               _wasActivated = true;
               notify();   // we must redaw all connected segment
               return;
            }
         }
      } else {
         for ( ui32 n = 0; n < _manipulators.size(); ++n )
         {
            // if the last state wasn't activated && one of the manipulator is activated => we need to redraw the activated manipulator
            if ( (*_manipulators[ n ]).checkEvent( s.segment, strokeStart, strokeStart ) )
            {
               _lastActivated = _manipulators[ n ];
               if ( !_wasActivated )
                  notify();
               _wasActivated = true;
               return;
            }
         }
      }
*/
      _lastActivated.unref(); // no manipulator are activated
      if ( _wasActivated )
      {
         // means last refresh, it was activated, but it is not anymore so we need to display the inactivated state...
         notify();
         _wasActivated = false;
      }
   }
}
}