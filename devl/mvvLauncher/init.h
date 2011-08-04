#ifndef MVV_INIT_H_
# define MVV_INIT_H_

# include <mvvPlatform/types.h>
# include <mvvPlatform/event-mouse.h>
# include <mvvPlatform/layout-pane.h>
# include <mvvPlatform/layout-widget.h>
# include <mvvPlatform/context.h>
# include <mvvPlatform/context-volumes.h>
# include <mvvPlatform/context-tools.h>
# include <mvvPlatform/context-global.h>
# include <mvvPlatform/engine-handler-impl.h>
# include <mvvPlatform/layout-pane-textbox.h>
# include <mvvPlatform/layout-pane-cmdl.h>
# include <mvvPlatform/layout-widget-selectbox.h>
# include <mvvMprPlugin/context-segments.h>
# include <mvvMprPlugin/layout-segment.h>
# include <mvvMprPlugin/segment-tool-pointer.h>
# include <mvvMprPlugin/segment-tool-camera.h>
# include <mvvMprPlugin/segment-tool-annotations.h>
# include <mvvMprPlugin/segment-tool-autocenter.h>
# include <mvvMprPlugin/layout-mip.h>
# include <mvvMprPlugin/annotation-point.h>
# include <mvvMprPlugin/mip-tool-pointer.h>
# include <mvvScript/completion.h>
# include <mvvScript/compiler.h>
# include <core/mvv-layout.h>
# include "callbacks.h"
#include <GL/freeglut.h>

using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

/**
 -BUG: script VolumeContainer v1 -> create segments, VolumeContainer v2 -> v1 = v2 -> should replace all the volumes, we must define a kind of operator=
 */
namespace mvv
{
   class MousePointerImpl : public platform::MousePointer
   {
   public:
      MousePointerImpl()
      {
         _mousePointer = MousePointer::MOUSE_NORMAL;
      }

      virtual void setMousePointer( MouseGraphic graphic )
      {
         int g = (int)graphic;
         glutSetCursor( g );
      }

      virtual ui32 getMousePointer() const
      {
         return _mousePointer;
      }

   private:
      ui32     _mousePointer;
   };

   struct ApplicationVariables
   {
      unsigned int                        screenTextureId;
      Image                               screen;
      EventMouse                          mouseEvent;

      Context                             context;
      EngineHandlerImpl                   engineHandler;
      OrderManagerThreadPool              orderManager;

      RefcountedTyped<Font>               font;
      RefcountedTyped<Pane>               layout;

      CompilerFrontEnd                    compiler;
      RuntimeValues*                      oldLayout;  // in case a script if modifying the root of the layout, we must be able to detect it by comparing the pointers...
      Callbacks                           callbacks;

      ApplicationVariables( ui32 sx, ui32 sy, ui32 nbThreads, const std::string& mainScript, const std::vector<std::string>& importPath, const std::string& font, const std::vector<std::string>& argv ) : screen( sx, sy, 3 ), orderManager( nbThreads ), callbacks( compiler )
      {  
         for ( ui32 n = 0; n < importPath.size(); ++n )
         {
            compiler.addImportDirectory( importPath[ n ] );
         }

         initFont( font );
         initContext();

         // handle argv
         if ( argv.size() != 0 )
         {
            std::stringstream ss;
            ss << "int argc=" << argv.size() << "; ";
            ss << "string argv[]={";
            for ( size_t n = 0; n < argv.size(); ++n )
            {
               ss << "\"" << argv[ n ] << "\"";
               if ( n + 1 != argv.size() )
                  ss << ", ";
            }
            ss << " };";
            Error::ErrorType result = compiler.run( ss.str() );
            ensure( result == Error::SUCCESS, "Problem in creating the command line arguments" );
         }
         initScript( mainScript );

         // event
         mouseEvent.isMouseRightButtonJustReleased = false;
         mouseEvent.isMouseLeftButtonJustReleased = false;
         mouseEvent.isMouseRightButtonJustPressed = false;
         mouseEvent.isMouseLeftButtonJustPressed = false;
         mouseEvent.isMouseRightButtonPressed = false;
         mouseEvent.isMouseLeftButtonPressed = false;
      }

   private:
      
      void initScript( const std::string& mainScript )
      {
         // set the extension, so we can access the context
         compiler.setContextExtension( RefcountedTyped<Context>( &context, false ) );

         // "include \"../../mvvLauncher/script/single\""
         std::cout << "importing script:" << mainScript << std::endl;
         nll::core::Timer time;
         try
         {
            Error::ErrorType state = compiler.run( mainScript );
            std::cout << "initial script runtime=" << time.getCurrentTime() << std::endl;
            if ( state != Error::SUCCESS )
            {
               std::cerr << "script failure:" << compiler.getLastErrorMesage() << std::endl;
               exit(1);
            }
         }

         catch ( std::runtime_error e )
         {
            std::cerr << "script failure, exception:" << e.what() << std::endl;
            exit(1);
         } catch ( ... )
         {
            std::cerr << "script failure, exception thrown" << std::endl;
            exit(1);
         }
         std::cout << "script parsed!" << std::endl;

         // the only thing we absolutely need to find is a handle on the "layout", which MUST be defined in the script
         // the reference must be kept alive until the end of the program
         try
         {
            const RuntimeValue& layoutRef = compiler.getVariable( mvv::Symbol::create( "layout" ) );
            FunctionLayoutConstructorSegment::Pointee* pointee = reinterpret_cast<FunctionLayoutConstructorSegment::Pointee*>( (*layoutRef.vals)[ 0 ].ref );
            layout = pointee->pane;
            oldLayout = const_cast<RuntimeValues*>( layoutRef.vals.getDataPtr() );
         } catch ( std::exception& e )
         {
            std::cerr << "warning:" << e.what() << " This may be a problem if the application is with a viewer attached..." << std::endl;
            return;
         }

         // update the layout
         platform::ContextGlobal* global = context.get<platform::ContextGlobal>();
         if ( !global )
            throw std::runtime_error( "global context uncorrectly initialized" );

         /*
         //
         // replace
         //
         layout = RefcountedTyped<Pane>( new PaneEmpty( nll::core::vector2ui( 0, 0 ), nll::core::vector2ui( 0, 0 ), nll::core::vector3uc( 0, 0, 0 ) ) );
         std::vector<mvv::Symbol> choices = nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "test1" ), mvv::Symbol::create( "test2" ), mvv::Symbol::create( "test3" ) );
         int index;
         RefcountedTyped<Pane> widget( new WidgetSelectBox( layout, nll::core::vector2ui( 0, 0 ), 80, choices, index, font ) );
         (*layout).insert( widget );
         //
         //
         //
         */

         global->layout = layout;

         (*layout).setSize( nll::core::vector2ui( screen.sizex(), screen.sizey() ) );
         (*layout).updateLayout();
      }

      void initFont( const std::string& fontpath )
      {
         const std::string fontBmp = fontpath + ".bmp";
         const std::string fontTxt = fontpath + ".txt";
         std::cout << "loading font:" << fontTxt << std::endl;
         std::vector<char> chars;
         std::ifstream i( ( fontTxt ).c_str() );
         if ( !i.good() )
         {
            std::cout << "can't load font:" << fontTxt << std::endl;
            exit( 1 );
         }

         std::string line;
         while ( !i.eof() )
         {
            std::getline( i, line );
            ensure( line.size() < 2, "expected 1 character by line" );
            if ( line.size() == 1 )
            {
               chars.push_back( line[ 0 ] );
            } else chars.push_back( static_cast<char>( 0 ) );
         }
         ensure( chars.size() == 256, "we are expecting 256 chars, currently=" + nll::core::val2str( chars.size() ) );
         font = RefcountedTyped<Font>( new FontBitmapMatrix( fontBmp, nll::core::vector2ui( 16, 16 ), nll::core::vector2ui( 16, 16 ), chars, true, nll::core::vector3uc( 0, 0, 0 ), nll::core::vector2ui( 0, 0), nll::core::vector2ui( 0, 0), false ) );
      }

      void initContext()
      {
         ContextVolumes* ctxVolumes = new ContextVolumes();
         context.add( ctxVolumes );

         ContextTools* ctxTools = new ContextTools( ctxVolumes->volumes, engineHandler, orderManager );
         context.add( ctxTools );

         ContextSegments* ctxSegments = new ContextSegments();
         context.add( ctxSegments );

         ContextGlobal* ctxGlobal = new ContextGlobal( engineHandler, orderManager, *font, screen );
         ctxGlobal->completion = RefcountedTyped<parser::CompletionInterface>( new parser::Completion( compiler ) );
         ctxGlobal->mousePointer = RefcountedTyped<platform::MousePointer>( new MousePointerImpl() );

         context.add( ctxGlobal );
      }
   };
}

#endif