#include "init.h"

#undef FLOAT
#undef INT

#include <GL/freeglut.h>
#include <mvvForms/mvvForms.h>

#undef FLOAT
#undef INT

static mvv::ApplicationVariables* applicationVariables;

int toLower( int c )
{
   if ( c >= 'A' && c <= 'Z' )
   {
      return 'a' + c - 'A';
   }
   return c;
}

int noCtrl( int c )
{
   return 'a' + c - 1;
}

void handleOrders( int )
{
   glutTimerFunc( 0, handleOrders, 0 );

   // check the layout has not been changed...
   const RuntimeValue& layoutRef = applicationVariables->compiler.getVariable( mvv::Symbol::create( "layout" ) );
   RuntimeValues* currentLayout = const_cast<RuntimeValues*>( layoutRef.vals.getDataPtr() );
   if ( currentLayout != applicationVariables->oldLayout )
   {
      // we must update the current layout
      FunctionLayoutConstructorSegment::Pointee* pointee = reinterpret_cast<FunctionLayoutConstructorSegment::Pointee*>( (*layoutRef.vals)[ 0 ].ref );
      applicationVariables->layout = pointee->pane;
      applicationVariables->oldLayout = const_cast<RuntimeValues*>( layoutRef.vals.getDataPtr() );
      (*applicationVariables->layout).setSize( nll::core::vector2ui( applicationVariables->screen.sizex(), applicationVariables->screen.sizey() ) );
      (*applicationVariables->layout).updateLayout();
   }

   // check the callbacks
   applicationVariables->callbacks.run();

   // run orders & engines
   // note that we need to flush all the engines for rendering: if there are some actions in the pipeline,
   // the rendering may flicker or even performance drop (it is rendering intermediate state whereas it should
   // have been done in one pass)...
   applicationVariables->engineHandler.run();
   applicationVariables->orderManager.run();
   applicationVariables->engineHandler.run();
   applicationVariables->orderManager.run();
   applicationVariables->engineHandler.run();
   applicationVariables->orderManager.run();
   applicationVariables->engineHandler.run();
   applicationVariables->orderManager.run();
   applicationVariables->engineHandler.run();
   applicationVariables->orderManager.run();
   applicationVariables->engineHandler.run();
   applicationVariables->orderManager.run();
}

void renderObjects()
{
   // generate the texture we are going to draw
   (*applicationVariables->layout).draw( applicationVariables->screen );

   //nll::core::writeBmp( applicationVariables->screen, "c:/screen.bmp" );
   glBindTexture( GL_TEXTURE_2D, applicationVariables->screenTextureId );
   glTexImage2D(GL_TEXTURE_2D, 0, 3, applicationVariables->screen.sizex(), applicationVariables->screen.sizey(),
                0, GL_RGB, GL_UNSIGNED_BYTE, applicationVariables->screen.getBuf() );

   // draw the screen
   glEnable( GL_TEXTURE_2D );
   glBindTexture( GL_TEXTURE_2D, applicationVariables->screenTextureId );

   glBegin(GL_QUADS);
   glTexCoord2d( 0, 0 );
   glVertex2f( 0, 0 );
   glTexCoord2d( 1, 0 );
   glVertex2f( (GLfloat)(*applicationVariables->layout).getSize()[ 0 ], 0 );
   glTexCoord2d( 1, 1 );
   glVertex2f( (GLfloat)(*applicationVariables->layout).getSize()[ 0 ], (GLfloat)(*applicationVariables->layout).getSize()[ 1 ] );
   glTexCoord2d( 0, 1 );
	glVertex2f( 0, (GLfloat)(*applicationVariables->layout).getSize()[ 1 ] );
	glEnd();
}



void display()
{
   // Clear frame buffer and depth buffer
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   // Set up viewing transformation, looking down -Z axis
   glLoadIdentity();

   // Render the scene
   renderObjects();

   // Make sure changes appear onscreen
   glutSwapBuffers();
   glutPostRedisplay();
}

void reshape( GLint w, GLint h )
{
   glViewport(0,0,(GLsizei) w, (GLsizei) h);
   
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluOrtho2D (0, w, 0, h);
   
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   applicationVariables->screen = nll::core::Image<nll::ui8>( w, h, 3 );
   (*applicationVariables->layout).setSize( nll::core::vector2ui( w, h ) );
   (*applicationVariables->layout).updateLayout();
}

void initGraphics()
{
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LESS );
   glShadeModel( GL_SMOOTH );

   // configure the display texture
   glGenTextures( 1, &applicationVariables->screenTextureId );
   glBindTexture( GL_TEXTURE_2D, applicationVariables->screenTextureId );
   glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST );
}
void mouseButton(int button, int state, int x, int y)
{
   // the (0, 0) is top-left instead of bottom left, so just align the mouse coordinate with screen coordinate
   y = applicationVariables->screen.sizey() - y - 1;

   //std::cout << "event-main" << std::endl;
   applicationVariables->mouseEvent.mousePosition = nll::core::vector2ui( x, y );
   applicationVariables->mouseEvent.isMouseRightButtonJustReleased = false;
   applicationVariables->mouseEvent.isMouseLeftButtonJustReleased = false;
   applicationVariables->mouseEvent.isMouseRightButtonJustPressed = false;
   applicationVariables->mouseEvent.isMouseLeftButtonJustPressed = false;


   if ( button == GLUT_LEFT_BUTTON )
   {
      if ( state == GLUT_UP )
      {
         applicationVariables->mouseEvent.isMouseLeftButtonJustReleased =true;
         if ( applicationVariables->mouseEvent.isMouseLeftButtonPressed )
         {
            //std::cout << "LEFT DOWN" << std::endl;
            applicationVariables->mouseEvent.mouseLeftReleasedPosition = nll::core::vector2ui( x, y );
         }

         // button unreleased
         applicationVariables->mouseEvent.isMouseLeftButtonPressed = false;
      } 
      if ( state == GLUT_DOWN )
      {
         //std::cout << "LEFT PRESSED" << std::endl;
         applicationVariables->mouseEvent.isMouseLeftButtonJustPressed =true;
         if (!applicationVariables->mouseEvent.isMouseLeftButtonPressed )
         {
            // the first click position
            applicationVariables->mouseEvent.mouseLeftClickedPosition = nll::core::vector2ui( x, y );
         }
         applicationVariables->mouseEvent.isMouseLeftButtonPressed = true;
      }
   }

   if ( button == GLUT_RIGHT_BUTTON )
   {
      if ( state == GLUT_UP )
      {
         applicationVariables->mouseEvent.isMouseRightButtonJustReleased =true;
         if ( applicationVariables->mouseEvent.isMouseRightButtonPressed )
         {
            //std::cout << "Right DOWN" << std::endl;
            applicationVariables->mouseEvent.mouseRightReleasedPosition = nll::core::vector2ui( x, y );
         }

         // button unreleased
         applicationVariables->mouseEvent.isMouseRightButtonPressed = false;
      } 
      if ( state == GLUT_DOWN )
      {
         //std::cout << "Right PRESSED" << std::endl;
         applicationVariables->mouseEvent.isMouseRightButtonJustPressed =true;
         if (!applicationVariables->mouseEvent.isMouseRightButtonPressed )
         {
            // the first click position
            applicationVariables->mouseEvent.mouseRightClickedPosition = nll::core::vector2ui( x, y );
         }
         applicationVariables->mouseEvent.isMouseRightButtonPressed = true;
      }
   }
   (*applicationVariables->layout).receive( applicationVariables->mouseEvent );
}

void mouseMotion(int x, int y)
{
   // the (0, 0) is top-left instead of bottom left, so just align the mouse coordinate with screen coordinate
   y = applicationVariables->screen.sizey() - y - 1;

   // left
   if ( applicationVariables->mouseEvent.isMouseLeftButtonJustPressed )
   {
      //applicationVariables->mouseEvent.isMouseLeftButtonJustReleased = true;
      applicationVariables->mouseEvent.isMouseLeftButtonJustPressed = false;
   } else {
      applicationVariables->mouseEvent.isMouseLeftButtonJustReleased = false;
   }

   // right
   if ( applicationVariables->mouseEvent.isMouseRightButtonJustPressed )
   {
      //applicationVariables->mouseEvent.isMouseRightButtonJustReleased = true;
      applicationVariables->mouseEvent.isMouseRightButtonJustPressed = false;
   } else {
      applicationVariables->mouseEvent.isMouseRightButtonJustReleased = false;
   }
   applicationVariables->mouseEvent.mousePosition = nll::core::vector2ui( x, y );

   //(*applicationVariables.layout).receive( applicationVariables.mouseEvent ); // TODO put back!

 //  RefcountedTyped<Segment> segment1;
 //  applicationVariables->context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment1"), segment1 );
 //  (*segment1).position.setValue( 2, (float)x/10 );

 //  RefcountedTyped<Segment> segment2;
 //  bool found = applicationVariables->context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment2"), segment2 );
 //  if ( found )
 //  {
 //     (*segment2).position.setValue( 2, (float)x/10 );
 //  }
   (*applicationVariables->layout).receive( applicationVariables->mouseEvent );
}

void keyboard( unsigned char key, int x, int y )
{
   EventKeyboard e;
   e.mousePosition = nll::core::vector2ui( x, y );
   e.key = key;
   int mod = glutGetModifiers();
   if ( mod & GLUT_ACTIVE_ALT )
      e.isAlt = true;
   else
      e.isAlt = false;
   if ( mod & GLUT_ACTIVE_CTRL )
      e.isCtrl = true;
   else
      e.isAlt = false;
   if ( mod & GLUT_ACTIVE_SHIFT )
      e.isShift = true;
   else
      e.isShift = false;

   // handle callbacks
   Callbacks::Key::Modifier modifier = Callbacks::Key::NORMAL;
   if ( e.isShift )
      modifier = Callbacks::Key::SHIFT;
   if ( e.isAlt )
      modifier = Callbacks::Key::ALT;
   if ( e.isCtrl )
   {
      key = noCtrl( key );
      modifier = Callbacks::Key::CTRL;
   }
   applicationVariables->callbacks.handleKey( toLower( key ), modifier );  // we lower the case as it should be handled by the modifier instead

   (*applicationVariables->layout).receive( e );

   // handle 'esc' at the end so we have the opportunity to grab this key (i.e. save the sate) before quitting
   if ( key == 27 )
   {
      (*applicationVariables->layout).destroy();
      applicationVariables->orderManager.kill();
      applicationVariables->context.clear();
      exit( 0 );
   }
}

void keyboardSpecial( int key, int x, int y )
{
   applicationVariables->callbacks.handleKey( key, Callbacks::Key::NORMAL );

   EventKeyboard e;
   e.mousePosition = nll::core::vector2ui( x, y );

   switch ( key )
   {
   case GLUT_KEY_LEFT:
      e.key = EventKeyboard::KEY_LEFT; break;
   case GLUT_KEY_UP:
      e.key = EventKeyboard::KEY_UP; break;
   case GLUT_KEY_RIGHT:
      e.key = EventKeyboard::KEY_RIGHT; break;
   case GLUT_KEY_DOWN:
      e.key = EventKeyboard::KEY_DOWN; break;
   case GLUT_KEY_HOME:
      e.key = EventKeyboard::KEY_HOME; break;
   case GLUT_KEY_END:
      e.key = EventKeyboard::KEY_END; break;
   default:
      return;
   }

   (*applicationVariables->layout).receive( e );
}


bool checkOptionArgv( int argc, char** argv, const std::string& val, const char delim, std::vector<std::string>& params )
{
   for ( int n = 1; n < argc; ++n )
   {
      std::string str = argv[ n ];
      std::vector<const char*> splits = nll::core::split( str, '=' );
      if ( splits.size() == 2 && strcmp( splits[ 0 ], val.c_str() ) == 0 )
      {
         std::vector<std::string> opts;
         std::string str2 = splits[ 1 ];
         std::vector<const char*> vals = nll::core::split( str2, delim );
         for ( size_t m = 0; m < vals.size(); ++m )
         {
            opts.push_back( vals[ m ] );
         }
         params = opts;
         return true;
      }
   }
   return false;
}

bool checkOptionVal( int argc, char** argv, const std::string& opt, std::string& val )
{
   for ( int n = 1; n < argc; ++n )
   {
      std::string str = argv[ n ];
      std::vector<const char*> splits = nll::core::split( str, '=' );
      if ( splits.size() == 2 && strcmp( splits[ 0 ], opt.c_str() ) == 0 )
      {
         std::string str2 = splits[ 1 ];
         std::vector<const char*> vals = nll::core::split( str2, ' ' );
         if ( vals.size() != 1 )
         {
            std::cerr << "error: expected commandline -sizex=XXX" << std::endl;
            exit( 1 );
         }
         val = vals[ 0 ];
         return true;
      }
   }
   return false;
}

//
// arguments:
// sizex sizey nbThreads initialscript importpath:path1;path2;path3 font -nowindow
//
int main(int argc, char** argv)
{
   try
   { 
      int sizex = 1280;
      int sizey = 1024;

      int nbThreads = 12; //8;
      std::string mainScript = "../../mvvLauncher/script/single";
      std::string font = "../../nllTest/data/font/bitmapfont1_24";
      bool nowindow = false;
      std::vector<std::string> argvParam;
      std::vector<std::string> importPaths;

      std::string val;
      if ( checkOptionVal( argc, argv, "-sizex", val ) )
      {
         sizex = atoi( val.c_str() );
      }
      if ( checkOptionVal( argc, argv, "-sizey", val ) )
      {
         sizey = atoi( val.c_str() );
      }
      if ( checkOptionVal( argc, argv, "-threads", val ) )
      {
         nbThreads = atoi( val.c_str() );
      }
      if ( checkOptionVal( argc, argv, "-input", val ) )
      {
         mainScript = val;
      }
      if ( checkOptionVal( argc, argv, "-font", val ) )
      {
         font = val;
      }
      checkOptionArgv( argc, argv, "-argv", ' ', argvParam );
      checkOptionArgv( argc, argv, "-import", ';', importPaths );
      if ( checkOptionVal( argc, argv, "-nowindow", val ) )
      {
         nowindow = true;
      }
      mainScript = "include \"" + mainScript + "\""; // we will include the script to import it!

      // init
      std::cout << "Viewer is attached=" << !nowindow << std::endl;
      applicationVariables = new mvv::ApplicationVariables( sizex, sizey, nbThreads, mainScript, importPaths, font, argvParam, nowindow );

      if ( !nowindow )
      {
         // GLUT Window Initialization:
         glutInit( &argc, argv );
         glutInitWindowSize( (*applicationVariables->layout).getSize()[ 0 ], (*applicationVariables->layout).getSize()[ 1 ] );
         glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE /*| GLUT_DEPTH*/ );
         glutCreateWindow( "Medical Volume Viewer" );
         //glutGameModeString( "1280x1024" );
         //glutEnterGameMode();

         // Initialize OpenGL graphics state
         initGraphics();

         // Register callbacks:
         glutDisplayFunc( display );
         glutReshapeFunc( reshape );
         glutKeyboardFunc( keyboard );
         glutSpecialFunc( keyboardSpecial );
         glutMouseFunc( mouseButton );
         glutMotionFunc( mouseMotion );
         glutPassiveMotionFunc( mouseMotion );
         glutTimerFunc( 0, handleOrders, 0 );

         // Turn the flow of control over to GLUT
         glutMainLoop ();
      } else {
         // as long as orders have not been runned and results dispatched we need to keep the interpreter alive
         applicationVariables->engineHandler.run();
         applicationVariables->orderManager.run();

         ui32 nbOrders = applicationVariables->orderManager.getNumberOfOrdersToRun();
         while ( nbOrders )
         {
            // run orders & engines
            applicationVariables->engineHandler.run();
            applicationVariables->orderManager.run();
            nbOrders = applicationVariables->orderManager.getNumberOfOrdersToRun();
         }
      }
   }

   catch ( std::runtime_error& e )
   {
      std::cerr << "Fatal error: " << e.what() << std::endl;
      exit(1);
   }

   catch (...)
   {
      std::cerr << "unexpexted exception thrown, exiting... " << std::endl;
      exit(1);
   }

   return 0;
}
