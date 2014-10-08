#ifndef H_GUI_TEXTBOX
#define H_GUI_TEXTBOX

#include "gui/gui_session.h"

/* textbox structure */
typedef struct {
	int x, y, w, h;
	unsigned char update;
	char *text;
	gui_scrollbar *sb;
	SDL_Surface *bg;
	unsigned char visible;
} gui_textbox;

/* common textbox functions */
gui_textbox *gui_create_textbox(int x, int y, int w, int h, char *text, gui_session *session);
int gui_textbox_change_text(gui_textbox *tb, char *text);
int gui_destroy_textbox(gui_textbox *textbox);
void gui_show_textbox(gui_textbox *textbox);
int gui_init_textbox(void);
int gui_quit_textbox(void);

#endif /* H_GUI_TEXTBOX */
