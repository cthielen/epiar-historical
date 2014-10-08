#include "gui.h"
#include "includes.h"
#include "system/path.h"
#include "system/video/video.h"

static int gui_btab_register_session(gui_session *session, gui_btab *bt);

gui_btab *gui_create_btab(int x, int y, gui_session *session) {
	gui_btab *bt = (gui_btab *)malloc(sizeof(gui_btab));

	assert(bt);

	bt->x = x;
	bt->y = y;
	bt->update = 0;
	bt->visible = 1;
	bt->n_items = 0;
	bt->selected = -1;
	bt->bts_off = NULL;
	bt->bts_on = NULL;

	if (gui_btab_register_session(session, bt) != 0) {
		free(bt);
		return (NULL);
	}

	return (bt);
}

/* returns -1 on failure, else, the number slot the tab is */
int gui_btab_add_tab(gui_btab *bt, char *file_off, char *file_on) {
	SDL_Surface *s_off = NULL, *s_on = NULL, **list_on = NULL, **list_off = NULL;
	int i;

	if (bt == NULL)
		return (-1);
	if (file_off == NULL)
		return (-1);
	/* file_on can be null, dont check against that */
#ifndef WIN32
#warning btab does apply_game_path() on its own, dont call it in the parameters
#endif
	/* load the images */
	s_off = load_image(apply_game_path(file_off), ALPHA);
	assert(s_off);

	/* you do not need an on image, but no callback for static, non-changing images */
	if (file_on != NULL) {
		s_on = load_image(apply_game_path(file_on), ALPHA);
		assert(s_on);
	}

	/* create the new lists of surfaces */
	list_off = (SDL_Surface **)calloc(bt->n_items + 1, sizeof(SDL_Surface *));
	assert(list_off);
	list_on = (SDL_Surface **)calloc(bt->n_items + 1, sizeof(SDL_Surface *));
	assert(list_on);

	/* copy the older lists to the newer lists */
	for (i = 0; i < bt->n_items; i++) {
		list_off[i] = bt->bts_off[i];
		list_on[i] = bt->bts_on[i];
	}

	/* and copy the new one (SDL_Surface *s) */
	list_off[bt->n_items] = s_off;
	list_on[bt->n_items] = s_on;

	/* delete the old list */
	free(bt->bts_off);
	free(bt->bts_on);

	/* set the new list */
	bt->bts_off = list_off;
	bt->bts_on = list_on;

	bt->n_items++; /* increment the number of items */

	if  (bt->n_items == 1)
	  bt->selected = 1; /* ensure that a btab w/ one item becomes default item */

	return (bt->n_items);
}

/* returns -1 on failure, else, the number slot the tab is */
int gui_btab_add_tab_eaf(FILE *eaf, gui_btab *bt, char *file_off, char *file_on) {
	SDL_Surface *s_off = NULL, *s_on = NULL, **list_on = NULL, **list_off = NULL;
	int i;

	if (bt == NULL)
		return (-1);
	if (file_off == NULL)
		return (-1);
	/* file_on can be null, dont check against that */

	/* load the images */
	s_off = load_image_eaf(eaf, file_off, ALPHA);
	assert(s_off);

	/* you do not need an on image, but no callback for static, non-changing images */
	if (file_on != NULL) {
		s_on = load_image_eaf(eaf, file_on, ALPHA);
		assert(s_on);
	}

	/* create the new lists of surfaces */
	list_off = (SDL_Surface **)calloc(bt->n_items + 1, sizeof(SDL_Surface *));
	assert(list_off);
	list_on = (SDL_Surface **)calloc(bt->n_items + 1, sizeof(SDL_Surface *));
	assert(list_on);

	/* copy the older lists to the newer lists */
	for (i = 0; i < bt->n_items; i++) {
		list_off[i] = bt->bts_off[i];
		list_on[i] = bt->bts_on[i];
	}

	/* and copy the new one (SDL_Surface *s) */
	list_off[bt->n_items] = s_off;
	list_on[bt->n_items] = s_on;

	/* delete the old list */
	free(bt->bts_off);
	free(bt->bts_on);

	/* set the new list */
	bt->bts_off = list_off;
	bt->bts_on = list_on;

	bt->n_items++; /* increment the number of items */

	if (bt->n_items == 1)
	  bt->selected = 1; /* ensure that the first item added is the default item */

	return (bt->n_items);
}

int gui_destroy_btab(gui_btab *bt) {
	int i;

	if (bt == NULL)
		return (-1);

	/* free all tabs */
	for (i = 0; i < bt->n_items; i++) {
		SDL_FreeSurface(bt->bts_off[i]);
		/* be careful, not all tabs are required to have on states */
		if (bt->bts_on[i] != NULL)
			SDL_FreeSurface(bt->bts_on[i]);
	}

	free(bt);

	return (0);
}

void gui_show_btab(gui_btab *bt) {
	int i, width = 0;
	SDL_Rect rect;

	if (bt == NULL)
		return;

	for (i = 0; i < bt->n_items; i++) {
		rect.x = bt->x + width;
		rect.y = bt->y;
		rect.w = bt->bts_off[i]->w; /* off and on should have same w/h, so they are interchangable */
		rect.h = bt->bts_off[i]->h;

		if (bt->selected == i)
			blit_surface(bt->bts_on[i], NULL, &rect, 0);
		else
			blit_surface(bt->bts_off[i], NULL, &rect, 0);

		width += rect.w;
	}
}

int gui_init_btab(void) {
	return (0);
}

int gui_quit_btab(void) {
	return (0);
}

static int gui_btab_register_session(gui_session *session, gui_btab *bt) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (bt == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = bt;
	session->child_type[slot] = GUI_BTAB;

	return (0);
}

/* returns element  if the x,y is in the tab */
int gui_btab_check_clicks(gui_btab *bt, int x, int y) {
	int i;
	int width = 0;

	if (bt == NULL)
		return (0);

	if (x < bt->x)
		return (0); /* couldnt be our btab */
	if (y < bt->y)
		return (0);

	if (bt->n_items == 0)
		return (0);

	/* notice off and on are not differentiated - there should be no diff in the w/h */

	if (y > (bt->y + bt->bts_off[0]->h)) /* all heights should really be the same for btabs */
		return (0);

	for (i = 0; i < bt->n_items; i++) {
		width += bt->bts_off[i]->w;

		if (x < (bt->x + width)) {
			/* if it's already selected, dont select it again */
			if (bt->selected != i) {
				if (bt->bts_on[i] != NULL) {
					bt->selected = i; /* this can only become the current selected if it is clickable, i.e. has on and off states */
					/* do the callback if one */
					if (bt->callback)
						(*bt->callback) (i);
				}
				return (i);
			} else {
				return (0); /* must return, this is the tab they clicked on, checking the next tab, due to the math, would result in "that tab being clicked", which is incorrect */
			}
		}
	}

	return (0);
}

int gui_btab_set_callback(gui_btab *bt, void (*callback) (int which)) {
	if (bt == NULL)
		return (-1);
	if (callback == NULL)
		return (-1);

	bt->callback = callback;

	return (0);
}
