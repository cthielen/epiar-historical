#include "com_defs.h"
#include "gui/gui.h"
#include "includes.h"
#include "system/font.h"
#include "system/video/video.h"

static SDL_Surface *deselected, *selected;

static int gui_tab_register_session(gui_session *session, gui_tab *tab);

gui_tab *gui_create_tab(int x, int y, char *tabs, unsigned char flat, gui_session *session) {
	gui_tab *tab = (gui_tab *)malloc(sizeof(gui_tab));
	int i;

	if (tabs == NULL) {
		free(tab);
		return (NULL);
	}
	if (strlen(tabs) <= 0) {
		free(tab);
		return (NULL);
	}
	if (session == NULL) {
		free(tab);
		return (NULL);
	}

	tab->x = x;
	tab->y = y;
	tab->visible = 1;
	tab->update = 0;
	tab->flat = flat;
	tab->selected = 0; /* 0 as in the first one ... of course */
	tab->callback = NULL;
	tab->associated_frame = NULL;
	tab->bg = NULL;

	tab->num_tabs = 0;
	/* count the number of tabs, all tabs must end in \n, so just count the number of those */
	for (i = 0; i < (signed)strlen(tabs); i++) {
		if (tabs[i] == '\n')
			tab->num_tabs++;
	}

	/* make a copy of the tabs */
	tab->tabs = (char *)malloc(sizeof(char) * (strlen(tabs) + 1));
	memset(tab->tabs, 0, sizeof(char) * (strlen(tabs) + 1));
	strcpy(tab->tabs, tabs);

	if (gui_tab_register_session(session, tab) != 0) {
		free(tab->tabs);
		free(tab);
		return (NULL);
	}

	return (tab);
}

int gui_destroy_tab(gui_tab *tab) {
	if (tab == NULL)
		return (-1);

	free(tab->tabs);
	free(tab);

	return (0);
}

void gui_show_tab(gui_tab *tab) {
	int i;
	char tab_name[25] = {0};
	int cur_tab = 0;
	SDL_Rect rect;
#ifndef WIN32
#warning ASSUMING TAB NAME IS NO MORE THAN 25 CHARACTERS
#endif

	if (tab == NULL)
		return;

	if (!tab->visible)
		return;

	if (tab->num_tabs <= 0)
		return;

	/* draw the background (only for non-flat tabs) */
	if ((tab->bg == NULL) && (!tab->flat)) {
		SDL_Rect src, dest;

#ifndef WIN32
#warning tab w/h assumptions here
#endif
		tab->bg = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, 20, 56 * tab->num_tabs, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
		assert(tab->bg);

		src.x = tab->x;
		src.y = tab->y;
		src.w = 20;
		src.h = 56 * tab->num_tabs;
		dest.x = 0;
		dest.y = 0;
		dest.w = src.w;
		dest.h = src.h;

		SDL_BlitSurface(screen, &src, tab->bg, &dest);
	} else if (!tab->flat) {
		rect.x = tab->x;
		rect.y = tab->y;
		rect.w = tab->bg->w;
		rect.h = tab->bg->h;

		blit_surface(tab->bg, NULL, &rect, 0);
	}

	for (i = 0; i < (signed)strlen(tab->tabs); i++) {
		if (tab->tabs[i] == '\n') {
			SDL_Surface *temp = NULL, *text = NULL, *rotated = NULL;
			int w, h, base;

			/* draw the tab */
			rect.x = tab->x;
			if (tab->flat)
				rect.y = tab->y + (cur_tab * 52);
			else
				rect.y = tab->y + (cur_tab * 56);
			rect.w = deselected->w; /* selected and deselected have the same w/h, so it doesnt matter */
			rect.h = deselected->h;

			if (cur_tab == tab->selected) {
				/* this tab is highlighted */
				if (tab->flat) {
					fill_rect(&rect, map_rgb(48, 142, 141));
				} else {
					blit_surface(selected, NULL, &rect, 0);
				}
			} else {
				/* this tab is not highlighted */
				if (tab->flat) {
					fill_rect(&rect, map_rgb(0, 98, 96));
				} else {
					/* deselected tabs are a little less wide, so we need to right align a little more */
					rect.x += 3;
					blit_surface(deselected, NULL, &rect, 0);
				}
			}

			/* and draw the text name on that ... by, er, drawing the text, then rotating */
			if (cur_tab == tab->selected) {
				temp = epiar_render_text_surf(gui_font_bold, white, 0, AFONT_RENDER_SOLID, screen->format, tab_name);
				epiar_size_text(gui_font_bold, tab_name, &w, &h, &base);
			} else {
				temp = epiar_render_text_surf(gui_font_normal, white, 0, AFONT_RENDER_SOLID, screen->format, tab_name);
				epiar_size_text(gui_font_normal, tab_name, &w, &h, &base);
			}
			assert(temp);
			text = SDL_DisplayFormat(temp);
			rotated = rotate(text, 90.0f);
			assert(rotated);

			rect.x += (base / 2);
			rect.y += (deselected->h / 2);
			rect.y -= (rotated->h / 2); /* center text on tab */
			rect.w = rotated->w;
			rect.h = rotated->h;
			blit_surface(rotated, NULL, &rect, 0);

			SDL_FreeSurface(temp);
			SDL_FreeSurface(text);
			SDL_FreeSurface(rotated);

			if (tab->flat) {
				/* draw the line at the top of it as is in the flat style */
				rect.x = tab->x;
				rect.y = tab->y;
				rect.w = deselected->w; /* selected and deselected have the same w/h, so it doesnt matter */
				rect.h = 1;

				fill_rect(&rect, map_rgb(48, 142, 141));

				/* draw the line at the bottom of it as is in the flat style */
				rect.x = tab->x;
				rect.y = tab->y + (cur_tab * 52) + deselected->h;
				rect.w = deselected->w; /* selected and deselected have the same w/h, so it doesnt matter */
				rect.h = 1;

				fill_rect(&rect, map_rgb(48, 142, 141));

				/* draw the seperating lines as is in the flat style */
				rect.x = tab->x;
				rect.y = tab->y;
				rect.w = 1; /* selected and deselected have the same w/h, so it doesnt matter */
				rect.h = tab->num_tabs * 52;

				fill_rect(&rect, map_rgb(48, 142, 141));

				rect.x += deselected->w;

				fill_rect(&rect, map_rgb(48, 142, 141));
			}

			cur_tab++;
			memset(tab_name, 0, sizeof(char) * 25);
		} else {
			strncat(tab_name, &tab->tabs[i], 1);
		}
	}
}

int gui_init_tab(void) {

	deselected = load_image_eaf(epiar_eaf, "gui/tb_d.png", 0);
	selected = load_image_eaf(epiar_eaf, "gui/tb_s.png", 0);

	return (0);	
}

int gui_quit_tab(void) {

	SDL_FreeSurface(deselected);
	SDL_FreeSurface(selected);

	return (0);	
}

static int gui_tab_register_session(gui_session *session, gui_tab *tab) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (tab == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = tab;
	session->child_type[slot] = GUI_TAB;

	return (0);
}

void gui_tab_set_callback(gui_tab *tab, void (*callback) (int selected)) {
	if (tab == NULL)
		return;
	if (callback == NULL)
		return;

	tab->callback = callback;
}

/* associated a frame w/ the tab, so the frame is redrawn w/ the tab */
void gui_tab_associate_frame(gui_tab *tab, gui_frame *frame) {
	if ((tab == NULL) || (frame == NULL))
		return;

	tab->associated_frame = frame;
}
