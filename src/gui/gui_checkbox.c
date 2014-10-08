#include "com_defs.h"
#include "gui/gui.h"
#include "system/video/video.h"

static SDL_Surface *on, *off;

static int gui_checkbox_register_session(gui_session *session, gui_checkbox *checkbox);

gui_checkbox *gui_create_checkbox(int x, int y, unsigned char checked, gui_session *session) {
	gui_checkbox *checkbox = (gui_checkbox *)malloc(sizeof(gui_checkbox));

	checkbox->x = x;
	checkbox->y = y;
	checkbox->visible = 1;
	checkbox->checked = checked;
	checkbox->callback = NULL;

	if (gui_checkbox_register_session(session, checkbox) != 0) {
		free(checkbox);
		return (NULL);
	}

	return (checkbox);
}

int gui_destroy_checkbox(gui_checkbox *checkbox) {
	if (checkbox == NULL)
		return (-1);

	free(checkbox);

	return (0);
}

void gui_show_checkbox(gui_checkbox *checkbox) {
	SDL_Rect rect;

	if (checkbox == NULL)
		return;

	if (!checkbox->visible)
		return;

	rect.x = checkbox->x;
	rect.y = checkbox->y;
	rect.w = on->w;
	rect.h = on->h; /* on and off have the same dimensions */

	if (checkbox->checked)
		blit_surface(on, NULL, &rect, 0);
	else
		blit_surface(off, NULL, &rect, 0);
}

int gui_init_checkbox(void) {

	on = load_image_eaf(epiar_eaf, "gui/cb_c.png", 0);
	off = load_image_eaf(epiar_eaf, "gui/cb_uc.png", 0);

	return (0);
}

int gui_quit_checkbox(void) {
	SDL_FreeSurface(on);
	SDL_FreeSurface(off);

	return (0);
}

static int gui_checkbox_register_session(gui_session *session, gui_checkbox *checkbox) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (checkbox == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = checkbox;
	session->child_type[slot] = GUI_CHECKBOX;

	return (0);
}

void gui_checkbox_set_callback(gui_checkbox *checkbox, void (*callback) (void)) {
	if (checkbox == NULL)
		return;
	if (callback == NULL)
		return;

	checkbox->callback = callback;
}
