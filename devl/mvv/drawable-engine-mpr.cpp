# include "stdafx.h"
# include "drawable-engine-mpr.h"

namespace mvv
{
   MprToolkit::~MprToolkit()
   {
      for ( Mprs::iterator it = _mprs.begin(); it != _mprs.end(); ++it )
         (*it)->removeToolkit( *this );
   }

   void MprToolkitMove::_updateMprs()
   {
      for ( Mprs::iterator it = _mprs.begin(); it != _mprs.end(); ++it )
      {
         (*it)->origin.setValue( _position );
         (*it)->zoom.setValue( 0, _zoom[ 0 ] );
         (*it)->zoom.setValue( 1, _zoom[ 1 ] );
      }
   }

   void MprToolkitMove::handle( const InteractionEvent& event, DrawableMprToolkits& source, ResourceImageRGB* mpr )
   {
      if ( event.isMouseLeftButtonPressed )
      {
         if ( !_isLeftCurrentlyPressed )
         {
            _initialOrigin = source.origin.getValue();

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
            _initialZoom[ 0 ] = source.zoom[ 0 ];
            _initialZoom[ 1 ] = source.zoom[ 1 ];
            _initialOrigin = source.origin.getValue();

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
         nll::core::vector3d pos( _initialOrigin[ 0 ] + ( diffMouse[ 0 ] * source.vector1[ 0 ] / source.zoom[ 0 ] + diffMouse[ 1 ] * source.vector2[ 0 ] / source.zoom[ 1 ] ),
                                  _initialOrigin[ 1 ] + ( diffMouse[ 0 ] * source.vector1[ 1 ] / source.zoom[ 0 ] + diffMouse[ 1 ] * source.vector2[ 1 ] / source.zoom[ 1 ] ),
                                  _initialOrigin[ 2 ] + ( diffMouse[ 0 ] * source.vector1[ 2 ] / source.zoom[ 0 ] + diffMouse[ 1 ] * source.vector2[ 2 ] / source.zoom[ 1 ] )  );
         _position = pos;
         _updateMprs();
         return;
      }

      if ( !event.isMouseLeftButtonPressed && event.isMouseRightButtonPressed )
      {
         nll::core::vector2i diffMouse( - event.mousePosition[ 0 ] + _initialMousePos[ 0 ],
                                          event.mousePosition[ 1 ] - _initialMousePos[ 1 ] );
         double sign = ( _initialMousePos[ 1 ] > event.mousePosition[ 1 ] ) ? 1 : -1;
         double d = diffMouse.norm2() * sign;

         nll::core::StaticVector<double, 3> cross = nll::core::cross( source.vector1.getValue(), source.vector2.getValue() );
         assert( nll::core::equal( cross.norm2(), 1.0, 1e-5 ) );  // the base vector1, vector2 must be normalized
         nll::core::vector3d pos( _initialOrigin[ 0 ] + d * cross[ 0 ] / 10,
                                  _initialOrigin[ 1 ] + d * cross[ 1 ] / 10,
                                  _initialOrigin[ 2 ] + d * cross[ 2 ] / 10 );
         _position = pos;
         _updateMprs();
         return;
      }

      if ( event.isMouseLeftButtonPressed && event.isMouseRightButtonPressed )
      {
         nll::core::vector2i diffMouse( - event.mousePosition[ 0 ] + _initialMousePos[ 0 ],
                                          event.mousePosition[ 1 ] - _initialMousePos[ 1 ] );
         double d = diffMouse.norm2();
         if ( event.mousePosition[ 1 ] >= _initialMousePos[ 1 ] )
         {
            _zoom[ 0 ] = _initialZoom[ 0 ] * (double)( 1 + d / 100 );
            _zoom[ 1 ] = _initialZoom[ 1 ] * (double)( 1 + d / 100 );
         } else {
            _zoom[ 0 ] = _initialZoom[ 0 ] / (double)( 1 + d / 100 );
            _zoom[ 1 ] = _initialZoom[ 1 ] / (double)( 1 + d / 100 );
         }
         _updateMprs();
         return;
      }
   }

   void MprToolkitMove::attach( DrawableMprToolkits* r )
   {
      if ( _mprs.size() == 0 )
      {
         // attach and set position to the middle of the biggest volume
         ResourceVolumes::iterator choice;
         ui32 size = 0;
         for ( ResourceVolumes::iterator it = r->volumes.begin(); it != r->volumes.end(); ++it )
         {
            ui32 volSize = (*it)->getSize()[ 0 ] * (*it)->getSize()[ 1 ] * (*it)->getSize()[ 2 ];
            if ( volSize > size )
            {
               size = volSize;
               choice = it;
            }
         }
         _position = (*choice)->indexToPosition( nll::core::vector3d( (*choice)->getSize()[ 0 ] / 2,
                                                                      (*choice)->getSize()[ 1 ] / 2,
                                                                      (*choice)->getSize()[ 2 ] / 2 ) );
         r->origin.setValue( _position );
      }
      _mprs.insert( r );
   }

   //void MprToolkitPoint::run()
  // {
      /*
      // we don't do anything is the target is not within the MPR
      if ( _targetPos[ 0 ] > _mpr.image.sizex() || _targetPos[ 1 ] > _mpr.image.sizey() )
         return true;

      _img.image.clone( _mpr.image );
      ResourceImageRGB::Image::DirectionalIterator it = _img.image.getIterator( _targetPos[ 0 ], 0, 1 );
      for ( ui32 y = 0; y < _img.image.sizey(); ++y )
      {
         *it = 255;
         it.addy();
      }
      it = _img.image.getIterator( 0, _targetPos[ 1 ], 1 );
      for ( ui32 x = 0; x < _img.image.sizex(); ++x )
      {
         *it = 255;
         it.addx();
      }*/
 //  }


   /*
   MprToolkitTarget::MprToolkitTarget( DrawableMprToolkits& toolkits, OrderProvider& orderProvider, const ResourceImageRGB& mpr ) : MprToolkit( toolkits, orderProvider, mpr )
   {
      attach( *( (ResourceImageRGB*)&mpr ) );
      _isLeftCurrentlyPressed = false;
      _targetPos[ 0 ] = toolkits.renderingSize[ 0 ] / 2;
      _targetPos[ 1 ] = toolkits.renderingSize[ 1 ] / 2;
   }

   void MprToolkitTarget::handle( const InteractionEvent& event )
   {
   }

   void MprToolkitMove::handle( const InteractionEvent& event )
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

         nll::core::StaticVector<double, 3> cross = nll::core::cross( _toolkits.vector1.getValue(), _toolkits.vector2.getValue() );
         assert( nll::core::equal( cross.norm2(), 1.0, 1e-5 ) );  // the base vector1, vector2 must be normalized
         nll::core::vector3d pos( _initialOrigin[ 0 ] + d * cross[ 0 ] / 10,
                                  _initialOrigin[ 1 ] + d * cross[ 1 ] / 10,
                                  _initialOrigin[ 2 ] + d * cross[ 2 ] / 10 ); // TODO: use the normal and not (0, 0, 1) vector
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
   */
}