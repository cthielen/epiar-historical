#ifndef H_GUI_SCROLLBAR
#define H_GUI_SCROLLBAR

#include "gui/gui_session.h"

/* window structure */
typedef struct {
	int x, y, h;
	float progress;
	unsigned char update;
	void *buddy;
	enum CHILD_TYPE buddy_type;
	unsigned char visible;
} gui_scrollbar;

/* common window functions */
gui_scrollbar *gui_create_scrollbar(int x, int y, int h, float progress, gui_session *session);
int gui_destroy_scrollbar(gui_scrollbar *scrollbar);
void gui_show_scrollbar(gui_scrollbar *scrollbar);
int gui_init_scrollbar(void);
int gui_quit_scrollbar(void);
void gui_scrollbar_set_buddy(gui_scrollbar *scrollbar, void *buddy, enum CHILD_TYPE buddy_type);

#endif /* H_GUI_SCROLLBAR */
