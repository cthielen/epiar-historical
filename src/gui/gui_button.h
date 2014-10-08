#ifndef H_GUI_BUTTON
#define H_GUI_BUTTON

#include "gui/gui_session.h"

#define BTN_PRESSED 0x00000001
#define BTN_REDRAW  0x00000010

/* window structure */
typedef struct {
	short int x, y, w, h;
	char *label;
	unsigned char update; /* simple variable meaning a call to update_button is needed */
	Uint32 state;
	void (*callback) (void); /* callback function */
	unsigned char visible;
} gui_button;

/* common window functions */
gui_button *gui_create_button(short int x, short int y, short int w, short int h, char *label, gui_session *session);
int gui_destroy_button(gui_button *button);
void gui_show_button(gui_button *button);
int gui_init_button(void);
int gui_quit_button(void);
void gui_update_button(gui_button *button);
void gui_button_set_callback(gui_button *button, void (*callback) (void));

#endif /* H_GUI_WINDOW */
