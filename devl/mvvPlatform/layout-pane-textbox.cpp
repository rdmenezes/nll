#include "layout-pane-textbox.h"
#include "layout-pane-cmdl.h"

namespace mvv
{
namespace platform
{
   void PaneTextbox::draw( Image& image )
   {
      if ( !_visible )
         return;

      for ( ui32 n = 0; n < _decorators.size(); ++n )
      {
         ( *_decorators[ n ] ).process();
      }

      if ( _needToBeRefreshed )
      {
         _draw( image );
         _needToBeRefreshed = false;
      }

      for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it )
      {
         (**it).draw( image );
      }
   }

   void PaneTextbox::_draw( Image& image )
   {
      // clear the textbox, we need to reprint everything
      Image::DirectionalIterator oy = image.getIterator( _origin[ 0 ], _origin[ 1 ], 0 );

      for ( ui32 yy = 0; yy < _size[ 1 ]; ++yy )
      {
         Image::DirectionalIterator lo = oy;
         for ( ui32 xx = 0; xx < _size[ 0 ]; ++xx )
         {
            lo.pickcol( 0 ) = _background[ 0 ];
            lo.pickcol( 1 ) = _background[ 1 ];
            lo.pickcol( 2 ) = _background[ 2 ];

            lo.addx();
         }
         oy.addy();
      }

      _drawText( image );

      // finally decorate the text...
      for ( unsigned n = 0; n < _decorators.size(); ++n )
      {
         (*_decorators[ n ]).draw( image );
      }
   }

   void PaneTextbox::_receive( const EventMouse& e )
   {
      for ( ui32 n = 0; n < _decorators.size(); ++n )
      {
         bool res = (*_decorators[ n ]).receive( e );
         if ( res )
            break;
      }
   }

   void PaneTextbox::_receive( const EventKeyboard& e )
   {
      for ( ui32 n = 0; n < _decorators.size(); ++n )
      {
         bool res = (*_decorators[ n ]).receive( e );
         if ( res )
            break;
      }
   }

   void LayoutPaneDecoratorCompletion::process()
   {
      if ( _current != -1 )
      {
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;
         ui32& _currentChar = position->currentChar;

         if ( _cutPoint < _src._text[ _currentLine ].text.size() )
         {
            _src._text[ _currentLine ].text[ _cutPoint ] = 0;
         }

         _src._text[ _currentLine ].text = std::string( _src._text[ _currentLine ].text.c_str() ) + _choices[ _current ].getName();
         _currentChar = _src._text[ _currentLine ].text.size();

         _selectionParent.erase( _selection );
         _selection = RefcountedTyped<Pane>();
         _selectionParent.notify();
         _current = -1;
      }
   }

   bool LayoutPaneDecoratorCompletion::receive( const EventKeyboard& e )
   {
      bool needsToRefresh = ( _selection.getDataPtr() && isChar( e.key ) );
      if ( needsToRefresh )
      {
         // we need to insert the letter typed firs!
         LayoutPaneDecoratorCursor* decorator = _src.get<LayoutPaneDecoratorCursor>();
         if ( decorator )
         {
            decorator->receive( e );
         }

         // refresh the screen
         _selectionParent.notify();
      }

      if ( _selection.getDataPtr() && !isChar( e.key ) && !( e.key == ' ' && e.isCtrl ) )
      {
         _selectionParent.erase( _selection );

         // refresh the screen
         _selectionParent.notify();

         return false;
      }

      if ( e.key == ' ' && e.isCtrl || needsToRefresh )
      {
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;

         std::set<mvv::Symbol> choices = _completion.findMatch( _src._text[ _currentLine ].text, _cutPoint );

         if ( _selection.getDataPtr() )
         {
            _selectionParent.erase( _selection );
            _selectionParent.notify();
         }

         if ( choices.size() )
         {
            _choices.clear();
            for ( std::set<mvv::Symbol>::iterator it = choices.begin(); it != choices.end(); ++it )
               _choices.push_back( *it );

            Pane::PaneRef ref( &_selectionParent, false );
            RefcountedTyped<Pane> widget( new WidgetSelectBox( ref, nll::core::vector2ui( 0, 0 ), 120, _choices, _current, _src._font ) );
            _selectionParent.insert( widget );
            _selection = widget;
         }

         return true;
      }

      return false;
   }
}
}