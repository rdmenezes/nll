#ifndef MVV_PLATFORM_LAYOUT_PANE_TEXTBOX_H_
# define MVV_PLATFORM_LAYOUT_PANE_TEXTBOX_H_

# include "layout-pane.h"
# include "font.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Writable interface, allowing to write on an interface
    */
   class MVVPLATFORM_API Writable
   {
   public:
      virtual void write( const std::string& s, const nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 255 ) ) = 0;
   };

   class PaneTextboxDecorator;

   /**
    @ingroup platform
    @brief Represent a region of the screen, able to react to events and draw in a buffer

    A pane can contains nested sub panes and other panes that will be overlayed on this pane.
    */
   class MVVPLATFORM_API PaneTextbox : public Pane, public Writable
   {
      friend class LayoutPaneDecoratorCursor;
      friend class LayoutPaneDecoratorCursorPosition;
      friend class LayoutPaneDecoratorCursorBasic;
      friend class LayoutPaneDecoratorCursorYDirection;
      friend class LayoutPaneDecoratorCursorEnterFile;
      friend class LayoutPaneDecoratorCursorEnterConsole;

   public:
      struct Format
      {
         Format( const std::string& s ) : text( s ), color( nll::core::vector3uc( 255, 255, 255 ) )
         {}

         Format()
         {}

         Format( const std::string& s, const nll::core::vector3uc& col ) : text( s ), color( col )
         {}

         std::string             text;
         nll::core::vector3uc    color;
      };

      PaneTextbox( const nll::core::vector2ui& origin,
                   const nll::core::vector2ui& size,
                   RefcountedTyped<Font>& font,
                   ui32 textSize = 12,
                   const nll::core::vector3uc textColor = nll::core::vector3uc( 255, 255, 255 ),
                   const nll::core::vector3uc background = nll::core::vector3uc( 0, 0, 0 ),
                   bool displayEndToFront = false ) : Pane( origin, size, background ), _font( font ), _textSize( textSize ), _textColor( textColor )
      {
         ensure( textSize > 0, "error: text size is <= 0" );

         _needToBeRefreshed = true;
         _displayEndToFront = displayEndToFront;
         _text.push_back( Format( "", textColor ) );
      }

      ui32 getTextSize() const
      {
         return _textSize;
      }

      // return a type of decorator
      template <class Decorator>
      Decorator* get()
      {
         for ( ui32 n = 0; n < _decorators.size(); ++n )
         {
            Decorator* d = dynamic_cast<Decorator*>( &( *_decorators[ n ] ) );
            if ( d )
               return d;
         }
         return 0;
      }

      virtual ~PaneTextbox()
      {
      }

      virtual void updateLayout()
      {
         notify();
      }

      void add( RefcountedTyped<PaneTextboxDecorator>& decorator )
      {
         _decorators.push_back( decorator );
      }

      virtual void write( const std::string& s, const nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 255 ) )
      {
         if ( _text.size() == 1 && _text[ 0 ].text == "" )
            _text.clear();

         _text.push_back( Format( s, color ) );
         notify();
      }

   protected:
      virtual void _draw( Image& image );

      // draw all the text stored and visible in the current window
      virtual void _drawText( Image& image )
      {
         const nll::core::vector2ui maxPos( _size[ 0 ] + _origin[ 0 ], _size[ 1 ] + _origin[ 1 ] );
         const ui32 charxsize = _textSize;
         const ui32 charysize = _textSize;
         const ui32 nbLine = _size[ 1 ] / charysize;

         const ui32 line = _windowPosition[ 1 ] / charysize;   // the first line to be displayed
         const ui32 character = _windowPosition[ 0 ] / charxsize;

         (*_font).setColor( _textColor );
         (*_font).setSize( _textSize );
         for ( ui32 y = 0; y < nbLine; ++y )
         {
            if ( !_displayEndToFront )
            {
               if ( _text.size() > ( y + line ) && _text[ y + line ].text.size() > character )
               {
                  const ui32 posx = getCharacterPositionX( 0, y );
                  const ui32 posy = getCharacterPositionY( y );

                  Format& format = _text[ y + line ];
                  (*_font).setColor( format.color );

                  const char* text = format.text.c_str() + character;
                  (*_font).write( text, nll::core::vector2ui( posx, posy ), image, nll::core::vector2ui( 0, 0 ), maxPos );
               }
            } else {
               const int ytext = (int)_text.size() - 1 - y - line;
               if ( ytext >= 0 && ytext < (int)_text.size() && _text[ ytext ].text.size() > character )
               {
                  const ui32 posx = getCharacterPositionX( 0, y );
                  const ui32 posy = getCharacterPositionY( y );

                  Format& format = _text[ ytext ];
                  (*_font).setColor( format.color );

                  const char* text = format.text.c_str() + character;
                  (*_font).write( text, nll::core::vector2ui( posx, posy ), image, nll::core::vector2ui( 0, 0 ), maxPos );
               }
            }
         }
      }

      ui32 getCharacterPositionY( ui32 lineNumberInText )
      {
         const ui32 charysize = _textSize;
         const ui32 line = _windowPosition[ 1 ] / charysize;
         const ui32 dy = _windowPosition[ 1 ] % charysize;
         return _origin[ 1 ] + _size[ 1 ] - charysize - ( charysize * lineNumberInText + line + dy );
      }

      ui32 getCharacterPositionX( ui32 colNumberInText, ui32 lineNumberInText )
      {
         const ui32 charxsize = _textSize;
         const ui32 col = _windowPosition[ 0 ] / charxsize;
         const ui32 dx = _windowPosition[ 0 ] % charxsize;

         if ( _text.size() == 0 )
            return 0; // there is nothing to display!
         assert( lineNumberInText <= _text.size() );
         assert( colNumberInText <= _text[ lineNumberInText ].text.size() );
         const ui32 posx = (*_font).getSize( _text[ lineNumberInText ].text, nll::core::vector2ui( 0, 0 ), colNumberInText );
         return _origin[ 0 ] + posx + col + dx;
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
      std::vector<Format>                          _text;
      bool                                         _displayEndToFront;

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

   // stores the attributs shared by the cursor decroators
   class MVVPLATFORM_API LayoutPaneDecoratorCursorPosition : public PaneTextboxDecorator
   {
      typedef PaneTextboxDecorator   Base;

   public:
      LayoutPaneDecoratorCursorPosition( PaneTextbox& src ) : Base( src )
      {
         currentLine = 0;
         currentChar = 0;
      }

      virtual void draw( Image& i )
      {
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;
         ui32& _currentChar = position->currentChar;

         ui32 ypos = _src.getCharacterPositionY( _currentLine );
         ui32 xpos = _src.getCharacterPositionX( _currentChar, _currentLine );
         if ( ypos >= _src._origin[ 1 ] && ypos < _src._origin[ 1 ] + _src._size[ 1 ] &&
              xpos >= _src._origin[ 0 ] && xpos < _src._origin[ 0 ] + _src._size[ 0 ] )
         {
            Image::DirectionalIterator line = i.getIterator( xpos, ypos, 0 );
            for ( ui32 n = 0; n < _src._textSize / 2; ++n )
            {
               line.pickcol( 0 ) = _src._textColor[ 0 ];
               line.pickcol( 1 ) = _src._textColor[ 1 ];
               line.pickcol( 2 ) = _src._textColor[ 2 ];
               line.addx();
            }
         }
      }

      virtual bool receive( const EventMouse& )
      {
         return false;
      }

      virtual bool receive( const EventKeyboard& )
      {
         return false;
      }

      ui32 currentLine;
      ui32 currentChar;
   };

   // handle home, end, supr, backspace, left, right
   class MVVPLATFORM_API LayoutPaneDecoratorCursorBasic : public PaneTextboxDecorator
   {
      typedef PaneTextboxDecorator   Base;

   public:
      LayoutPaneDecoratorCursorBasic( PaneTextbox& src ) : Base( src )
      {
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
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;
         ui32& _currentChar = position->currentChar;

         if ( e.key == EventKeyboard::KEY_SUPR )
         {
            ui32 lineSize = (ui32)_src._text[ _currentLine ].text.size();
            if ( _currentChar < lineSize )
            {
               for ( ui32 n = _currentChar; n < _src._text[ _currentLine ].text.size(); ++n )
               _src._text[ _currentLine ].text[ n ] = _src._text[ _currentLine ].text[ n + 1 ];
               _src._text[ _currentLine ].text.resize( lineSize - 1 );
            } else {
               if ( _src._text.size() > _currentLine + 1 )
               {
                  ui32 nextLineSize = (ui32)_src._text[ _currentLine + 1 ].text.size();
                  _src._text[ _currentLine ].text.resize( lineSize + nextLineSize );
                  for ( ui32 n = 0; n < nextLineSize; ++n )
                  {
                     char c = _src._text[ _currentLine + 1 ].text[ n ];
                     _src._text[ _currentLine ].text[ _currentChar + n ] = c;
                  }

                  std::vector<PaneTextbox::Format>::iterator it = _src._text.begin() + _currentLine + 1;
                  _src._text.erase( it );
               }
            }

            _src.notify();
            return true;
         }

         if ( e.key == EventKeyboard::KEY_END )
         {
            _currentChar = (ui32)_src._text[ _currentLine ].text.size();
            _src.notify();
            return true;
         }

         if ( e.key == EventKeyboard::KEY_HOME )
         {
            _currentChar = 0;
            _src.notify();
            return true;
         }

         if ( e.key == EventKeyboard::KEY_BACKSPACE )
         {
            if ( _currentChar )
            {
               --_currentChar;
               for ( ui32 n = _currentChar; n < _src._text[ _currentLine ].text.size(); ++n )
                  _src._text[ _currentLine ].text[ n ] = _src._text[ _currentLine ].text[ n + 1 ];
               _src._text[ _currentLine ].text.resize( _src._text[ _currentLine ].text.size() - 1 );
            } else {
               if ( _currentLine )
               {
                  const ui32 size = (ui32)_src._text[ _currentLine - 1 ].text.size();
                  _src._text[ _currentLine - 1 ].text.insert( size, _src._text[ _currentLine ].text );

                  std::vector<PaneTextbox::Format>::iterator it = _src._text.begin() + _currentLine;
                  _src._text.erase( it );

                  --_currentLine;
                  _currentChar = size;
               }
            }
            _src.notify();
            return true;
         }

         if ( e.key == EventKeyboard::KEY_LEFT )
         {
            if ( _currentChar )
            {
               --_currentChar;
               _src.notify();
               return true;
            } else {
               if ( _currentLine )
               {
                  --_currentLine;
                  _currentChar = (ui32)_src._text[ _currentLine ].text.size();
                  _src.notify();
                  return true;
               } else return true;
            }
         }

         if ( e.key == EventKeyboard::KEY_RIGHT )
         {
            if ( _currentChar >= _src._text[ _currentLine ].text.size() )
            {
               if ( _currentLine + 1 < _src._text.size() )
               {
                  _currentChar = 0;
                  ++_currentLine;
                  _src.notify();
               } else {
                  _currentChar = (ui32)_src._text[ _currentLine ].text.size();
               }
               return true;
            } else {
               ++_currentChar;
               _src.notify();
               return true;
            }
         }
         return false;
      }
   };

   // handle cursor direction: left, right
   class MVVPLATFORM_API LayoutPaneDecoratorCursorYDirection : public PaneTextboxDecorator
   {
      typedef PaneTextboxDecorator   Base;

   public:
      LayoutPaneDecoratorCursorYDirection( PaneTextbox& src ) : Base( src )
      {
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
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;
         ui32& _currentChar = position->currentChar;

         if ( e.key == EventKeyboard::KEY_DOWN )
         {
            if ( _currentLine + 1 < _src._text.size() )
            {
               ++_currentLine;
               if ( _src._text[ _currentLine ].text.size() < _currentChar )
                  _currentChar = (ui32)_src._text[ _currentLine ].text.size();
               _src.notify();
               return true;
            } else {
               _currentChar = (ui32)_src._text[ _currentLine ].text.size();
               _src.notify();
               return true;
            }
         }

         if ( e.key == EventKeyboard::KEY_UP )
         {
            if ( _currentLine )
            {
               --_currentLine;
               if ( _src._text[ _currentLine ].text.size() < _currentChar )
                  _currentChar = (ui32)_src._text[ _currentLine ].text.size();
               _src.notify();
               return true;
            } else {
               _currentChar = 0;
               _src.notify();
               return true;
            }
         }
         return false;
      }
   };

   class MVVPLATFORM_API LayoutPaneDecoratorCursorEnterFile : public PaneTextboxDecorator
   {
      typedef PaneTextboxDecorator   Base;

   public:
      LayoutPaneDecoratorCursorEnterFile( PaneTextbox& src ) : Base( src )
      {
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
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;
         ui32& _currentChar = position->currentChar;

         if ( e.key == EventKeyboard::KEY_ENTER )
         {
            // copy from the cursor->EOL the new line
            std::string s;
            const size_t sizenl = _src._text[ _currentLine ].text.size() - _currentChar;
            if ( sizenl )
            {
               s.resize( sizenl );
               for ( ui32 n = 0; n < sizenl; ++n )
                  s[ n ] = _src._text[ _currentLine ].text[ n + _currentChar ];
            }
            else
            {
               s = "";
            }

            // update
            std::vector<PaneTextbox::Format>::iterator it = _src._text.begin() + _currentLine + 1;
            _src._text.insert( it, s );
            _src._text[ _currentLine ].text.resize( _currentChar );
            _src.notify();
            ++_currentLine;
            _currentChar = 0;
            return true;
         }
         return false;
      }
   };

   class MVVPLATFORM_API LayoutPaneDecoratorCursorEnterConsole : public PaneTextboxDecorator
   {
      typedef PaneTextboxDecorator   Base;

   public:
      LayoutPaneDecoratorCursorEnterConsole( PaneTextbox& src, Writable& writable ) : Base( src ), _writable( writable )
      {
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
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( e.key == EventKeyboard::KEY_ENTER )
         {
            if ( _src._text.size() > 0 && _src._text[ 0 ].text.size() > 0 )
            {
               std::stringstream ss;
               for ( ui32 n = 0; n < _src._text.size(); ++n )
               {
                  ss << _src._text[ n ].text;
                  if ( n + 1 != _src._text.size() )
                     ss << std::endl;
               }

               _writable.write( ss.str() );

               if ( position )
               {
                  position->currentChar = 0;
                  position->currentLine = 0;
               }
               _src._text = nll::core::make_vector<PaneTextbox::Format>( PaneTextbox::Format("", _src._textColor) );
            }

            _src.notify();
            return true;
         }
         return false;
      }

   private:
      Writable&   _writable;
   };

   // all special characters should be handled by the other decorators
   // this decorator only handle printable character
   class MVVPLATFORM_API LayoutPaneDecoratorCursor : public PaneTextboxDecorator
   {
      typedef PaneTextboxDecorator   Base;

   public:
      LayoutPaneDecoratorCursor( PaneTextbox& src ) : Base( src )
      {
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
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;
         ui32& _currentChar = position->currentChar;

         if ( e.key == EventKeyboard::KEY_TAB )
         {
            _src._text[ _currentLine ].text.insert( _currentChar, 3, ' ' );
            _currentChar += 3;
            _src.notify();
            return true;
         }

         // normal character
         ensure( _currentChar < 256 && _currentChar >= 0, "at this point it must be an ASCII character..." );
         _src._text[ _currentLine ].text.insert( _currentChar, 1, (char)e.key );
         ++_currentChar;
         std::cout << "receive key=" << e.key << " shift=" << e.isShift << std::endl;
         _src.notify();
         return true;
      }
   };
}
}
#endif