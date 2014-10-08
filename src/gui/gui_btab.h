#ifndef H_GUI_BTAB
#define H_GUI_BTAB

#include "gui/gui_session.h"
#include "includes.h"

/* btab structure */
typedef struct {
	int x, y;
	unsigned char update;
	unsigned char visible;
	int n_items;
	int selected;
	SDL_Surface **bts_off, **bts_on;
	void (*callback) (int which);
} gui_btab;

/* common btab functions */
gui_btab *gui_create_btab(int x, int y, gui_session *session);
int gui_btab_add_tab(gui_btab *bt, char *file_off, char *file_on);
int gui_btab_add_tab_eaf(FILE *eaf, gui_btab *bt, char *file_off, char *file_on);
int gui_btab_set_callback(gui_btab *bt, void (*callback) (int which));
int gui_destroy_btab(gui_btab *bt);
void gui_show_btab(gui_btab *bt);
int gui_init_btab(void);
int gui_quit_btab(void);

/* gui functions (you don't need them really) */
int gui_btab_check_clicks(gui_btab *bt, int x, int y);

#endif /* H_GUI_BTAB */
