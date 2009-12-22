#ifndef MVV_PLATFORM_LAYOUT_WIDGET_H_
# define MVV_PLATFORM_LAYOUT_WIDGET_H_

# include "layout-pane.h"

namespace mvv
{
namespace platform
{
   class WidgetDropDown : public Pane
   {
   public:
      WidgetDropDown( const nll::core::vector2ui& origin,
                      const nll::core::vector2ui& size,
                      PaneRef entry,
                      Panes& list,
                      const nll::core::vector3uc background = nll::core::vector3uc( 255, 255, 255 ) ) : Pane( origin, size, background  ), _entry( entry )
      {
         for ( Panes::iterator it = list.begin(); it != list.end(); ++it )
         {
            insert( *it );
            (**it).setVisible( false );
         }
         _dropDownActive = false;
         _entrySize = size;
         _choice = -1;

         updateLayout();
      }

      virtual void _draw( Image& i )
      {
         (*_entry).draw( i );
      }

      virtual void updateLayout()
      {
         ui32 n = 0;

         (*_entry).setSize( _entrySize );
         (*_entry).setOrigin( _origin );
         (*_entry).updateLayout();
         for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it, ++n )
         {
            (**it).setSize( _entrySize );
            (**it).setOrigin( nll::core::vector2ui( _origin[ 0 ], _origin[ 1 ] - ( n + 1 ) * _entrySize[ 1 ] ) );
            (**it).updateLayout();
         }
      }

      virtual bool dispatchMouseEventToWidgetOnly( const EventMouse& ) const
      {
         // if the drop down is active, it must catch the event
         return _dropDownActive;
      }

   protected:
      virtual void _receive( const EventMouse& e )
      {
         std::cout << "event" << this << std::endl;
         // if a widget has the control
         for ( Panes::reverse_iterator it = _widgets.rbegin(); it != _widgets.rend(); ++it )
         {
            if ( (**it).dispatchMouseEventToWidgetOnly( e ) )
            {
               // we dispatch it to the child only, it is prioritary
               (**it).receive( e );
               return;
            }
         }

         // else...
         std::cout << "receive-widget=" << e.mouseLeftClickedPosition[ 0 ] << " " << e.mouseLeftClickedPosition[ 1 ] << std::endl;
         if ( !_dropDownActive )
         {
            if ( e.isMouseLeftButtonJustPressed && isInside( e.mouseLeftClickedPosition ) )
            {
               std::cout << "actif:" << this << std::endl;
               setDropDownActive( true );
               return;
            }
         } else {
            if ( e.isMouseLeftButtonJustPressed && !isInside( e.mouseLeftClickedPosition ) )
            {
               int n = 0;
               _choice = -1;
               for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it, ++n )
               {
                  if ( (**it).isInside( e.mouseLeftClickedPosition ) )
                  {
                     _choice = n;
                  }
               }
               std::cout << "inactif" << this << std::endl;
               setDropDownActive( false );
            }
         }
      }

      void setDropDownActive( bool visible )
      {
         if ( visible != _dropDownActive )
         {
            for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it )
            {
               (**it).setVisible( visible );
            }
            _dropDownActive = visible;
            updateLayout();
         }
      }

   protected:
      PaneRef              _entry;
      bool                 _dropDownActive;
      nll::core::vector2ui _entrySize;
      int                 _choice;
   };
}
}

#endif
