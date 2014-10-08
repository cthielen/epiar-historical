#ifndef H_GUI_FRAME
#define H_GUI_FRAME

#include "gui/gui_session.h"

/* window structure */
typedef struct {
	int x, y, w, h;
	unsigned char update;
	void (*dc) (int x, int y, int w, int h);
	unsigned char visible;
} gui_frame;

/* common window functions */
gui_frame *gui_create_frame(int x, int y, int w, int h, gui_session *session);
int gui_destroy_frame(gui_frame *frame);
void gui_show_frame(gui_frame *frame);
int gui_init_frame(void);
int gui_quit_frame(void);
void gui_frame_associate_drawing(gui_frame *frame, void (*dc) (int x, int y, int w, int h));

#endif /* H_GUI_FRAME */
