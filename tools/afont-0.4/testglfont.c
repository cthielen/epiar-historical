/* testglfont.c */

#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"

#include "afont_gl.h"

int main( int argc, char **argv )
{
  afont *a;
  afont *a2;
  SDL_Event event;
  SDL_Surface *screen;
  Uint32 videoflags;
  int quit = 0;

  if(argc < 2) {
    fprintf(stderr, "Usage: %s [font file]\n", argv[0]);
    return 0;
  }

  /* Init SDL */
  SDL_Init(SDL_INIT_VIDEO);
  atexit(SDL_Quit);

  a = afont_load(argv[1]);
  afont_gl_make_bitmaps(a);

  if(argc == 3) {
    a2 = afont_load(argv[2]);
    afont_gl_make_bitmaps(a2);
  } else {
    a2 = NULL;
  }

  videoflags = SDL_OPENGL;
  videoflags |= SDL_GL_DOUBLEBUFFER;
  videoflags |= SDL_HWPALETTE;
  videoflags |= SDL_HWSURFACE;
  videoflags |= SDL_HWACCEL;

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  screen = SDL_SetVideoMode(640, 480, 32, videoflags);

  /* Initialize OpenGL */
  glShadeModel( GL_SMOOTH );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClearDepth( 1.0f );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  /* Setup our viewport. */
  glViewport( 0, 0, 640, 480 );

  /* change to the projection matrix and set our viewing volume. */
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity( );

  /* Set our perspective */
  gluPerspective( 45.0f, 640.0 / 480.0, 0.1f, 100.0f );

  /* Make sure we're changing the model view and not the projection */
  glMatrixMode( GL_MODELVIEW );

  /* Reset The View */
  glLoadIdentity( );

  while(!quit) {
    char buf[128];
    
    /* Draw the screen */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity( );
    glTranslatef(0.0, 0.0, -1.0);
    glColor3f(1.0, 1.0, 0.0);
    glRasterPos2f(-0.25, 0.0);
    sprintf(buf, "The time is: %d", SDL_GetTicks());
    afont_gl_render_text(a, buf);

    if(a2) {
      glRasterPos2f(-0.25, 0.10);
      sprintf(buf, "The time is: %d", SDL_GetTicks());
      afont_gl_render_text(a2, buf);
    }

    SDL_GL_SwapBuffers();
    SDL_Delay(200);

    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_KEYDOWN)
        quit = 1;
    }
 
  }

  afont_free(a);

  return 0;
}
