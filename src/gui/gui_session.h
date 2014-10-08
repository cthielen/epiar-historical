#ifndef H_GUI_SESSION
#define H_GUI_SESSION

#include "gui/gui.h"
#include "includes.h"

#define MAX_SESSION_CHILDREN 75

/* definitions for gui_session_pass_mouse_input state */
#define GS_BTN_DOWN          0x1
#define GS_BTN_LEFT_RELEASED 0x2

enum CHILD_TYPE {GUI_NONE, GUI_WINDOW, GUI_TEXT_ENTRY, GUI_BUTTON, GUI_IMAGE, GUI_CHECKBOX, GUI_FRAME, GUI_LABEL, GUI_SCROLLBAR, GUI_TEXTBOX, GUI_TAB, GUI_KEYBOX, GUI_LISTBOX, GUI_BTAB};

typedef struct {
	unsigned char active; /* whether or not the session is running */
	void *children[MAX_SESSION_CHILDREN]; /* widgets (windows, text entries, etc.) controlled by session */
	enum CHILD_TYPE child_type[MAX_SESSION_CHILDREN]; /* descriptors for the void * */
	short int keyboard_focus; /* which child has keyboard focus */
	unsigned char mouse_down;
} gui_session;

gui_session *gui_create_session(void);
int gui_destroy_session(gui_session *session);
void gui_session_activate(gui_session *session);
void gui_session_show_all(gui_session *session); /* displays all session children */
int gui_session_destroy_all(gui_session *session); /* destroys all session children, but not the session itself */
void gui_session_pass_keyboard_input(SDL_Event *event, gui_session *session); /* gives keyboard input to widget */
void gui_session_pass_mouse_input(SDL_Event *event, gui_session *session, Uint8 state); /* gives mouse input to widget */
void gui_session_update(gui_session *session);

#endif /* H_GUI_SESSION */
