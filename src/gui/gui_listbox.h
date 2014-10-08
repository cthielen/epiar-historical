#ifndef H_GUI_LISTBOX
#define H_GUI_LISTBOX

#include "gui/gui_session.h"

/* listbox structure */
typedef struct {
  int x, y, w, h, item_h;
  unsigned char update;
  unsigned char visible;
  gui_scrollbar *sb;
  void (*callback) (void *lb, int x, int y, int w, int h, int item);
  int selected;
  int num_items;
  int top_scrolled; /* the item # for the top most item if you're scrolled down a little */
  SDL_Surface *trans_area; /* the area under the scrollbar, used in drawing */
} gui_listbox;

/* common listbox functions */
gui_listbox *gui_create_listbox(int x, int y, int w, int h, int item_h, gui_session *session);
int gui_listbox_set_callback(gui_listbox *lb, void (*callback) (void *lb, int x, int y, int w, int h, int item));
int gui_listbox_add_item(gui_listbox *lb);
int gui_destroy_listbox(gui_listbox *listbox);
void gui_show_listbox(gui_listbox *lb);
int gui_init_listbox(void);
int gui_quit_listbox(void);

#endif /* H_GUI_LISTBOX */
