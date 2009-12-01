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

   void MprToolkitMove::handle( const InteractionEvent& event, DrawableMprToolkits& source, ResourceSliceRGB* )
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
         nll::core::vector2i diffMouse( - (int)event.mousePosition[ 0 ] + (int)_initialMousePos[ 0 ],
                                          (int)event.mousePosition[ 1 ] - (int)_initialMousePos[ 1 ] );
         nll::core::vector3f pos( _initialOrigin[ 0 ] + ( diffMouse[ 0 ] * source.vector1[ 0 ] / source.zoom[ 0 ] + diffMouse[ 1 ] * source.vector2[ 0 ] / source.zoom[ 1 ] ),
                                  _initialOrigin[ 1 ] + ( diffMouse[ 0 ] * source.vector1[ 1 ] / source.zoom[ 0 ] + diffMouse[ 1 ] * source.vector2[ 1 ] / source.zoom[ 1 ] ),
                                  _initialOrigin[ 2 ] + ( diffMouse[ 0 ] * source.vector1[ 2 ] / source.zoom[ 0 ] + diffMouse[ 1 ] * source.vector2[ 2 ] / source.zoom[ 1 ] )  );
         _position = pos;
         _updateMprs();
         return;
      }

      if ( !event.isMouseLeftButtonPressed && event.isMouseRightButtonPressed )
      {
         nll::core::vector2i diffMouse( - (int)event.mousePosition[ 0 ] + (int)_initialMousePos[ 0 ],
                                          (int)event.mousePosition[ 1 ] - (int)_initialMousePos[ 1 ] );
         float sign = ( _initialMousePos[ 1 ] > (int)event.mousePosition[ 1 ] ) ? 1 : -1;
         float d = static_cast<float>( fabs( (float)diffMouse[ 1 ] ) ) * sign;

         nll::core::StaticVector<float, 3> cross = nll::core::cross( source.vector1.getValue(), source.vector2.getValue() );
         assert( nll::core::equal( cross.norm2(), 1.0, 1e-5 ) );  // the base vector1, vector2 must be normalized
         nll::core::vector3f pos( _initialOrigin[ 0 ] + d * cross[ 0 ] / 10,
                                  _initialOrigin[ 1 ] + d * cross[ 1 ] / 10,
                                  _initialOrigin[ 2 ] + d * cross[ 2 ] / 10 );
         _position = pos;
         _updateMprs();
         return;
      }

      if ( event.isMouseLeftButtonPressed && event.isMouseRightButtonPressed )
      {
         nll::core::vector2i diffMouse( - (int)event.mousePosition[ 0 ] + (int)_initialMousePos[ 0 ],
                                          (int)event.mousePosition[ 1 ] - (int)_initialMousePos[ 1 ] );
         //double d = diffMouse.norm2();
         float d = static_cast<float>( fabs( (float)diffMouse[ 1 ] ) );
         if ( (int)event.mousePosition[ 1 ] >= _initialMousePos[ 1 ] )
         {
            _zoom[ 0 ] = _initialZoom[ 0 ] * (float)( 1 + d / 100 );
            _zoom[ 1 ] = _initialZoom[ 1 ] * (float)( 1 + d / 100 );
         } else {
            _zoom[ 0 ] = _initialZoom[ 0 ] / (float)( 1 + d / 100 );
            _zoom[ 1 ] = _initialZoom[ 1 ] / (float)( 1 + d / 100 );
         }
         _updateMprs();
         return;
      }
   }

   void MprToolkitMove::attach( DrawableMprToolkits* r )
   {
      if ( _mprs.size() == 0 )
      {
         if ( r->volumes.size() )
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

            // we add 0.5 voxel to be in the center
            _position = (*choice)->indexToPosition( nll::core::vector3f( (*choice)->getSize()[ 0 ] / 2 + 0.5f,
                                                                         (*choice)->getSize()[ 1 ] / 2 + 0.5f,
                                                                         (*choice)->getSize()[ 2 ] / 2 + 0.5f ) );
            r->origin.setValue( _position );
         } else {
            r->origin.setValue( nll::core::vector3f( 0, 0, 0 ) );
         }
      }
      _mprs.insert( r );
   }

   bool MprToolkitPoint::run( DrawableMprToolkits& tk, ResourceSliceRGB* i )
   {
      assert( i ); // it has to be a valid image
      nll::core::vector3f diff( _initialOrigin[ 0 ] - tk.origin[ 0 ],
                                _initialOrigin[ 1 ] - tk.origin[ 1 ],
                                _initialOrigin[ 2 ] - tk.origin[ 2 ] );

      // (0, 0) is the middle of the screen
      nll::core::vector2d pos( i->slice.getStorage().sizex() / 2 + tk.zoom[ 0 ] * ( tk.vector1[ 0 ] * diff[ 0 ] + tk.vector1[ 1 ] * diff[ 1 ] + tk.vector1[ 2 ] * diff[ 2 ] ),
                               i->slice.getStorage().sizey() / 2 + tk.zoom[ 1 ] * ( tk.vector2[ 0 ] * diff[ 0 ] + tk.vector2[ 1 ] * diff[ 1 ] + tk.vector2[ 2 ] * diff[ 2 ] ) );

      if ( pos[ 0 ] >= 0 && pos[ 0 ] < i->slice.getStorage().sizex() &&
           pos[ 1 ] >= 0 && pos[ 1 ] < i->slice.getStorage().sizey() )
      {
         ResourceSliceRGB::Slice::DirectionalIterator it = i->slice.getStorage().getIterator( (ui32)pos[ 0 ], 0, 1 );   
         for ( ui32 y = 0; y < i->slice.getStorage().sizey(); ++y )
         {
            *it = 255;
            it.addy();
         }

         it = i->slice.getStorage().getIterator( 0, (ui32)pos[ 1 ], 1 );
         for ( ui32 x = 0; x < i->slice.getStorage().sizex(); ++x )
         {
            *it = 255;
            it.addx();
         }
      }
      return true;
   }

   void MprToolkitPoint::attach( DrawableMprToolkits* r )
   {
      if ( _mprs.size() == 0 )
      {
         if ( r->volumes.size() )
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

            _initialOrigin = (*choice)->indexToPosition( nll::core::vector3f( (float)(*choice)->getSize()[ 0 ] / 2,
                                                                              (float)(*choice)->getSize()[ 1 ] / 2,
                                                                              (float)(*choice)->getSize()[ 2 ] / 2 ) );
         } else {
            _initialOrigin = nll::core::vector3f( 0, 0, 0 );
         }
      }
      _mprs.insert( r );
   }
}