#ifndef MVV_PLATFORM_LAYOUT_PANE_CMDL_H_
# define MVV_PLATFORM_LAYOUT_PANE_CMDL_H_

# include "layout-pane-textbox.h"
# include <mvvScript/compiler.h>

namespace mvv
{
namespace platform
{
   class MVVPLATFORM_API LayoutCommandLine : public Pane 
   {
   public:
      LayoutCommandLine( const nll::core::vector2ui& origin,
                         const nll::core::vector2ui& size,
                         RefcountedTyped<Font>& font,
                         parser::CompilerFrontEnd& engine,
                         const ui32 fontSize = 17 ) : Pane( origin, size ), _engine( engine )
      {
         PaneTextbox* textBoxDisplay = new PaneTextbox( nll::core::vector2ui(0, 0),
                                                        nll::core::vector2ui(0, 0),
                                                        font, fontSize,
                                                        nll::core::vector3uc( 255, 255, 255 ),
                                                        nll::core::vector3uc( 30, 30, 30 ),
                                                        true );
         PaneTextbox* textBoxCmd = new PaneTextbox( nll::core::vector2ui(0, 0),
                                                    nll::core::vector2ui(0, 0),
                                                    font, fontSize,
                                                    nll::core::vector3uc( 255, 255, 255 ),
                                                    nll::core::vector3uc( 0, 0, 0 ) );
         RefcountedTyped<PaneTextboxDecorator> cursor( new LayoutPaneDecoratorCursor( *textBoxCmd ) );
         RefcountedTyped<PaneTextboxDecorator> cursorPos( new LayoutPaneDecoratorCursorPosition( *textBoxCmd ) );
         RefcountedTyped<PaneTextboxDecorator> cursorBasic( new LayoutPaneDecoratorCursorBasic( *textBoxCmd ) );
         RefcountedTyped<PaneTextboxDecorator> cursorYDirection( new LayoutPaneDecoratorCursorYDirection( *textBoxCmd ) );
         RefcountedTyped<PaneTextboxDecorator> cursorEnter( new LayoutPaneDecoratorCursorEnterConsole( *textBoxCmd, *textBoxDisplay ) );
         textBoxCmd->add( cursorPos );
         textBoxCmd->add( cursorBasic );
         textBoxCmd->add( cursorYDirection );
         textBoxCmd->add( cursorEnter );
         textBoxCmd->add( cursor );

         PaneListVertical* list = new PaneListVertical( nll::core::vector2ui(0, 0),
                                                        nll::core::vector2ui(0, 0) );
         list->addChild( RefcountedTyped<Pane>( textBoxDisplay ), 0.97 );
         list->addChild( RefcountedTyped<Pane>( textBoxCmd ), 0.03 );
         _subLayout = RefcountedTyped<Pane>( list );
      }

      virtual void _receive( const EventMouse& e )
      {
         (*_subLayout).receive( e );
      }

      virtual void _receive( const EventKeyboard& e )
      {
         (*_subLayout).receive( e );
      }

      virtual void _draw( Image& image )
      {
         (*_subLayout)._draw( image );
      }

      virtual void updateLayout()
      {
         Pane* pane = &(*_subLayout);
         pane->setOrigin( _origin );
         pane->setSize( _size );
         pane->updateLayout();
      }

   protected:
      RefcountedTyped<Pane>      _textBoxCmd;
      RefcountedTyped<Pane>      _textBoxDisplay;
      RefcountedTyped<Pane>      _subLayout;
      parser::CompilerFrontEnd&          _engine;
   };
}
}
#endif