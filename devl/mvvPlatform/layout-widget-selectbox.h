#ifndef MVV_PLATFORM_WIDGET_SELECTBOX_H_
# define MVV_PLATFORM_WIDGET_SELECTBOX_H_

# include "layout-pane.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Allows to display a list of text within the current window
    */
   class MVVPLATFORM_API WidgetSelectBox : public Pane
   {
   public:
      // forceRefresh : refresh all the time this widget
      WidgetSelectBox( bool forceRefresh, const nll::core::vector2ui& origin, ui32 sizex, const std::vector<std::string>& choices, int& choice, RefcountedTyped<Font>& font, ui32 fontSize = 14, nll::core::vector3uc background = nll::core::vector3uc(127, 127, 127) ) : Pane( origin, nll::core::vector2ui( 1, 1 ) ), _forceRefresh( forceRefresh ), _sizex( sizex ), _choices( choices ), _choice( choice ), _font( font ), _fontSize( fontSize ), _background( background )
      {
         // init the choice to nothing
         _choice = -1;
         _currentChoice = 0;
      }

      virtual void draw( Image& i )
      {
         if ( _forceRefresh || _needToBeRefreshed )
         {
            _draw( i );
            _needToBeRefreshed = false;
         }
      }

      virtual void _draw( Image& i )
      {
         const ui32 sizey = 2 * _fontSize;
         const ui32 maxy = (*_father).getOrigin()[ 1 ] + (*_father).getSize()[ 1 ];
         const ui32 maxx = (*_father).getOrigin()[ 0 ] + (*_father).getSize()[ 0 ];

         ui32 col = 0;
         ui32 row = 0;
         for ( ui32 n = 0; n < _choices.size(); ++n )
         {
            if ( _origin[ 1 ] + sizey * ( row + 1 ) < maxy )
            {
               _displayBox( i, nll::core::vector2ui( _origin[ 0 ] + _sizex * col, _origin[ 1 ] + sizey * row ), nll::core::vector2ui( _sizex, sizey ), _background, _choices[ n ], _currentChoice == n );
               ++row;
            } else {
               row = 0;
               ++col;
               if ( _origin[ 0 ] + _sizex * ( col + 1 ) >= maxx )
               {
                  // we are out of the window...
                  break;
               } else {
                  _displayBox( i, nll::core::vector2ui( _origin[ 0 ] + _sizex * col, _origin[ 1 ] + sizey * row ), nll::core::vector2ui( _sizex, sizey ), _background, _choices[ n ], _currentChoice == n );
                  ++row;
               }
            }
         }
      }

      void _displayBox( Image& i, const nll::core::vector2ui& position, const nll::core::vector2ui& size, const nll::core::vector3uc& background, const std::string& text, bool selected )
      {
         const nll::core::vector3uc dark = background / 2;
         const nll::core::vector3uc back = selected ? background : dark;

         Image::DirectionalIterator line = i.getIterator( position[ 0 ], position[ 1 ], 0 );
         for ( ui32 y = 0; y < size[ 1 ] - 1; ++y )
         {
            Image::DirectionalIterator pixel = line;
            for ( ui32 x = 0; x < size[ 0 ]; ++x )
            {
               pixel.pickcol( 0 ) = back[ 0 ];
               pixel.pickcol( 1 ) = back[ 1 ];
               pixel.pickcol( 2 ) = back[ 2 ];
               pixel.addx();
            }

            line.addy();
         }

         for ( ui32 x = 0; x < size[ 0 ]; ++x )
         {
            line.pickcol( 0 ) = background[ 0 ];
            line.pickcol( 1 ) = background[ 1 ];
            line.pickcol( 2 ) = background[ 2 ];
            line.addx();
         }

         Image::DirectionalIterator linev = i.getIterator( position[ 0 ] + size[ 0 ] - 1, position[ 1 ], 0 );
         for ( ui32 y = 0; y < size[ 1 ]; ++y )
         {
            linev.pickcol( 0 ) = background[ 0 ];
            linev.pickcol( 1 ) = background[ 1 ];
            linev.pickcol( 2 ) = background[ 2 ];
            linev.addy();
         }

         (*_font).setColor( nll::core::vector3uc( 255, 255, 255 ) );
         (*_font).setSize( _fontSize );
         (*_font).write( text, position, i, position, nll::core::vector2ui( position[ 0 ] + size[ 0 ], position[ 1 ] + size[ 1 ] ) );
      }

      virtual void updateLayout()
      {
      }

      virtual bool dispatchMouseEventToWidgetOnly( const EventKeyboard& e ) const
      {
         return e.key == EventKeyboard::KEY_UP || e.key == EventKeyboard::KEY_DOWN || e.key == EventKeyboard::KEY_ENTER;
      }

      virtual void _receive( const EventKeyboard& e )
      {
         if ( e.key == EventKeyboard::KEY_UP )
         {
            if ( _currentChoice + 1 < _choices.size() )
            {
               notify();
               ++_currentChoice;
            }
            return;
         }

         if ( e.key == EventKeyboard::KEY_DOWN )
         {
            if ( _currentChoice > 0 )
            {
               notify();
               --_currentChoice;
            }
            return;
         }

         if ( e.key == EventKeyboard::KEY_ENTER )
         {
            std::cout << "ENTER" << std::endl;
            _choice = _currentChoice;
            return;
         }
      }

      virtual Pane* find( const mvv::Symbol& )
      {
         return 0;
      }

      virtual void destroy()
      {
         // do nothing
      }

   private:
      WidgetSelectBox( const WidgetSelectBox& );
      WidgetSelectBox& operator=( const WidgetSelectBox& );

   private:
      bool                             _forceRefresh;
      ui32                             _sizex;
      std::vector<std::string>         _choices;
      int&                             _choice;
      ui32                             _currentChoice;
      RefcountedTyped<Font>            _font;
      ui32                             _fontSize;

      nll::core::vector3uc             _background;
   };
}
}
#endif