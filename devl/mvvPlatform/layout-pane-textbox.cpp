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
         Pane* root = _src.getRoot();

         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;
         ui32& _currentChar = position->currentChar;

         // if we display the prototype, we actually don't want to replace the text...
         if ( !_isGuiDisplayingPrototype )
         {
            if ( _cutPoint < _src._text[ _currentLine ].text.size() )
            {
               _src._text[ _currentLine ].text[ _cutPoint ] = 0;
            }

            _src._text[ _currentLine ].text = std::string( _src._text[ _currentLine ].text.c_str() ) + _choices[ _current ];
            _currentChar = (ui32)_src._text[ _currentLine ].text.size();
         }

         root->erase( _selection );
         _selection = RefcountedTyped<Pane>();
         root->notify();
         _current = -1;
      }
   }

   bool LayoutPaneDecoratorCompletion::receive( const EventKeyboard& e )
   {
      Pane* root = _src.getRoot();

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
         root->notify();
      }

      if ( e.key == '\t' )
      {
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;

         std::vector<std::string> prototypes;
         _completion.getType( _src._text[ _currentLine ].text, _cutPoint, prototypes );
         _isGuiDisplayingPrototype = true;

         if ( _selection.getDataPtr() )
         {
            root->erase( _selection );
            root->notify();
         }

         if ( prototypes.size() )
         {
            _choices = prototypes;
            ui32 nbChar = 0;
            for ( ui32 n = 0; n < _choices.size(); ++n )
            {
               if ( _choices[ n ].size() > nbChar )
                  nbChar = (ui32)_choices[ n ].size();
            }

            nll::core::vector2ui origin( _src._origin[ 0 ], _src._origin[ 1 ] + _src._size[ 1 ] );
            RefcountedTyped<Pane> widget( new WidgetSelectBox( true, origin, std::min<ui32>( 500, nbChar * _fontSize * 0.5 ), _choices, _current, _src._font, _fontSize, nll::core::vector3uc( 0, 127, 0 ) ) );
            root->insert( widget );
            _selection = widget;
         }

         return true;
      }

      if ( _selection.getDataPtr() && !isChar( e.key ) && !( e.key == ' ' && e.isCtrl ) )
      {
         root->erase( _selection );

         // refresh the screen
         root->notify();

         return false;
      }

      if ( e.key == ' ' && e.isCtrl || needsToRefresh )
      {
         LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
         if ( !position )
            throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
         ui32& _currentLine = position->currentLine;

         std::vector<std::string> prototypes;
         std::set<mvv::Symbol> choices = _completion.findMatch( _src._text[ _currentLine ].text, _cutPoint, prototypes );
         if ( choices.size() == 1 )
         {
            LayoutPaneDecoratorCursorPosition* position = _src.get<LayoutPaneDecoratorCursorPosition>();
            if ( !position )
               throw std::exception( "LayoutPaneDecoratorCursor needs a LayoutPaneDecoratorCursorPosition  for a textbox decorator" );
            ui32& _currentLine = position->currentLine;
            ui32& _currentChar = position->currentChar;

            // only one choice, so directly replace it...
            if ( _cutPoint < _src._text[ _currentLine ].text.size() )
            {
               _src._text[ _currentLine ].text[ _cutPoint ] = 0;
            }

            _src._text[ _currentLine ].text = std::string( _src._text[ _currentLine ].text.c_str() ) + choices.begin()->getName();
            _currentChar = (ui32)_src._text[ _currentLine ].text.size();

            _isGuiDisplayingPrototype = true;
         } else {
            _isGuiDisplayingPrototype = false;
         }

         if ( _selection.getDataPtr() )
         {
            root->erase( _selection );
            root->notify();
         }

         if ( choices.size() > 1 )
         {
            _choices.clear();
            for ( std::set<mvv::Symbol>::iterator it = choices.begin(); it != choices.end(); ++it )
               _choices.push_back( (*it).getName() );

            nll::core::vector2ui origin( _src._origin[ 0 ], _src._origin[ 1 ] + _src._size[ 1 ] );
            RefcountedTyped<Pane> widget( new WidgetSelectBox( true, origin, 120, _choices, _current, _src._font ) );
            root->insert( widget );
            _selection = widget;
         }

         if ( choices.size() == 1 )
         {
            _choices = prototypes;
            ui32 nbChar = 0;
            for ( ui32 n = 0; n < _choices.size(); ++n )
            {
               if ( _choices[ n ].size() > nbChar )
                  nbChar = (ui32)_choices[ n ].size();
            }

            nll::core::vector2ui origin( _src._origin[ 0 ], _src._origin[ 1 ] + _src._size[ 1 ] );
            RefcountedTyped<Pane> widget( new WidgetSelectBox( true, origin, std::min<ui32>( 500, nbChar * _fontSize * 0.5 ), _choices, _current, _src._font, _fontSize, nll::core::vector3uc( 0, 127, 0 ) ) );
            root->insert( widget );
            _selection = widget;
         }

         return true;
      }

      return false;
   }
}
}