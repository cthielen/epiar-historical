#include "com_defs.h"
#include "gui/gui.h"
#include "system/video/video.h"

static SDL_Surface *up_arrow, *down_arrow, *background, *dinge;

static int gui_scrollbar_register_session(gui_session *session, gui_scrollbar *scrollbar);

gui_scrollbar *gui_create_scrollbar(int x, int y, int h, float progress, gui_session *session) {
	gui_scrollbar *sb = (gui_scrollbar *)malloc(sizeof(gui_scrollbar));

	sb->x = x;
	sb->y = y;
	sb->h = h;
	sb->progress = progress;
	sb->update = 0;
	sb->visible = 1;
	sb->buddy = NULL;
	sb->buddy_type = GUI_NONE;

	if (gui_scrollbar_register_session(session, sb) != 0) {
		free(sb);
		return (NULL);
	}

	return (sb);
}

int gui_destroy_scrollbar(gui_scrollbar *scrollbar) {
	if (scrollbar == NULL)
		return (-1);

	free(scrollbar);

	return (0);
}

void gui_show_scrollbar(gui_scrollbar *scrollbar) {
	SDL_Rect src, rect;
	float alt_height;

	if (scrollbar == NULL)
		return;

	if (!scrollbar->visible)
		return;

	/* draw the up arrow */
	rect.x = scrollbar->x;
	rect.y = scrollbar->y;
	rect.w = up_arrow->w;
	rect.h = up_arrow->h;

	blit_surface(up_arrow, NULL, &rect, 0);

	/* draw the down arrow */
	rect.x = scrollbar->x;
	rect.y = scrollbar->y + scrollbar->h - down_arrow->h;
	rect.w = down_arrow->w;
	rect.h = down_arrow->h;

	blit_surface(down_arrow, NULL, &rect, 0);

	/* draw the background, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = scrollbar->h - up_arrow->h - down_arrow->h;

		iterations_needed = pixels_needed / background->h;

		pixels_needed -= (iterations_needed * background->h);

		src.x = 0;
		src.y = 0;
		src.w = background->w;
		src.h = background->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = scrollbar->x;
			rect.y = scrollbar->y + up_arrow->h;
			rect.w = background->w;
			rect.h = background->h;

			rect.y += (i * background->h);

			blit_surface(background, &src, &rect, 0);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.h = pixels_needed;
			rect.x = scrollbar->x;
			rect.y = scrollbar->y + up_arrow->h;
			rect.w = background->w;
			rect.h = pixels_needed;

			rect.y += (i * background->h);

			blit_surface(background, &src, &rect, 0);
		}
	}

	/* determine where to draw the dinge (the scrollbar thing, i don't know what to call it) */
	rect.x = scrollbar->x;
	alt_height = (float)scrollbar->h - (float)up_arrow->h - (float)down_arrow->h - (float)dinge->h;
	rect.y = (int)(scrollbar->progress * alt_height) + scrollbar->y + up_arrow->h;
	rect.w = dinge->w;
	rect.h = dinge->h;

	blit_surface(dinge, NULL, &rect, 0);
}

int gui_init_scrollbar(void) {
	up_arrow = load_image_eaf(epiar_eaf, "gui/sb_au.png", ALPHA);
	down_arrow = load_image_eaf(epiar_eaf, "gui/sb_ad.png", ALPHA);
	background = load_image_eaf(epiar_eaf, "gui/sb_bg.png", 0);
	dinge = load_image_eaf(epiar_eaf, "gui/sb_d.png", 0);

	return (0);
}

int gui_quit_scrollbar(void) {
	SDL_FreeSurface(up_arrow);
	SDL_FreeSurface(down_arrow);
	SDL_FreeSurface(background);
	SDL_FreeSurface(dinge);

	return (0);
}

static int gui_scrollbar_register_session(gui_session *session, gui_scrollbar *scrollbar) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (scrollbar == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = scrollbar;
	session->child_type[slot] = GUI_SCROLLBAR;

	return (0);
}

void gui_scrollbar_set_buddy(gui_scrollbar *scrollbar, void *buddy, enum CHILD_TYPE buddy_type) {
	if (scrollbar == NULL)
		return;
	if (buddy == NULL)
		return;

	if ((buddy_type == GUI_TEXTBOX) || (buddy_type == GUI_LISTBOX)) {
		scrollbar->buddy = buddy;
		scrollbar->buddy_type = buddy_type;
	}
}
