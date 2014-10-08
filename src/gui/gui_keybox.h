#ifndef H_GUI_KEYBOX
#define H_GUI_KEYBOX

#include "gui/gui_session.h"

/* keybox structure */
typedef struct {
	int x, y;
	unsigned char update;
	unsigned char visible;
	gui_label *label;
	SDL_Surface *trans_area;
	SDLKey *associated_key;
} gui_keybox;

/* common keybox functions */
gui_keybox *gui_create_keybox(int x, int y, char *label, SDLKey *associated_key, gui_session *session);
int gui_destroy_keybox(gui_keybox *kb);
void gui_show_keybox(gui_keybox *kb);
int gui_init_keybox(void);
int gui_quit_keybox(void);
SDLKey gui_keybox_prompt_new_key(void);

#endif /* H_GUI_KEYBOX */
