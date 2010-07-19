#ifndef MVV_PLATFORM_LAYOUT_PANE_TEXTBOX_H_
# define MVV_PLATFORM_LAYOUT_PANE_TEXTBOX_H_

# include "layout-pane.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Represent a region of the screen, able to react to events and draw in a buffer

    A pane can contains nested sub panes and other panes that will be overlayed on this pane.
    */
   class MVVPLATFORM_API PaneTextbox : public Pane
   {
      friend class Decorator;

   public:
      class Decorator
      {
      public:
         Decorator( PaneTextbox& src ) : _src( src ) {}

         virtual ~Decorator(){}

         // draw the decorator
         virtual void draw( Image& ) = 0;

         // receive the event, return true if the event is intercepted (and not propagated to other decorators...)
         virtual bool receive( const EventMouse& );

         // receive the event, return true if the event is intercepted (and not propagated to other decorators...)
         virtual bool receive( const EventKeyboard& );

      private:
         Decorator& operator=( const Decorator& );
         Decorator( const Decorator& );

      protected:
         PaneTextbox&  _src;
      };

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

      void add( RefcountedTyped<Decorator>& decorator )
      {
         _decorators.push_back( decorator );
      }

   protected:
      virtual void _draw( Image& image )
      {
         if ( _needToBeRefreshed )
         {
            std::cout << "textbox refreshed" << std::endl;
            _drawText( image );

            // finally decorate the text...
            for ( int n = 0; n < _decorators.size(); ++n )
            {
               (*_decorators[ n ]).draw( image );
            }
            _needToBeRefreshed = false;
         }
      }

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

      virtual void _receive( const EventMouse& )
      {
      }

      virtual void _receive( const EventKeyboard& )
      {
      }

      // force the layout to be redraw next time
      void notify()
      {
         _needToBeRefreshed = true;
      }

   protected:
      bool                                         _needToBeRefreshed;
      std::vector< RefcountedTyped<Decorator> >    _decorators;

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
}
}
#endif