#ifndef MVV_PLATFORM_LAYOUT_PANE_TEXTBOX_H_
# define MVV_PLATFORM_LAYOUT_PANE_TEXTBOX_H_

# include "layout-pane.h"
# include "font.h"

namespace mvv
{
namespace platform
{
   class PaneTextboxDecorator;

   /**
    @ingroup platform
    @brief Represent a region of the screen, able to react to events and draw in a buffer

    A pane can contains nested sub panes and other panes that will be overlayed on this pane.
    */
   class MVVPLATFORM_API PaneTextbox : public Pane
   {
      friend class LayoutPaneDecoratorCursor;

   public:
      PaneTextbox( const nll::core::vector2ui& origin,
                   const nll::core::vector2ui& size,
                   RefcountedTyped<Font>& font,
                   ui32 textSize = 12,
                   const nll::core::vector3uc textColor = nll::core::vector3uc( 255, 255, 255 ),
                   const nll::core::vector3uc background = nll::core::vector3uc( 0, 0, 0 ) ) : Pane( origin, size, background ), _font( font ), _textSize( textSize ), _textColor( textColor )
      {
         _needToBeRefreshed = true;
         ensure( textSize > 0, "error: text size is <= 0" );

         _text.push_back( "123465789" );
         _text.push_back( "abcdefg" );
         _text.push_back( "123486abcdefg" );
      }

      virtual ~PaneTextbox()
      {
      }

      virtual void updateLayout()
      {
      }

      void add( RefcountedTyped<PaneTextboxDecorator>& decorator )
      {
         _decorators.push_back( decorator );
      }

   protected:
      virtual void _draw( Image& image );

      // draw all the text stored and visible in the current window
      virtual void _drawText( Image& image )
      {
         const ui32 charxsize = _textSize;
         const ui32 charysize = _textSize;

         const ui32 nbLine = _size[ 1 ] / charysize;
         const ui32 character = _windowPosition[ 0 ] / charxsize;
         const ui32 line = _windowPosition[ 1 ] / charysize;
         const ui32 dx = _windowPosition[ 0 ] % charxsize;
         const ui32 dy = _windowPosition[ 1 ] % charysize;

         const nll::core::vector2ui maxPos( _size[ 0 ] + _origin[ 0 ], _size[ 1 ] + _origin[ 1 ] );

         (*_font).setColor( _textColor );
         (*_font).setSize( _textSize );
         for ( ui32 y = 0; y < nbLine; ++y )
         {
            if ( _text.size() > ( y + line ) &&
                 _text[ y + line ].size() > character )
            {
               const char* text = _text[ y + line ].c_str() + character;
               (*_font).write( text, nll::core::vector2ui( dx, _size[ 1 ] - 1 - ( charysize + dy + charysize * y ) ), image, nll::core::vector2ui( 0, 0 ), maxPos );
            }
         }
      }

      ui32 getCharacterPositionY( ui32 line )
      {
         return _textSize * line;
      }

      virtual void _receive( const EventMouse& e );

      virtual void _receive( const EventKeyboard& e );

      // force the layout to be redraw next time
      void notify()
      {
         _needToBeRefreshed = true;
      }

   protected:
      bool                                                  _needToBeRefreshed;
      std::vector< RefcountedTyped<PaneTextboxDecorator> >  _decorators;

      // window properties
      // a window is defined by the visible size, a maximum size and a poistion
      // when displayed, we draw the window relatively to this position and size
      nll::core::vector2ui                         _windowPosition;  // relative to (0, 0) -> _maxWindowSize
      nll::core::vector2ui                         _maxWindowSize;

      // text properties
      RefcountedTyped<Font>                        _font;
      ui32                                         _textSize;
      nll::core::vector3uc                         _textColor;
      std::vector<std::string>                     _text;

   };

   class MVVPLATFORM_API PaneTextboxDecorator
   {
   public:
      PaneTextboxDecorator( PaneTextbox& src ) : _src( src ) {}

      virtual ~PaneTextboxDecorator(){}

      // draw the decorator
      virtual void draw( Image& ) = 0;

      // receive the event, return true if the event is intercepted (and not propagated to other decorators...)
      virtual bool receive( const EventMouse& ) = 0;

      // receive the event, return true if the event is intercepted (and not propagated to other decorators...)
      virtual bool receive( const EventKeyboard& ) = 0;

   private:
      PaneTextboxDecorator& operator=( const PaneTextboxDecorator& );
      PaneTextboxDecorator( const PaneTextboxDecorator& );

   protected:
      PaneTextbox&  _src;
   };

   class MVVPLATFORM_API LayoutPaneDecoratorCursor : public PaneTextboxDecorator
   {
      typedef PaneTextboxDecorator   Base;

   public:
      LayoutPaneDecoratorCursor( PaneTextbox& src ) : Base( src )
      {
         _currentLine = 0;
         _currentChar = 0;
      }

      virtual void draw( Image& )
      {
      }

      virtual bool receive( const EventMouse& )
      {
         return false;
      }

      virtual bool receive( const EventKeyboard& e )
      {
         // erase character
         if ( e.key == EventKeyboard::KEY_BACKSPACE )
         {
            if ( _currentChar )
            {
               --_currentChar;
               for ( ui32 n = _currentChar; n < _src._text[ _currentLine ].size(); ++n )
                  _src._text[ _currentLine ][ n ] = _src._text[ _currentLine ][ n + 1 ];
            } else {
               if ( _currentLine )
               {
                  const ui32 size = (ui32)_src._text[ _currentLine - 1 ].size();
                  _src._text[ _currentLine - 1 ].insert( size, _src._text[ _currentLine ] );

                  std::vector<std::string>::iterator it = _src._text.begin() + _currentLine;
                  _src._text.erase( it );

                  --_currentLine;
                  _currentChar = size;
               }
            }
            _src.notify();
            return false;
         }

         // end of line
         if ( e.key == EventKeyboard::KEY_ENTER )
         {
            // copy from the cursor->EOL the new line
            std::string s;
            const size_t sizenl = _src._text[ _currentLine ].size() - _currentChar;
            if ( sizenl )
            {
               s.resize( sizenl );
               for ( ui32 n = 0; n < sizenl; ++n )
                  s[ n ] = _src._text[ _currentLine ][ n + _currentChar ];
            }
            else
            {
               s = "";
            }

            // update
            std::vector<std::string>::iterator it = _src._text.begin() + _currentLine + 1;
            _src._text.insert( it, s );
            _src._text[ _currentLine ].resize( _currentChar );
            _src.notify();
            ++_currentLine;
            _currentChar = 0;
            return false;
         }

         // normal character
         ensure( _currentChar < 256 && _currentChar >= 0, "at this point it must be an ASCII character..." );
         _src._text[ _currentLine ].insert( _currentChar, 1, (char)e.key );
         ++_currentChar;
         std::cout << "receive key=" << e.key << " shift=" << e.isShift << std::endl;
         _src.notify();
         return false;
      }

      ui32 _currentLine;
      ui32 _currentChar;
   };
}
}
#endif