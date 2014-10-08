#include "gui/gui.h"
#include "system/font.h"
#include "system/save.h"
#include "system/video/video.h"

gui_session *save_session;

static void okay_btn_handle(void);

void save_game_menu(void) {
	gui_window *save_win;
	gui_button *okay_btn;

	save_session = gui_create_session();

	save_win = gui_create_window(250, 250, 300, 100, save_session);
	okay_btn = gui_create_button(357, 320, 85, 22, "Okay", save_session);

	gui_button_set_callback(okay_btn, okay_btn_handle);

	gui_session_show_all(save_session);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 330, 280, "Feature not implemented");

	gui_main(save_session);
	SDL_ShowCursor(1);

	gui_session_destroy_all(save_session);
	gui_destroy_session(save_session);
}

static void okay_btn_handle(void) {
	save_session->active = 0;
}
