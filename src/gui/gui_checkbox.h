#ifndef H_GUI_CHECKBOX
#define H_GUI_CHECKBOX

#include "gui/gui_session.h"

/* checkbox structure */
typedef struct {
	int x, y;
	unsigned char checked;
	unsigned char update;
	void (*callback) (void);
	unsigned char visible;
} gui_checkbox;

/* common checkbox functions */
gui_checkbox *gui_create_checkbox(int x, int y, unsigned char checked, gui_session *session);
int gui_destroy_checkbox(gui_checkbox *checkbox);
void gui_show_checkbox(gui_checkbox *checkbox);
int gui_init_checkbox(void);
int gui_quit_checkbox(void);
void gui_checkbox_set_callback(gui_checkbox *checkbox, void (*callback) (void));

#endif /* H_GUI_CHECKBOX */
