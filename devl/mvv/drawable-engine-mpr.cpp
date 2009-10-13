# include "stdafx.h"
# include "drawable-engine-mpr.h"

namespace mvv
{
   void MprToolkitTranslation::handle( const InteractionEvent& event )
   {
      if ( event.isMouseLeftButtonPressed )
      {
         if ( !_isCurrentlyPressed )
         {
            _initialOrigin = _toolkits.origin.getValue();
            _initialMousePos = nll::core::vector2i( event.mousePosition[ 0 ], event.mousePosition[ 1 ] );
            _isCurrentlyPressed = true;

            // nothing to do
            return;
         }

         nll::core::vector2i diffMouse( - event.mousePosition[ 0 ] + _initialMousePos[ 0 ],
                                        event.mousePosition[ 1 ] - _initialMousePos[ 1 ] );
         nll::core::vector3d pos( _initialOrigin[ 0 ] + ( diffMouse[ 0 ] * _toolkits.vector1[ 0 ] / _toolkits.zoom[ 0 ] + diffMouse[ 1 ] * _toolkits.vector2[ 0 ] / _toolkits.zoom[ 1 ] ),
                                  _initialOrigin[ 1 ] + ( diffMouse[ 0 ] * _toolkits.vector1[ 1 ] / _toolkits.zoom[ 0 ] + diffMouse[ 1 ] * _toolkits.vector2[ 1 ] / _toolkits.zoom[ 1 ] ),
                                  _initialOrigin[ 2 ] + ( diffMouse[ 0 ] * _toolkits.vector1[ 2 ] / _toolkits.zoom[ 0 ] + diffMouse[ 1 ] * _toolkits.vector2[ 2 ] / _toolkits.zoom[ 1 ] )  );
         _toolkits.origin.setValue( pos );
      } else {
         _isCurrentlyPressed = false;
      }
   }
}