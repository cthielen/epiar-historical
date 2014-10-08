#ifndef H_GUI_IMAGE
#define H_GUI_IMAGE

#include "gui/gui_session.h"

/* window structure */
typedef struct {
  SDL_Surface *image;
  short int x, y;
  unsigned char update;
  unsigned char visible;
} gui_image;

/* common window functions */
gui_image *gui_create_image(short int x, short int y, char *filename, gui_session *session);
gui_image *gui_create_image_eaf(FILE *eaf_file, short int x, short int y, char *filename, gui_session *session);
int gui_destroy_image(gui_image *image);
void gui_show_image(gui_image *image);
int gui_init_image(void);
int gui_quit_image(void);

#endif /* H_GUI_IMAGE */
