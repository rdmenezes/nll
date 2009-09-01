#include "init.h"
#include <GL/freeglut.h>

static mvv::ApplicationVariables applicationVariables;
static double sx = 0.75, sy = 0.75;
static double dx = 0.38, dy = 0.38;
static int resx = 256;
static int resy = 256;

void handleOrders( int value )
{
   glutTimerFunc( 3, handleOrders, 0 );

   // code
   applicationVariables.handleOrders();
   applicationVariables.runEngines();


   static int nbFps = 0;
   static unsigned last = clock();
   ++nbFps;

   if ( ( clock() - last ) / (double)CLOCKS_PER_SEC >= 1 )
   {
      std::cout << "fps=" << nbFps << std::endl;
      nbFps = 0;
      last = clock();
   }
}

void renderObjects()
{
   // generate the texture we are going to draw
   applicationVariables.rootLayout->draw( applicationVariables.screen );
   glBindTexture( GL_TEXTURE_2D, applicationVariables.screenTextureId );
   glTexImage2D(GL_TEXTURE_2D, 0, 3, applicationVariables.screen.sizex(), applicationVariables.screen.sizey(),
                0, GL_RGB, GL_UNSIGNED_BYTE, applicationVariables.screen.getBuf() );

   // draw the screen
   glEnable( GL_TEXTURE_2D );
   glBindTexture( GL_TEXTURE_2D, applicationVariables.screenTextureId );

   glBegin(GL_QUADS);
   glTexCoord2d( 0, 0 );
	glVertex2f(0,resy);
   glTexCoord2d( 1, 0 );
	glVertex2f(resx,resy);
   glTexCoord2d( 1, 1 );
	glVertex2f(resx,0);
   glTexCoord2d( 0, 1 );
	glVertex2f(0,0);
	glEnd();

   //applicationVariables.originMpr1.setValue( 2, applicationVariables.originMpr1[ 2 ] - 0.03 );
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

   resx = w;
   resy = h;
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
}

void mouseMotion(int x, int y)
{
   //std::cout << "y = " << y << std::endl;
   applicationVariables.originMpr1.setValue( 2, (double)y / resy * 82 );
}

void keyboard(unsigned char key, int x, int y)
{
   if ( key == 'q' )
      exit( 0 );
   if ( key == 'k' )
      applicationVariables.originMpr1.setValue( 2, applicationVariables.originMpr1[ 2 ] + 0.3 );
   if ( key == 'l' )
      applicationVariables.originMpr1.setValue( 2, applicationVariables.originMpr1[ 2 ] - 0.3 );
}

int main(int argc, char** argv)
{
  // GLUT Window Initialization:
  glutInit (&argc, argv);
  glutInitWindowSize (512, 512);
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow ("Medical Volume Viewer");
  //glutGameModeString( "1280x800:32" );
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
 