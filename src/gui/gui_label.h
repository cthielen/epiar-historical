#ifndef H_GUI_LABEL
#define H_GUI_LABEL

#include "gui/gui_session.h"
#include "system/font.h"

/* window structure */
typedef struct {
  int x, y, base;
  SDL_Surface *label;
  unsigned char update;
  unsigned char visible;
} gui_label;

/* common window functions */
gui_label *gui_create_label(int x, int y, char *text, gui_session *session);
gui_label *gui_create_label_from_font(int x, int y, char *text, afont *font, gui_session *session);
int gui_destroy_label(gui_label *label);
void gui_show_label(gui_label *label);
int gui_init_label(void);
int gui_quit_label(void);

#endif /* H_GUI_LABEL */
