# include "stdafx.h"
# include "drawable-engine-mpr.h"

namespace mvv
{
   void MprToolkitTranslation::handle( const InteractionEvent& event )
   {
      if ( event.isMouseLeftButtonPressed )
      {
         if ( !_isLeftCurrentlyPressed )
         {
            _initialOrigin = _toolkits.origin.getValue();

            if ( !_isLeftCurrentlyPressed )
            {
               _initialMousePos = nll::core::vector2i( event.mousePosition[ 0 ], event.mousePosition[ 1 ] );
            }
            _isLeftCurrentlyPressed = true;
         }
      }

      if ( event.isMouseRightButtonPressed )
      {
         if ( !_isRightCurrentlyPressed )
         {
            _initialZoom[ 0 ] = _toolkits.zoom[ 0 ];
            _initialZoom[ 1 ] = _toolkits.zoom[ 1 ];
            _initialOrigin = _toolkits.origin.getValue();

            if ( !_isLeftCurrentlyPressed )
            {
               _initialMousePos = nll::core::vector2i( event.mousePosition[ 0 ], event.mousePosition[ 1 ] );
            }
            _isRightCurrentlyPressed = true;
         }
      } 
      
      if ( !event.isMouseRightButtonPressed && !event.isMouseLeftButtonPressed )
      {
         _isLeftCurrentlyPressed = false;
         _isRightCurrentlyPressed = false;
      }

      if ( event.isMouseLeftButtonPressed && !event.isMouseRightButtonPressed )
      {
         nll::core::vector2i diffMouse( - event.mousePosition[ 0 ] + _initialMousePos[ 0 ],
                                          event.mousePosition[ 1 ] - _initialMousePos[ 1 ] );
         nll::core::vector3d pos( _initialOrigin[ 0 ] + ( diffMouse[ 0 ] * _toolkits.vector1[ 0 ] / _toolkits.zoom[ 0 ] + diffMouse[ 1 ] * _toolkits.vector2[ 0 ] / _toolkits.zoom[ 1 ] ),
                                  _initialOrigin[ 1 ] + ( diffMouse[ 0 ] * _toolkits.vector1[ 1 ] / _toolkits.zoom[ 0 ] + diffMouse[ 1 ] * _toolkits.vector2[ 1 ] / _toolkits.zoom[ 1 ] ),
                                  _initialOrigin[ 2 ] + ( diffMouse[ 0 ] * _toolkits.vector1[ 2 ] / _toolkits.zoom[ 0 ] + diffMouse[ 1 ] * _toolkits.vector2[ 2 ] / _toolkits.zoom[ 1 ] )  );
         _toolkits.origin.setValue( pos );
         return;
      }

      if ( !event.isMouseLeftButtonPressed && event.isMouseRightButtonPressed )
      {
         nll::core::vector2i diffMouse( - event.mousePosition[ 0 ] + _initialMousePos[ 0 ],
                                          event.mousePosition[ 1 ] - _initialMousePos[ 1 ] );
         double d = diffMouse.norm2();
         nll::core::vector3d pos( _initialOrigin[ 0 ],
                                  _initialOrigin[ 1 ],
                                  _initialOrigin[ 2 ] + d / 10 ); // TODO: use the normal and not (0, 0, 1) vector
         _toolkits.origin.setValue( pos );
         return;
      }

      if ( event.isMouseLeftButtonPressed && event.isMouseRightButtonPressed )
      {
         nll::core::vector2i diffMouse( - event.mousePosition[ 0 ] + _initialMousePos[ 0 ],
                                          event.mousePosition[ 1 ] - _initialMousePos[ 1 ] );
         double d = diffMouse.norm2();
         if ( event.mousePosition[ 1 ] >= _initialMousePos[ 1 ] )
         {
            _toolkits.zoom.setValue( 0, _initialZoom[ 0 ] * (double)( 1 + d / 100 ) );
            _toolkits.zoom.setValue( 1, _initialZoom[ 1 ] * (double)( 1 + d / 100 ) );
         } else {
            _toolkits.zoom.setValue( 0, _initialZoom[ 0 ] / (double)( 1 + d / 100 ) );
            _toolkits.zoom.setValue( 1, _initialZoom[ 1 ] / (double)( 1 + d / 100 ) );
         }
         return;
      }
   }
}