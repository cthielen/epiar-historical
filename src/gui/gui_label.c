#include "gui/gui.h"
#include "system/font.h"
#include "system/video/video.h"

static int gui_label_register_session(gui_session *session, gui_label *label);

gui_label *gui_create_label_from_font(int x, int y, char *text, afont *font, gui_session *session) {
  gui_label *label = (gui_label *)malloc(sizeof(gui_label));
  int w, h;
  
  if (text == NULL) {
    free(label);
    return (NULL);
  }
  
  if (strlen(text) <= 0) {
    free(label);
    return (NULL);
  }
  
  label->x = x;
  label->y = y;
  label->visible = 1;
  
  label->label = epiar_render_text_surf(font, white, 0, AFONT_RENDER_BLENDED, screen->format, text);
  epiar_size_text(font, text, &w, &h, &label->base);
  
  if (gui_label_register_session(session, label) != 0) {
    SDL_FreeSurface(label->label);
    free(label);
    return (NULL);
  }
  
  return (label);
}

gui_label *gui_create_label(int x, int y, char *text, gui_session *session) {
  return (gui_create_label_from_font(x, y, text, gui_font_normal, session));
}

int gui_destroy_label(gui_label *label) {
	if (label == NULL)
		return (-1);

	SDL_FreeSurface(label->label);
	free(label);

	return (0);
}

void gui_show_label(gui_label *label) {
	SDL_Rect rect;

	if (label == NULL)
		return;

	if (!label->visible)
		return;

	rect.x = label->x;
	rect.y = label->y + (label->base / 2);
	rect.w = label->label->w;
	rect.h = label->label->h;

	blit_surface(label->label, NULL, &rect, 0);
}

int gui_init_label(void) {

	return (0);	
}

int gui_quit_label(void) {

	return (0);
}

static int gui_label_register_session(gui_session *session, gui_label *label) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (label == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = label;
	session->child_type[slot] = GUI_LABEL;

	return (0);
}
