#include "gui/gui.h"
#include "system/video/video.h"

static int gui_listbox_register_session(gui_session *session, gui_listbox *lb);

gui_listbox *gui_create_listbox(int x, int y, int w, int h, int item_h, gui_session *session) {
	gui_listbox *lb = (gui_listbox *)malloc(sizeof(gui_listbox));
	
	assert(lb);
	
	lb->x = x;
	lb->y = y;
	lb->w = w;
	lb->h = h;
	lb->item_h = item_h;
	lb->selected = -1;
	lb->num_items = 0;
	lb->visible = 1;
	lb->update = 0;
	lb->trans_area = NULL;
	lb->callback = NULL;
	lb->top_scrolled = 0;
	
	if (gui_listbox_register_session(session, lb) != 0) {
		free(lb);
		return (NULL);
	}
	
	/* allocate the scrollbar after we know it's registered in the session to avoid deleting scrollbar messes */
#ifndef WIN32
#warning scrollbar width assumed to be 17 pixels
#endif
	lb->sb = gui_create_scrollbar(lb->x + lb->w - 17, lb->y, lb->h, 0.0f, session);
	gui_scrollbar_set_buddy(lb->sb, lb, GUI_LISTBOX);
	
	return (lb);
}

int gui_listbox_set_callback(gui_listbox *lb, void (*callback) (void *lb, int x, int y, int w, int h, int item)) {
	if (lb == NULL)
		return (-1);
	if (callback == NULL)
		return (-1);
	
	lb->callback = callback;
	
	return (0);
}

int gui_destroy_listbox(gui_listbox *listbox) {
	if (listbox == NULL)
		return (-1);
	
	if (listbox->trans_area)
		SDL_FreeSurface(listbox->trans_area);
	
	/* notice we don't destroy the scrollbar. gui_session_destroy_all() will handle that for us */
	free(listbox);
	
	return (0);
}

void gui_show_listbox(gui_listbox *lb) {
	int i, item;
	SDL_Rect rect;
	
	if (lb == NULL)
		return;
	
	if (!lb->visible)
		return;
	
	/* draw the under area first */
	if (lb->trans_area == NULL) {
		SDL_Surface *area;
		SDL_Rect src, dest;
		
		area = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, lb->w - 17, lb->h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
		assert(area);
		rect.x = 0;
		rect.y = 0;
		rect.w = area->w;
		rect.h = area->h;
		SDL_FillRect(area, &rect, SDL_MapRGB(area->format, 0, 0, 0));
		SDL_SetAlpha(area, SDL_SRCALPHA, 93);
		
		rect.x = lb->x;
		rect.y = lb->y;
		rect.w = area->w;
		rect.h = area->h;
		blit_surface(area, NULL, &rect, 0);
		
		SDL_FreeSurface(area);
		
		/* now that the trans area is drawn, back up that area there */
		lb->trans_area = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, lb->w - 17, lb->h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
		src.x = lb->x;
		src.y = lb->y;
		src.w = lb->trans_area->w;
		src.h = lb->trans_area->h;
		dest.x = 0;
		dest.y = 0;
		dest.w = src.w;
		dest.h = src.h;
		
		SDL_BlitSurface(screen, &src, lb->trans_area, &dest);
	} else {
		rect.x = lb->x;
		rect.y = lb->y;
		rect.w = lb->trans_area->w;
		rect.h = lb->trans_area->h;
		
		blit_surface(lb->trans_area, NULL, &rect, 0);
	}
	
	if (lb->num_items == 0)
		return;
	
	/* now draw the items */
	/* set "item" to the first item for the top, as dedicated by the progress bar */
	item = (int)((float)lb->sb->progress * (float)(lb->num_items - 1));
	lb->top_scrolled = item;
	
	/* draw the top bar */
	rect.x = lb->x;
	rect.y = lb->y;
	rect.w = lb->w - 17;
	rect.h = 2;
	fill_rect(&rect, map_rgb(0, 93, 92));
	
	/* draw all the item borders and call the callback (if one exists) to draw what's inside each element */
	for (i = 0; (item < lb->num_items) && ((i * lb->item_h) < lb->h); i++) {
		if ((i + lb->top_scrolled) == lb->selected) {
			/* draw the selected bg */
			rect.x = lb->x;
			rect.y = lb->y + (i * lb->item_h) + 2;
			rect.w = lb->w - 17;
			rect.h = lb->item_h - 2;
			if ((rect.y + rect.h) > (lb->y + lb->h))
				rect.h = (lb->y + lb->h) - rect.y;
			
			fill_rect(&rect, map_rgb(0, 64, 65));
		}
		/* draw the bottom item bar */
		rect.x = lb->x;
		rect.y = lb->y + (i * lb->item_h) + lb->item_h;
		rect.w = lb->w - 17;
		rect.h = 2;
		if (rect.y < (lb->y + lb->h))
			fill_rect(&rect, map_rgb(0, 93, 92));
		
		if (lb->callback)
			(*lb->callback) (lb, rect.x, rect.y - lb->item_h, rect.w, lb->item_h, item);
		
		item++;
	}
}

int gui_init_listbox(void) {
	return (0); /* nothing to init */
}

int gui_quit_listbox(void) {
	return (0); /* nothing to quit */
}

/* merely increases the total number of items */
int gui_listbox_add_item(gui_listbox *lb) {
	if (lb == NULL)
		return (-1);
	
	lb->num_items++;

	/* auto-select the first item */
	if (lb->num_items == 1)
		lb->selected = 0;
	
	return (0);
}

static int gui_listbox_register_session(gui_session *session, gui_listbox *lb) {
	int i, slot = -1;
	
	if (session == NULL)
		return (-1);
	if (lb == NULL)
		return (-1);
	
	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}
	
	if (slot == -1)
		return (-1); /* session full */
	
	session->children[slot] = lb;
	session->child_type[slot] = GUI_LISTBOX;
	
	return (0);
}
