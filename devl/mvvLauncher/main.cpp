#include "init.h"
#include <GL/freeglut.h>

static mvv::ApplicationVariables* applicationVariables;

void handleOrders( int )
{
   glutTimerFunc( 1, handleOrders, 0 );

   // run orders & engines
   applicationVariables->engineHandler.run();
   applicationVariables->orderManager.run();

/*
   RefcountedTyped<Segment> segment1;
   applicationVariables->context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment1"), segment1 );
   (*segment1).position.notify();

   RefcountedTyped<Segment> segment2;
   bool found = applicationVariables->context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment2"), segment2 );
   if ( found )
      (*segment2).position.notify();
*/
   /*
   static int nbFps = 0;
   static unsigned last = clock();
   ++nbFps;

   if ( ( clock() - last ) / (double)CLOCKS_PER_SEC >= 1 )
   {
      std::cout << "----------------------------fps=" << nbFps << std::endl;
      nbFps = 0;
      last = clock();
   }*/
}

void renderObjects()
{
   // we need to flush all the engines for rendering: if there are some actions in the pipeline,
   // the rendering may flicker...
   applicationVariables->engineHandler.run();

   // generate the texture we are going to draw
   (*applicationVariables->layout).draw( applicationVariables->screen );

   //nll::core::writeBmp( applicationVariables.screen, "c:/screen.bmp" );
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
      applicationVariables->mouseEvent.isMouseLeftButtonJustReleased = true;
      applicationVariables->mouseEvent.isMouseLeftButtonJustPressed = false;
   } else {
      applicationVariables->mouseEvent.isMouseLeftButtonJustReleased = false;
   }

   // right
   if ( applicationVariables->mouseEvent.isMouseRightButtonJustPressed )
   {
      applicationVariables->mouseEvent.isMouseRightButtonJustReleased = true;
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

void keyboard(unsigned char key, int x, int y)
{
   if ( key == 'n' )
   {
      RefcountedTyped<Segment> segment1;
      applicationVariables->context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment1"), segment1 );
      std::cout << "check main pointer=" << (*segment1).position.getDataPtr() << std::endl;
      (*applicationVariables->segmentToolCamera).setPosition( nll::core::vector3f( -0.1, 1, 1 ) );
      /*
      RefcountedTyped<Segment> segment1;
      applicationVariables->context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment1"), segment1 );
      (*segment1).position.notify();

      RefcountedTyped<Segment> segment2;
      bool found = applicationVariables->context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment2"), segment2 );
      if ( found )
         (*segment2).position.notify();
         */
   }
   if ( key == 'm' )
   {
      applicationVariables->orderManager.notify();
   }
   if ( key == 'q' )
   {
      applicationVariables->layout.unref();
      applicationVariables->orderManager.kill();
      applicationVariables->context.clear();
      exit( 0 );
   }
}

int main(int argc, char** argv)
{
  applicationVariables = new mvv::ApplicationVariables();

  // GLUT Window Initialization:
  glutInit( &argc, argv );
  glutInitWindowSize( (*applicationVariables->layout).getSize()[ 0 ], (*applicationVariables->layout).getSize()[ 1 ] );
  glutInitDisplayMode( GLUT_RGB /*| GLUT_DOUBLE | GLUT_DEPTH*/ );
  glutCreateWindow( "Medical Volume Viewer" );
  //glutGameModeString( "1024x1024:32" );
  //glutEnterGameMode();

  // Initialize OpenGL graphics state
  initGraphics();

  // Register callbacks:
  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  glutKeyboardFunc( keyboard );
  glutMouseFunc( mouseButton );
  glutMotionFunc( mouseMotion );
  glutPassiveMotionFunc( mouseMotion );
  glutTimerFunc( 0, handleOrders, 0 );

  // Turn the flow of control over to GLUT
  glutMainLoop ();
  return 0;
}
 