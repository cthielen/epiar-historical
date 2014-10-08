#ifndef H_GUI_WINDOW
#define H_GUI_WINDOW

#include "gui/gui_session.h"

/* window structure */
typedef struct {
	int x, y, w, h;
	unsigned char update;
	unsigned char visible;
} gui_window;

/* common window functions */
gui_window *gui_create_window(int x, int y, int w, int h, gui_session *session);
int gui_destroy_window(gui_window *window);
void gui_show_window(gui_window *window);
int gui_init_window(void);
int gui_quit_window(void);

#endif /* H_GUI_WINDOW */
