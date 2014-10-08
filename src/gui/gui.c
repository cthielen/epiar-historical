#include "com_defs.h"
#include "gui/gui.h"
#include "includes.h"
#include "input/input.h"
#include "system/font.h"
#include "system/eaf.h"
#include "system/path.h"
#include "system/timer.h"

#define LEFT_BUTTON_DOWN 0x1

afont *gui_font_normal = NULL, *gui_font_bold = NULL;

int gui_init(void) {

	if (gui_init_window() != 0)
		return (-1);
	if (gui_init_text_entry() != 0)
		return (-1);
	if (gui_init_button() != 0)
		return (-1);
	if (gui_init_image() != 0)
		return (-1);
	if (gui_init_checkbox() != 0)
		return (-1);
	if (gui_init_frame() != 0)
		return (-1);
	if (gui_init_label() != 0)
		return (-1);
	if (gui_init_scrollbar() != 0)
		return (-1);
	if (gui_init_textbox() != 0)
		return (-1);
	if (gui_init_tab() != 0)
		return (-1);
	if (gui_init_keybox() != 0)
		return (-1);
	if (gui_init_listbox() != 0)
		return (-1);
	if (gui_init_btab() != 0)
		return (-1);

	/* load Vera.af from EAF archive */
	if (eaf_find_file(epiar_eaf, "fonts/Vera.af") != 0) {
		printf("Couldn't find \"fonts/Vera.af\" in epiar.eaf.\n");
		return (-1);
	}

	fseek(epiar_eaf, 29, SEEK_CUR); /* get to the start of the file inside the eaf archive */

	gui_font_normal = epiar_load_fp(epiar_eaf);
	assert(gui_font_normal);

	/* load VeraBd.af from EAF archive */
	if (eaf_find_file(epiar_eaf, "fonts/VeraBd.af") != 0) {
		printf("Couldn't find \"fonts/Vera.af\" in epiar.eaf.\n");
		return (-1);
	}

	fseek(epiar_eaf, 29, SEEK_CUR); /* get to the start of the file inside the eaf archive */

	gui_font_bold = epiar_load_fp(epiar_eaf);
	assert(gui_font_bold);

	return (0);
}

int gui_quit(void) {

	if (gui_quit_window() != 0)
		return (-1);
	if (gui_quit_text_entry() != 0)
		return (-1);
	if (gui_quit_button() != 0)
		return (-1);
	if (gui_quit_image() != 0)
		return (-1);
	if (gui_quit_checkbox() != 0)
		return (-1);
	if (gui_quit_frame() != 0)
		return (-1);
	if (gui_quit_label() != 0)
		return (-1);
	if (gui_quit_scrollbar() != 0)
		return (-1);
	if (gui_quit_textbox() != 0)
		return (-1);
	if (gui_quit_tab() != 0)
		return (-1);
	if (gui_quit_keybox() != 0)
		return (-1);
	if (gui_quit_listbox() != 0)
		return (-1);
	if (gui_quit_btab() != 0)
		return (-1);

	epiar_free(gui_font_normal);
	gui_font_normal = NULL;
	epiar_free(gui_font_bold);
	gui_font_bold = NULL;

	return (0);
}

/* main loop that checks input and runs callbacks */
void gui_main(gui_session *session) {
	SDL_Event event;
	Uint8 mouse_state = 0;
	int a;

	pause_timer();

	while(SDL_PollEvent(&event)); /* clear any pending events */

	a = SDL_ShowCursor(1);

	while (session->active == 1) {

		while(SDL_PollEvent(&event)) {

			switch(event.type) {
			case SDL_QUIT:
				session->active = 0;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				{
					gui_session_pass_keyboard_input(&event, session); /* passes leftovers to the focused widget */
					break;
				}
			case SDL_MOUSEBUTTONUP:
				{
					if ((event.button.state == SDL_RELEASED) && (mouse_state & LEFT_BUTTON_DOWN)) {
						gui_session_pass_mouse_input(&event, session, GS_BTN_LEFT_RELEASED);
						mouse_state ^= LEFT_BUTTON_DOWN; /* turn it off */
					}
					break;
				}
			case SDL_MOUSEBUTTONDOWN:
				{
					if (event.button.button == SDL_BUTTON_LEFT)
						mouse_state |= LEFT_BUTTON_DOWN;
					gui_session_pass_mouse_input(&event, session, GS_BTN_DOWN);
					break;
				}
			default:
				break;
			}
		}

		gui_session_update(session);

		SDL_Delay(10);
	}

	SDL_ShowCursor(a);

	unpause_timer();

	reset_input();
}
