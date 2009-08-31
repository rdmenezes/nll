#include <GL/freeglut.h>

void RenderObjects()
{
   glClear(GL_COLOR_BUFFER_BIT);
   glPushMatrix();
   glRotatef(0,0.0,0.0,1.0);
   glColor3f(1.0,1.0,1.0);
   glRectf(-25.0,-25.0,25.0,25.0);
   glPopMatrix();
}

void display()
{
   // Clear frame buffer and depth buffer
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   // Set up viewing transformation, looking down -Z axis
   glLoadIdentity();

   // Render the scene
   RenderObjects();

   // Make sure changes appear onscreen
   glutSwapBuffers();
}

void reshape( GLint w, GLint h )
{
   glViewport(0,0,(GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-50.0,50.0,-50.0,50.0,-1.0,1.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

void InitGraphics()
{
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LESS );
   glShadeModel( GL_SMOOTH );  
   //gluLookAt(0, 0, -1, 0, 0, -1, 0, 1, 0);
}
void MouseButton(int button, int state, int x, int y)
{
}

void MouseMotion(int x, int y)
{
}

void Keyboard(unsigned char key, int x, int y)
{
}

int main(int argc, char** argv)
{
  // GLUT Window Initialization:
  glutInit (&argc, argv);
  glutInitWindowSize (512, 512);
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow ("CS248 GLUT example");
  // Initialize OpenGL graphics state
  InitGraphics();
  // Register callbacks:
  glutDisplayFunc (display);
  glutReshapeFunc (reshape);
  glutKeyboardFunc (Keyboard);
  glutMouseFunc (MouseButton);
  glutMotionFunc (MouseMotion);
  //glutIdleFunc (AnimateScene);
  
  // Turn the flow of control over to GLUT
  glutMainLoop ();
  return 0;
}
 