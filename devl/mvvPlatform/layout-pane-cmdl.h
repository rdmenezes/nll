#ifndef MVV_PLATFORM_LAYOUT_PANE_CMDL_H_
# define MVV_PLATFORM_LAYOUT_PANE_CMDL_H_

# include "layout-pane-textbox.h"
# include <mvvScript/compiler-dummy-interface.h>

namespace mvv
{
namespace platform
{
   class MVVPLATFORM_API LayoutCommandLine : public Pane 
   {
      class WritableCommand : public Writable
      {
      public:
         WritableCommand( parser::InterpreterRuntimeInterface& engine, Writable& sink ) : _engine( engine ), _sink( sink )
         {
         }

         virtual void write( const std::string& s, const nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 255 ) )
         {
            std::stringstream ss;
            std::ostream& os = _engine.getStdOut();

            try
            {
               _engine.setStdOut( &ss );

               bool result = _engine.interpret( s );
               if ( !result )
               {
                  std::string msg = _engine.getLastErrorMesage();
                  size_t pos = msg.find_first_of( 10 );
                  if ( pos != std::string::npos )
                  {
                     msg = std::string( msg.begin(), msg.begin() + pos );
                  }
                  _engine.clearError();
                  _sink.write( msg, nll::core::vector3uc( 255, 0, 0 ) );
               }
            } catch ( std::runtime_error e )
            {
               std::stringstream ss;
               ss << "runtime error:" << e.what();
               _sink.write( ss.str(), nll::core::vector3uc( 255, 0, 0 ) );
            }

            std::string sout = ss.str();
            if ( sout.size() )
            {
               std::vector<const char*> strings = nll::core::split( sout, '\n' );
               for ( ui32 n = 0; n < strings.size(); ++n )
                  if ( strings[ n ] != std::string( "" ) && strings[ n ] != std::string( "\n" ) )
                     _sink.write( strings[ n ], color / 2 );
            }

            _engine.setStdOut( &os );   // restore the previous state
            _sink.write( s, color );
         }

      private:
         // disabled
         WritableCommand& operator=( const WritableCommand& );
         WritableCommand( WritableCommand& );

      private:
         parser::InterpreterRuntimeInterface&  _engine;
         Writable&                  _sink;
      };

   public:
      LayoutCommandLine( const nll::core::vector2ui& origin,
                         const nll::core::vector2ui& size,
                         RefcountedTyped<Font> font,
                         parser::InterpreterRuntimeInterface& engine,
                         parser::CompletionInterface& completionInterface,
                         const ui32 fontSize = 15 ) : Pane( origin, size ), _engine( engine )
      {
         PaneTextbox* textBoxDisplay = new PaneTextbox( nll::core::vector2ui(0, 0),
                                                        nll::core::vector2ui(0, 0),
                                                        font, fontSize,
                                                        nll::core::vector3uc( 255, 255, 255 ),
                                                        nll::core::vector3uc( 30, 30, 30 ),
                                                        true );
         _textBoxDisplayP = textBoxDisplay;
         PaneTextbox* textBoxCmd = new PaneTextbox( nll::core::vector2ui(0, 0),
                                                    nll::core::vector2ui(0, 0),
                                                    font, fontSize,
                                                    nll::core::vector3uc( 255, 255, 255 ),
                                                    nll::core::vector3uc( 0, 0, 60 ) );
         _textBoxCmdP = textBoxCmd;
         RefcountedTyped<PaneTextboxDecorator> cursor( new LayoutPaneDecoratorCursor( *textBoxCmd ) );
         RefcountedTyped<PaneTextboxDecorator> cursorPos( new LayoutPaneDecoratorCursorPosition( *textBoxCmd ) );
         RefcountedTyped<PaneTextboxDecorator> cursorBasic( new LayoutPaneDecoratorCursorBasic( *textBoxCmd ) );
         RefcountedTyped<PaneTextboxDecorator> cursorYDirection( new LayoutPaneDecoratorCursorYDirection( *textBoxCmd ) );

         _writableCommand = RefcountedTyped<WritableCommand>( new WritableCommand( engine, *textBoxDisplay ) );
         _enter = new LayoutPaneDecoratorCursorEnterConsole( *textBoxCmd, *_writableCommand );
         RefcountedTyped<PaneTextboxDecorator> cursorEnter( _enter );

         RefcountedTyped<PaneTextboxDecorator> completion( new LayoutPaneDecoratorCompletion( *textBoxCmd, completionInterface ) );
         textBoxCmd->add( completion );
         textBoxCmd->add( cursorPos );
         textBoxCmd->add( cursorBasic );
         textBoxCmd->add( cursorEnter );
         textBoxCmd->add( cursorYDirection );
         textBoxCmd->add( cursor );



         _vpane = new PaneListVertical( nll::core::vector2ui(0, 0),
                                                          nll::core::vector2ui(0, 0) );
         _vpane->addChild( RefcountedTyped<Pane>( textBoxDisplay ), 0.97 );
         _vpane->addChild( RefcountedTyped<Pane>( textBoxCmd ), 0.03 );
         _subLayout = RefcountedTyped<Pane>( _vpane );
         (*_subLayout).setFather( this );

         _importPreviousHistory( *_enter );
      }

      virtual void _receive( const EventMouse& e )
      {
         (*_subLayout).receive( e );
      }

      virtual bool sendMessage( const std::string& msg, const nll::core::vector3uc& color )
      {
         _textBoxDisplayP->write( msg, color );
         return true;
      }

      virtual Pane* find( const mvv::Symbol& s )
      {
         if ( s == mvv::Symbol::create( "mvv::platform::LayoutCommandLine" ) )
            return this;
         return 0;
      }

      virtual void _receive( const EventKeyboard& e )
      {
         (*_subLayout).receive( e );
      }

      virtual void draw( Image& image )
      {
         (*_subLayout).draw( image );

         for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it )
         {
            (**it).draw( image );
         }
      }

      virtual void _draw( Image& )
      {
         // nothing to do
      }

      virtual void updateLayout()
      {
         // we want exaclty the size for 1 line of text!
         const ui32 csize = _textBoxDisplayP->getTextSize();
         if ( _size[ 1 ] > csize )  // if smaller, useless to compute...
         {
            const double ratio = ( csize + 1.0 ) / _size[ 1 ];
            _vpane->setRatio( 0, 1.0 - ratio );
            _vpane->setRatio( 1, ratio );
         }

         // we need to update the layout properties...
         Pane& pane = *_subLayout;
         pane.setOrigin( _origin );
         pane.setSize( _size );
         pane.updateLayout();
      }

      virtual void destroy()
      {
         // export the written commands
         int numberOfItems = _numberOfItems;
         std::string path = _exportHisoryPath;
         _readConfig( numberOfItems, path );

         std::ofstream file( path.c_str() );
         if ( !file.good() )
            return;
         const std::vector<std::string>& history = _enter->getHistory();
         for ( ui32 n = 0; n < history.size(); ++n )
         {
            file << history[ n ];
            if ( n + 1 < history.size() )
               file << std::endl;
         }
      }

      ~LayoutCommandLine()
      {
      }

      virtual void notify()
      {
         _textBoxDisplayP->notify();
         _textBoxCmdP->notify();
         _needToBeRefreshed = true;
      }

   private:
      LayoutCommandLine& operator=( LayoutCommandLine& );
      LayoutCommandLine( const LayoutCommandLine& );

   private:
      void _importPreviousHistory( LayoutPaneDecoratorCursorEnterConsole& inputConsole )
      {
         int numberOfItems = 500;
         std::string path = "c:/Temp/command_history.txt";

         _readConfig( numberOfItems, path );

         std::vector<std::string> history;
         std::ifstream file( path.c_str() );
         if ( !file.good() )
            return;
         while ( !file.eof() )
         {
            std::string line;
            getline( file, line );
            history.push_back( line );
         }
         inputConsole.setHistory( history );

         _numberOfItems = numberOfItems;
         _exportHisoryPath = path;
      }

      void _readConfig( int& numberOfItems, std::string& path )
      {
         numberOfItems = nll::core::str2val<int>( _engine.getVariableText( mvv::Symbol::create( "sizeHistoryExport" ) ) );
         path = _engine.getVariableText( mvv::Symbol::create( "historyExportLocation" ) );
      }


   protected:
      RefcountedTyped<Pane>      _textBoxCmd;
      RefcountedTyped<Pane>      _textBoxDisplay;
      RefcountedTyped<Pane>      _subLayout;
      parser::InterpreterRuntimeInterface&  _engine;

      RefcountedTyped<WritableCommand> _writableCommand;

      // shortcuts...
      PaneListVertical*                      _vpane;
      PaneTextbox*                           _textBoxDisplayP;
      PaneTextbox*                           _textBoxCmdP;
      LayoutPaneDecoratorCursorEnterConsole* _enter;

      // last history config
      ui32        _numberOfItems;
      std::string _exportHisoryPath;
   };
}
}
#endif