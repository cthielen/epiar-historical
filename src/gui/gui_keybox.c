#include "gui/gui.h"
#include "system/font.h"
#include "system/video/video.h"

static int gui_keybox_register_session(gui_session *session, gui_keybox *kb);

static gui_session *new_key_session = NULL;

gui_keybox *gui_create_keybox(int x, int y, char *label, SDLKey *associated_key, gui_session *session) {
	gui_keybox *kb = (gui_keybox *)malloc(sizeof(gui_keybox));

	if (label == NULL) {
		free(kb);
		return (NULL);
	}
	if (strlen(label) <= 0) {
		free(kb);
		return (NULL);
	}
	if (session == NULL) {
		free(kb);
		return (NULL);
	}

	/* ensure registration first */
	if (gui_keybox_register_session(session, kb) != 0) {
		free(kb);
		return (NULL);
	}

	kb->x = x;
	kb->y = y;
	kb->update = 0;
	kb->visible = 1;
	kb->trans_area = NULL;
	kb->associated_key = associated_key;
	kb->label = gui_create_label(x + 55, y, label, session);

	return (kb);
}

int gui_destroy_keybox(gui_keybox *kb) {
	if (kb == NULL)
		return (-1);

	/* note: do not destroy the kb->label. the session will destroy that for us on gui_session_destroy_all() */
	free(kb);

	return (0);
}

void gui_show_keybox(gui_keybox *kb) {
	SDL_Rect rect;
	char key[80] = {0};
	int w, h, base;

	if (!kb->visible)
		return;

	/* set the text to print */
	sprintf(key, "%s", SDL_GetKeyName(*kb->associated_key));

	/* show the label, even though it might already have been show via gui_session_show_all(), we need */
	/* to show it ourselves in case we want to just display the keybox after redraw or something odd */
	gui_show_label(kb->label);

	/* render the transparent input box */
	if (kb->trans_area == NULL) {
		SDL_Surface *area;
		SDL_Rect src, dest;

		area = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, 50, 16, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
		assert(area);
		rect.x = 0;
		rect.y = 0;
		rect.w = area->w;
		rect.h = area->h;
		SDL_FillRect(area, &rect, SDL_MapRGB(area->format, 0, 0, 0));
		SDL_SetAlpha(area, SDL_SRCALPHA, 93);

		rect.x = kb->x + 1;
		rect.y = kb->y + 1;
		rect.w = area->w;
		rect.h = area->h;
		blit_surface(area, NULL, &rect, 0);

		SDL_FreeSurface(area);

		/* now that the trans area is drawn, back up that area there */
		kb->trans_area = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, 50, 16, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
		src.x = kb->x + 1;
		src.y = kb->y + 1;
		src.w = kb->trans_area->w;
		src.h = kb->trans_area->h;
		dest.x = 0;
		dest.y = 0;
		dest.w = src.w;
		dest.h = src.h;

		SDL_BlitSurface(screen, &src, kb->trans_area, &dest);
	} else {
		rect.x = kb->x + 1;
		rect.y = kb->y + 1;
		rect.w = kb->trans_area->w;
		rect.h = kb->trans_area->h;

		blit_surface(kb->trans_area, NULL, &rect, 0);
	}

	/* draw the box around the transparent area */
	/* top bar */
	rect.x = kb->x;
	rect.y = kb->y;
	rect.w = 51;
	rect.h = 1;
	fill_rect(&rect, map_rgb(7, 100, 138));
	/* bottom bar */
	rect.y += 16;
	fill_rect(&rect, map_rgb(7, 100, 138));
	/* left bar */
	rect.y = kb->y;
	rect.w = 1;
	rect.h = 16;
	fill_rect(&rect, map_rgb(7, 100, 138));
	/* right bar */
	rect.x += 50;
	fill_rect(&rect, map_rgb(7, 100, 138));

	/* render the text onto it */
	epiar_size_text(gui_font_normal, key, &w, &h, &base);
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, kb->x + 4, kb->y + base + 4, key);
}

int gui_init_keybox(void) {
	return (0);
}

int gui_quit_keybox(void) {
	return (0);
}

static int gui_keybox_register_session(gui_session *session, gui_keybox *kb) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (kb == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = kb;
	session->child_type[slot] = GUI_KEYBOX;

	return (0);
}

/* dialog box that asks for to press any key and returns what key was pressed */
SDLKey gui_keybox_prompt_new_key(void) {
	gui_window *dialog_wnd;
	gui_label *pak_lbl;
	unsigned char finished = 0;
	SDL_Event event;
	SDLKey the_key;
	SDL_Rect rect;

	new_key_session = gui_create_session();

	dialog_wnd = gui_create_window(275, 227, 250, 145, new_key_session);

	pak_lbl = gui_create_label(357, 285, "Press any key", new_key_session);

	gui_session_show_all(new_key_session);

	flip();

	while(!finished) {
		SDL_WaitEvent(&event);

		if (event.type == SDL_KEYDOWN) {
			the_key = event.key.keysym.sym;
			/* printf("%s was pressed\n", SDL_GetKeyName(the_key)); */
			finished = 1;
		}
	}

	gui_session_destroy_all(new_key_session);
	gui_destroy_session(new_key_session);

	rect.x = 275;
	rect.y = 227;
	rect.w = 250;
	rect.h = 145;
	fill_rect(&rect, map_rgb(0, 0, 0));

	return (the_key);
}
