#include "layout-pane.h"

namespace mvv
{
namespace platform
{
   Pane::Pane( const nll::core::vector2ui& origin,
               const nll::core::vector2ui& size,
               const nll::core::vector3uc background ) : _origin( origin ), _size( size ), _visible( true ), _disabled( false ), _background( background )
   {}

   const nll::core::vector2ui& Pane::getOrigin() const
   {
      return _origin;
   }

   const nll::core::vector2ui& Pane::getSize() const
   {
      return _size;
   }

   bool Pane::isInside( const nll::core::vector2ui& pos )
   {
      return pos[ 0 ] >= _origin[ 0 ] &&
             pos[ 1 ] >= _origin[ 1 ] &&
             pos[ 0 ] < ( _origin[ 0 ] + _size[ 0 ] ) &&
             pos[ 1 ] < ( _origin[ 1 ] + _size[ 1 ] );
   }

   void Pane::setVisible( bool isVisible )
   {
      if ( isVisible != _visible )
      {
         _visible = isVisible;

         if ( !_visible )
         {
            setSize( nll::core::vector2ui( 0, 0 ) );
         }
      }
   }

   bool Pane::isVisible() const
   {
      return _visible;
   }

   void Pane::setOrigin( const nll::core::vector2ui& origin )
   {
      if ( origin != _origin )
      {
         _origin = origin;
         updateLayout();
      }
   }

   void Pane::setSize( const nll::core::vector2ui& size )
   {
      if ( size != _size )
      {
         _size = size;
         updateLayout();
      }
   }

   void Pane::draw( Image& image )
   {
      if ( !_visible )
         return;
      _draw( image );

      for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it )
      {
         (**it).draw( image );
      }
   }
}
}