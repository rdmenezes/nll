#include "init.h"
#include <GL/freeglut.h>

static mvv::ApplicationVariables applicationVariables;

void handleOrders( int value )
{
   glutTimerFunc( 0, handleOrders, 0 );

}

void renderObjects()
{
   static int nbFps = 0;
   static unsigned last = clock();
   ++nbFps;

   if ( ( clock() - last ) / (double)CLOCKS_PER_SEC >= 1 )
   {
      std::cout << "fps=" << nbFps << std::endl;
      nbFps = 0;
      last = clock();
   }

   // generate the texture we are going to draw
   (*applicationVariables.layout).draw( applicationVariables.screen );
/*
   //nll::core::writeBmp( applicationVariables.screen, "c:/screen.bmp" );
   glBindTexture( GL_TEXTURE_2D, applicationVariables.screenTextureId );
   glTexImage2D(GL_TEXTURE_2D, 0, 3, applicationVariables.screen.sizex(), applicationVariables.screen.sizey(),
                0, GL_RGB, GL_UNSIGNED_BYTE, applicationVariables.screen.getBuf() );

   // draw the screen
   glEnable( GL_TEXTURE_2D );
   glBindTexture( GL_TEXTURE_2D, applicationVariables.screenTextureId );

   glBegin(GL_QUADS);
   glTexCoord2d( 0, 0 );
   glVertex2f( 0, 0 );
   glTexCoord2d( 1, 0 );
   glVertex2f( (GLfloat)(*applicationVariables.layout).getSize()[ 0 ], 0 );
   glTexCoord2d( 1, 1 );
   glVertex2f( (GLfloat)(*applicationVariables.layout).getSize()[ 0 ], (GLfloat)(*applicationVariables.layout).getSize()[ 1 ] );
   glTexCoord2d( 0, 1 );
	glVertex2f( 0, (GLfloat)(*applicationVariables.layout).getSize()[ 1 ] );
	glEnd();*/
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

   applicationVariables.screen = nll::core::Image<nll::ui8>( w, h, 3 );
   (*applicationVariables.layout).setSize( nll::core::vector2ui( w, h ) );
   (*applicationVariables.layout).updateLayout();
}

void initGraphics()
{
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LESS );
   glShadeModel( GL_SMOOTH );

   // configure the display texture
   glGenTextures( 1, &applicationVariables.screenTextureId );
   glBindTexture( GL_TEXTURE_2D, applicationVariables.screenTextureId );
   glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST );
}
void mouseButton(int button, int state, int x, int y)
{
   applicationVariables.mouseEvent.mousePosition = nll::core::vector2ui( x, y );
   applicationVariables.mouseEvent.isMouseRightButtonJustReleased = false;
   applicationVariables.mouseEvent.isMouseLeftButtonJustReleased = false;
   applicationVariables.mouseEvent.isMouseRightButtonJustPressed = false;
   applicationVariables.mouseEvent.isMouseLeftButtonJustPressed = false;

   if ( button == GLUT_LEFT_BUTTON )
   {
      if ( state == GLUT_UP )
      {
         applicationVariables.mouseEvent.isMouseLeftButtonJustReleased =true;
         if ( applicationVariables.mouseEvent.isMouseLeftButtonPressed )
         {
            applicationVariables.mouseEvent.mouseLeftReleasedPosition = nll::core::vector2ui( x, y );
         }

         // button unreleased
         applicationVariables.mouseEvent.isMouseLeftButtonPressed = false;
      } 
      if ( state == GLUT_DOWN )
      {
         applicationVariables.mouseEvent.isMouseLeftButtonJustPressed =true;
         if (!applicationVariables.mouseEvent.isMouseLeftButtonPressed )
         {
            // the first click position
            applicationVariables.mouseEvent.mouseLeftClickedPosition = nll::core::vector2ui( x, y );
         }
         applicationVariables.mouseEvent.isMouseLeftButtonPressed = true;
      }
   }

   if ( button == GLUT_RIGHT_BUTTON )
   {
      if ( state == GLUT_UP )
      {
         applicationVariables.mouseEvent.isMouseRightButtonJustReleased =true;
         applicationVariables.mouseEvent.isMouseRightButtonPressed = false;
         applicationVariables.mouseEvent.mouseLeftReleasedPosition = nll::core::vector2ui( x, y );
      } 
      if ( state == GLUT_DOWN )
      {
         applicationVariables.mouseEvent.isMouseRightButtonJustPressed =true;
         applicationVariables.mouseEvent.isMouseRightButtonPressed = true;
         applicationVariables.mouseEvent.mouseLeftClickedPosition = nll::core::vector2ui( x, y );
      }
   }
}

void mouseMotion(int x, int y)
{
   applicationVariables.mouseEvent.mousePosition = nll::core::vector2ui( x, y );
}

void keyboard(unsigned char key, int x, int y)
{
   if ( key == 'q' )
   {
      exit( 0 );
   }
}

int main(int argc, char** argv)
{
  // GLUT Window Initialization:
  glutInit( &argc, argv );
  glutInitWindowSize( (*applicationVariables.layout).getSize()[ 0 ], (*applicationVariables.layout).getSize()[ 1 ] );
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
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
  glutTimerFunc( 33, handleOrders, 0 );

  // Turn the flow of control over to GLUT
  glutMainLoop ();
  return 0;
}
 