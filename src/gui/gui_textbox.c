#include "gui/gui.h"
#include "system/font.h"
#include "system/video/video.h"

static int gui_textbox_register_session(gui_session *session, gui_textbox *textbox);

char *wrap_text(char *text, int w);
int calculate_drawn_lines(gui_textbox *textbox, int i);
int one_line_length(gui_textbox *textbox, int i);

gui_textbox *gui_create_textbox(int x, int y, int w, int h, char *text, gui_session *session) {
	gui_textbox *tb = (gui_textbox *)malloc(sizeof(gui_textbox));

	if ((text == NULL) || (session == NULL)) {
		free(tb);
		return (NULL);
	}

	tb->x = x;
	tb->y = y;
	tb->w = w;
	tb->h = h;
	tb->visible = 1;
	tb->update = 0;
	tb->bg = NULL;

	tb->text = wrap_text(text, w - 30);
	if (tb->text == NULL) {
		printf("error: could not wrap text for textbox\n");
		free(tb);
		return (NULL);
	}

	if (gui_textbox_register_session(session, tb) != 0) {
		free(tb->text);
		free(tb);
		return (NULL);
	}

	/* it's important the tb _does_ exist _for_sure_ before we do this, as it's a mess to clean up */
	tb->sb = gui_create_scrollbar(tb->x + tb->w - 17, tb->y, tb->h, 0.0, session);
	gui_scrollbar_set_buddy(tb->sb, tb, GUI_TEXTBOX);

	return (tb);
}

int gui_destroy_textbox(gui_textbox *textbox) {
	if (textbox == NULL)
		return (-1);

	free(textbox->text);
	/* note we do not destroy our scrollbar as the gui_session_destroy_all() will handle that */
	if (textbox->bg)
		SDL_FreeSurface(textbox->bg);
	free(textbox);

	return (0);
}

void gui_show_textbox(gui_textbox *textbox) {
	SDL_Rect src, dest;
	int i, j;
	int dist = 0;
	char buf[80] = {0};
	int chars_in_line = 0;
	int lines_drawn = 0;
	int max_lines = 0;
	int lines_total = 0;

	if (textbox == NULL)
		return;

	if (!textbox->visible)
		return;

#ifndef WIN32
#warning ASSUMING SCROLLBAR WIDTH TO BE 17 PIXELS
#endif
	if (textbox->bg == NULL) {
		/* there is no cached background (used for transparency), so, create one */
		textbox->bg = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, textbox->w - 19, textbox->h - 2, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
		assert(textbox->bg);
		SDL_SetAlpha(textbox->bg, SDL_SRCALPHA, 93);

		dest.x = textbox->x + 1;
		dest.y = textbox->y + 1;
		dest.w = textbox->w - 19;
		dest.h = textbox->h - 2;
		blit_surface(textbox->bg, NULL, &dest, 0);

		SDL_SetAlpha(textbox->bg, SDL_SRCALPHA, 255);

		src.x = dest.x;
		src.y = dest.y;
		src.w = dest.w;
		src.h = dest.h;
		dest.x = 0;
		dest.y = 0;

		SDL_BlitSurface(screen, &src, textbox->bg, &dest);
	} else {
		/* cached surface exists, so, use it */
		dest.x = textbox->x + 1;
		dest.y = textbox->y + 1;
		dest.w = textbox->bg->w;
		dest.h = textbox->bg->h;

		blit_surface(textbox->bg, NULL, &dest, 0);
	}

	/* display border */
	dest.x = textbox->x;
	dest.y = textbox->y;
	dest.w = textbox->w;
	dest.h = 1;
	fill_rect(&dest, map_rgb(7, 100, 138));
	dest.w = 1;
	dest.h = textbox->h;
	fill_rect(&dest, map_rgb(7, 100, 138));
	dest.x = textbox->x;
	dest.y = textbox->y + textbox->h;
	dest.w = textbox->w;
	dest.h = 1;
	fill_rect(&dest, map_rgb(7, 100, 138));
	dest.x += textbox->w;
	dest.y = textbox->y;
	dest.w = 1;
	dest.h = textbox->h;
	fill_rect(&dest, map_rgb(7, 100, 138));

	/* the displaying of the scrollbar is handled by the session */
	/* first line to draw should be at the closest line break and determined by the scroll bar */
	i = (int)(textbox->sb->progress * (float)strlen(textbox->text));

	/* find the nearest text break to i */
	for (j = i; j > 0; j--) {
		if (textbox->text[j] == 1)
			break;
		else
			dist++;
	}
	i -= (dist - 1);

	if (i == 1) i = 0; /* special case */

	/* see how many lines we can get for that */
	lines_total = calculate_drawn_lines(textbox, i);

	max_lines = (textbox->h - 2) / 18;

	/* if we arent going to fill up the box ... try to anyway */
	if (lines_total < max_lines) {
		while(lines_total < max_lines) {
			int change;

			change = one_line_length(textbox, i);

			if (change == 0)
				break;

			i -= change;

			if (i < 0) {
				i = 0;
				break;
			}
			lines_total = calculate_drawn_lines(textbox, i);
		}
	}

	/* finally, draw the text */
	for (; i < (signed)strlen(textbox->text); i++) {
		if ((lines_drawn * 18) > (textbox->h - 2))
			break;

		if (textbox->text[i] == 1) {
			int w, h, base;

			epiar_size_text(gui_font_normal, buf, &w, &h, &base);
			epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, textbox->x + 2, textbox->y + 4 + (lines_drawn * 16) + base, buf);

			chars_in_line = 0;
			memset(buf, 0, sizeof(char) * 80);
			lines_drawn++;
		} else {
			buf[chars_in_line] = textbox->text[i];
			chars_in_line++;
		}
	}

	if ((lines_drawn * 18) <= (textbox->h - 2)) {
		if (chars_in_line != 0) {
			int w, h, base;

			epiar_size_text(gui_font_normal, buf, &w, &h, &base);
			epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, textbox->x + 2, textbox->y + 4 + (lines_drawn * 16) + base, buf);
		}
	}
}

int gui_init_textbox(void) {

	return (0);
}

int gui_quit_textbox(void) {

	return (0);
}

static int gui_textbox_register_session(gui_session *session, gui_textbox *textbox) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (textbox == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = textbox;
	session->child_type[slot] = GUI_TEXTBOX;

	return (0);
}

char *wrap_text(char *text, int w) {
	int i;
	int lbreak = 0; /* last break */
	int chars_in_line = 0;
	char line_so_far[80] = {0};
	int text_w, text_h, text_base;
	char *wrapped = (char *)malloc(sizeof(char) * (strlen(text) + 1));
	memset(wrapped, 0, sizeof(char) * (strlen(text) + 1));
	strcpy(wrapped, text);

	if (text == NULL) {
		free(wrapped);
		return (NULL);
	}

	if (strlen(text) <= 0) {
		free(wrapped);
		return (NULL);
	}

	if (w <= 0) {
		free(wrapped);
		return (NULL);
	}

	for (i = 0; i < (signed)strlen(text); i++) {
		if (text[i] == ' ') lbreak = i;

		line_so_far[chars_in_line] = text[i];

		chars_in_line++;

		epiar_size_text(gui_font_normal, line_so_far, &text_w, &text_h, &text_base);
		if (text_w >= w) {
			memset(line_so_far, 0, sizeof(char) * 80);
			wrapped[lbreak] = 1;
			i = lbreak + 1;
			chars_in_line = 0;
		}
	}

	return (wrapped);
}

int gui_textbox_change_text(gui_textbox *tb, char *text) {
	char *new_text = NULL;

	if (tb == NULL)
		return (-1);
	if (text == NULL)
		return (-1);

	new_text = wrap_text(text, tb->w - 30);
	if (new_text == NULL)
		return (-1);

	free(tb->text);
	tb->text = new_text;

	tb->sb->progress = 0.0f;
	tb->sb->update = 1;

	return (0);
}

/* calculates, at the given position in the text, i, how many lines that would make for the textbox (to avoid the one line of text scroll issue) */
int calculate_drawn_lines(gui_textbox *textbox, int i) {
	char buf[80] = {0};
	int chars_in_line = 0;
	int lines_drawn = 0;

	for (; i < (signed)strlen(textbox->text); i++) {
		if ((lines_drawn * 18) > (textbox->h - 2))
			break;
		
		if (textbox->text[i] == 1) {
			chars_in_line = 0;
			memset(buf, 0, sizeof(char) * 80);
			lines_drawn++;
		} else {
			buf[chars_in_line] = textbox->text[i];
			chars_in_line++;
		}
	}

	return (lines_drawn);
}

int one_line_length(gui_textbox *textbox, int i) {
	int chars_in_line = 0;
	unsigned char passed_one = 0;

	if (i <= 0)
		return (0);

	for (; i >= 0; i--) {
		if (textbox->text[i] == 1) {
			if (passed_one)
				return (chars_in_line);
			else
				passed_one = 1;
		} else {
			chars_in_line++;
		}
	}

	return (chars_in_line);
}
