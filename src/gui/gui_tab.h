#ifndef H_GUI_TAB
#define H_GUI_TAB

#include "gui/gui_session.h"
#include "includes.h"

/* tab structure */
typedef struct {
	int x, y;
	unsigned char update;
	char *tabs;
	int num_tabs;
	int selected; /* which tab is selected */
	void (*callback) (int which);
	gui_frame *associated_frame;
	unsigned char flat;
	unsigned char visible;
	SDL_Surface *bg;
} gui_tab;

/* common tab functions */
gui_tab *gui_create_tab(int x, int y, char *tabs, unsigned char flat, gui_session *session);
int gui_destroy_tab(gui_tab *tab);
void gui_show_tab(gui_tab *tab);
int gui_init_tab(void);
int gui_quit_tab(void);
void gui_tab_set_callback(gui_tab *tab, void (*callback) (int selected));
void gui_tab_associate_frame(gui_tab *tab, gui_frame *frame);

#endif /* H_GUI_TAB */
