#include "gui/gui.h"
#include "system/font.h"
#include "system/video/video.h"

static gui_session *session;
static int question_response;

static void okay_cb(void);
static void yes_cb(void);
static void no_cb(void);

void gui_alert(char *msg) {
	gui_window *window;
	gui_image *icon;
	gui_button *okay;
	gui_label *lbl;
	int w, h, base;
	SDL_Rect rect;

	if (msg == NULL)
		return;

	session = gui_create_session();

	window = gui_create_window(250, 225, 300, 150, session);
	icon = gui_create_image(275, 255, "data/icons/alert.png", session);
	okay = gui_create_button(350, 320, 100, 35, "Noticed", session);
	gui_button_set_callback(okay, okay_cb);

	epiar_size_text(gui_font_normal, msg, &w, &h, &base);
	lbl = gui_create_label(410 - (w / 2), 266, msg, session);

	gui_session_activate(session);

	gui_session_show_all(session);

	flip();
	gui_main(session);

	gui_session_destroy_all(session);

	rect.x = 250;
	rect.y = 225;
	rect.w = 300;
	rect.h = 150;
	fill_rect(&rect, map_rgb(0, 0, 0));
}

void okay_cb(void) {
	session->active = 0;
}

int gui_question(char *msg) {
	gui_window *window;
	gui_image *icon;
	gui_button *yes_btn, *no_btn;
	gui_label *lbl;
	int w, h, base;
	SDL_Rect rect;

	if (msg == NULL)
		return (0);

	question_response = 0; /* no by default */

	session = gui_create_session();

	window = gui_create_window(250, 225, 300, 150, session);
	icon = gui_create_image(275, 255, "data/icons/question.png", session);
	yes_btn = gui_create_button(275, 320, 100, 35, "Yes", session);
	gui_button_set_callback(yes_btn, yes_cb);
	no_btn = gui_create_button(425, 320, 100, 35, "No", session);
	gui_button_set_callback(no_btn, no_cb);

	epiar_size_text(gui_font_normal, msg, &w, &h, &base);
	lbl = gui_create_label(410 - (w / 2), 266, msg, session);

	gui_session_activate(session);

	gui_session_show_all(session);

	flip();
	gui_main(session);

	gui_session_destroy_all(session);

	rect.x = 250;
	rect.y = 225;
	rect.w = 300;
	rect.h = 150;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));

	return (question_response);
}

static void yes_cb(void) {
	question_response = 1;
	session->active = 0;
}

static void no_cb(void) {
	question_response = 0;
	session->active = 0;
}
