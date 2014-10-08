#include "gui/gui.h"
#include "system/video/video.h"

gui_session *gui_create_session(void) {
	gui_session *session = (gui_session *)malloc(sizeof(gui_session));

	if (!session)
		return (NULL);

	/* set default session values */
	session->active = 1;
	session->keyboard_focus = -1;
	session->mouse_down = 0;
	memset(session->children, 0, sizeof(void *) * MAX_SESSION_CHILDREN);
	memset(session->child_type, 0, sizeof(enum CHILD_TYPE) * MAX_SESSION_CHILDREN);

	return (session);
}

int gui_destroy_session(gui_session *session) {
	if (session == NULL)
		return (-1);

	free(session);

	return (0);
}

/* displays all session children */
void gui_session_show_all(gui_session *session) {
	int i;

	if (session == NULL)
		return;

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type != GUI_NONE) {
			/* there's a registered widget in this slot, so draw it */
			switch(session->child_type[i]) {
				case GUI_WINDOW:
				{
					gui_window *window;

					window = (gui_window *)session->children[i];
					gui_show_window(window);

					break;
				}
				case GUI_FRAME:
				{
					gui_frame *frame;

					frame = (gui_frame *)session->children[i];
					gui_show_frame(frame);

					break;
				}
				case GUI_TEXT_ENTRY:
				{
					gui_text_entry *text_entry;

					text_entry = (gui_text_entry *)session->children[i];
					gui_show_text_entry(text_entry);

					break;
				}
				case GUI_BUTTON:
				{
					gui_button *btn;

					btn = (gui_button *)session->children[i];
					gui_show_button(btn);

					break;
				}
				case GUI_IMAGE:
				{
					gui_image *image;

					image = (gui_image *)session->children[i];
					gui_show_image(image);

					break;
				}
				case GUI_CHECKBOX:
				{
					gui_checkbox *checkbox;

					checkbox = (gui_checkbox *)session->children[i];
					gui_show_checkbox(checkbox);

					break;
				}
				case GUI_LABEL:
				{
					gui_label *label;

					label = (gui_label *)session->children[i];
					gui_show_label(label);

					break;
				}
				case GUI_TEXTBOX:
				{
					gui_textbox *tb;

					tb = (gui_textbox *)session->children[i];
					gui_show_textbox(tb);

					break;
				}
				case GUI_LISTBOX:
				{
					gui_listbox *lb;

					lb = (gui_listbox *)session->children[i];
					gui_show_listbox(lb);

					break;
				}
				case GUI_SCROLLBAR:
				{
					gui_scrollbar *sb;

					sb = (gui_scrollbar *)session->children[i];
					gui_show_scrollbar(sb);

					break;
				}
				case GUI_TAB:
				{
					gui_tab *tab;

					tab = (gui_tab *)session->children[i];
					gui_show_tab(tab);

					break;
				}
				case GUI_KEYBOX:
				{
					gui_keybox *kb;

					kb = (gui_keybox *)session->children[i];
					gui_show_keybox(kb);

					break;
				}
				case GUI_BTAB:
				{
					gui_btab *bt;

					bt = (gui_btab *)session->children[i];
					gui_show_btab(bt);

					break;
				}
				default:
					break;
			}
		}
	}

	/* ... and show it! */
	flip();
}

/* destroys all session children, but not the session itself */
int gui_session_destroy_all(gui_session *session) {
	int i;

	if (session == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] != GUI_NONE) {
			/* registered slot; destroy it */
			switch(session->child_type[i]) {
				case GUI_WINDOW:
				{
					gui_window *window;

					window = (gui_window *)session->children[i];
					gui_destroy_window(window);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_TEXT_ENTRY:
				{
					gui_text_entry *text_entry;

					text_entry = (gui_text_entry *)session->children[i];
					gui_destroy_text_entry(text_entry);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_BUTTON:
				{
					gui_button *btn;

					btn = (gui_button *)session->children[i];
					gui_destroy_button(btn);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_IMAGE:
				{
					gui_image *image;

					image = (gui_image *)session->children[i];
					gui_destroy_image(image);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_CHECKBOX:
				{
					gui_checkbox *checkbox;

					checkbox = (gui_checkbox *)session->children[i];
					gui_destroy_checkbox(checkbox);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_FRAME:
				{
					gui_frame *frame;

					frame = (gui_frame *)session->children[i];
					gui_destroy_frame(frame);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_LABEL:
				{
					gui_label *label;

					label = (gui_label *)session->children[i];
					gui_destroy_label(label);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_SCROLLBAR:
				{
					gui_scrollbar *sb;

					sb = (gui_scrollbar *)session->children[i];
					gui_destroy_scrollbar(sb);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_TEXTBOX:
				{
					gui_textbox *tb;

					tb = (gui_textbox *)session->children[i];
					gui_destroy_textbox(tb);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_TAB:
				{
					gui_tab *tab;

					tab = (gui_tab *)session->children[i];
					gui_destroy_tab(tab);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_KEYBOX:
				{
					gui_keybox *kb;

					kb = (gui_keybox *)session->children[i];
					gui_destroy_keybox(kb);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_LISTBOX:
				{
					gui_listbox *lb;

					lb = (gui_listbox *)session->children[i];
					gui_destroy_listbox(lb);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				case GUI_BTAB:
				{
					gui_btab *bt;

					bt = (gui_btab *)session->children[i];
					gui_destroy_btab(bt);
					session->child_type[i] = GUI_NONE;
					session->children[i] = NULL;

					break;
				}
				default:
					break;
			}
		}
	}

	return (0);
}

void gui_session_activate(gui_session *session) {
	if (session == NULL)
		return;

	session->active = 1;
}

void gui_session_pass_keyboard_input(SDL_Event *event, gui_session *session) {
	if (session == NULL)
		return;

	if (event == NULL)
		return;

	if (session->child_type[session->keyboard_focus] == GUI_NONE)
		return;

	switch(session->child_type[session->keyboard_focus]) {
		case GUI_TEXT_ENTRY:
		{
			gui_text_entry *text_entry;

			text_entry = (gui_text_entry *)session->children[session->keyboard_focus];

			if (!text_entry->visible)
				break;

			gui_text_entry_take_input(event, text_entry);

			break;
		}
		default:
			break;
	}
}

void gui_session_pass_mouse_input(SDL_Event *event, gui_session *session, Uint8 state) {
	int i;
	int x, y; /* coords of mouse */

	if (session == NULL)
		return;

	/* if they arent clicking the left button, who cares, however, if it was just released, we care */
	if ((event->button.state != SDL_BUTTON_LEFT) && !(state & GS_BTN_LEFT_RELEASED)) {
#ifndef NDEBUG
		printf("ignoring click\n");
#endif
		return; /* who cares if they didn't click */
	}

	if(event->type == SDL_MOUSEBUTTONDOWN)
		session->mouse_down = 1;
	else if(event->type == SDL_MOUSEBUTTONUP)
		session->mouse_down = 0;

	x = event->button.x;
	y = event->button.y;

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE)
			break; /* we always add at the beginning, so a none is the end of the list */

		if (session->child_type[i] == GUI_BUTTON) {
			/* check to see if this mouse event is in the button */
			gui_button *btn = (gui_button *)session->children[i];

			if (btn->visible) {
				if ((x >= btn->x) && (x <= (btn->x + btn->w))) {
					if ((y >= btn->y) && (y <= (btn->y + btn->h))) {
						/* the click was in this widget */
						if (btn->state & BTN_PRESSED) {
							btn->state ^= BTN_PRESSED; /* turn off pressed */
							btn->state |= BTN_REDRAW; /* request redraw */
							btn->update = 1;
							if (btn->callback)
								(btn->callback) ();
						} else {
							btn->state |= BTN_PRESSED;
							btn->state |= BTN_REDRAW;
							btn->update = 1;
						}
						return; /* only register click for one widget */
					}
				} else {
					/* they didnt click on the button */
					if ((btn->state & BTN_PRESSED) && (event->type == SDL_MOUSEBUTTONUP)) {
						/* a button is down, but the mouse was released not over the button, so depress it */
						btn->state ^= BTN_PRESSED;
						btn->state |= BTN_REDRAW;
						btn->update = 1;
					}
				}
			}
		}

		/* note that listbox checks doesnt check scrollbar, as that is already handled */
		if (session->child_type[i] == GUI_LISTBOX) {
			gui_listbox *lb = (gui_listbox *)session->children[i];

			if (lb->visible) {
				/* the minus 17 is b/c the scrollbar is drawn there */
				if ((x >= lb->x) && (x <= (lb->x + (lb->w - 17)))) {
					if ((y >= lb->y) && (y <= (lb->y + lb->h))) {
						/* click inside a visible listbox, so, what item did they click on? */
						int clicked_on = ((y - lb->y) / lb->item_h) + lb->top_scrolled;

						if ((clicked_on != lb->selected) && (clicked_on < lb->num_items)) {
							lb->selected = clicked_on;

							lb->update = 1;
						}
					}
				}
			}
		}

		/* check for btab clicks */
		if (session->child_type[i] == GUI_BTAB) {
			/* check to see if this mouse event is in the button */
			gui_btab *bt = (gui_btab *)session->children[i];

			if (bt->visible) {
				int a = 0;

				a = gui_btab_check_clicks(bt, x, y);

				bt->update = 1;

				if (a)
					return; /* gui_btab_check_clicks() returns true if its this and handles it, so, we only register one click */
			}
		}

		if (session->child_type[i] == GUI_TEXT_ENTRY) {
			/* check to see if this mouse event is in the button */
			gui_text_entry *te = (gui_text_entry *)session->children[i];

			if (te->visible) {
				if ((x >= te->x) && (x <= (te->x + te->w))) {
					if ((y >= te->y) && (y <= (te->y + te->h))) {
						gui_text_entry_set_focus(session, te);
						te->update = 1;
						return;
					}
				}
			}
		}

		if (session->child_type[i] == GUI_TAB) {
			/* check to see if this mouse event is in the button */
			gui_tab *tab = (gui_tab *)session->children[i];

			if (tab->visible) {
#ifndef WIN32
#warning ASSUMING NORMAL TABS ARE 22x51
#endif
				if ((x >= tab->x) && (x <= (tab->x + 22))) {
					if ((y >= tab->y) && (y <= (tab->y + (tab->num_tabs * 56)))) {
						/* the click was in this widget */
						/* figure out which tab was clicked */
						int selected = (y - tab->y) / 56;

						/* only do callback and all that if this tab isnt already selected */
						if (tab->selected != selected) {
							tab->selected = selected;
							tab->update = 1; /* redraw it */

							/* and if there's a frame associated, redraw that too */
							if (tab->associated_frame)
								tab->associated_frame->update = 1;

							if (tab->callback)
								(*tab->callback) (tab->selected); /* do callback if one is set */
						}

						return; /* only register click for one widget */
					}
				}
			}
		}

		if (session->child_type[i] == GUI_SCROLLBAR) {
			/* check to see if this mouse event is on the scrollbar */
			gui_scrollbar *sb = (gui_scrollbar *)session->children[i];

			if (sb->visible) {
#ifndef WIN32
#warning SCROLLBAR WIDTH HARD CODED TO 17 PIXELS AND ARROWS ASSUMED TO BE 17x17
#endif
				if ((x >= sb->x) && (x <= (sb->x + 17))) {
					if ((y >= sb->y) && (y <= (sb->y + 17))) {
						/* clicked the up arrow */
#ifndef WIN32
#warning PROGRESS CHANGE HARD CODED TO 5% PER CLICK
#endif
						sb->progress -= 0.05f;
						if (sb->progress < 0.0f)
							sb->progress = 0.0f;
						sb->update = 1;
					}
					if ((y >= (sb->y + sb->h - 17)) && (y <= (sb->y + sb->h))) {
						/* clicked the down arrow */
#ifndef WIN32
#warning PROGRESS CHANGE HARD CODED TO 5% PER CLICK
#endif
						sb->progress += 0.05f;
						if (sb->progress > 1.0f)
							sb->progress = 1.0f;
						sb->update = 1;
					}
				}

				if (sb->update) {
					if (sb->buddy) {
						if (sb->buddy_type == GUI_TEXTBOX) {
							gui_textbox *tb = (gui_textbox *)sb->buddy;
							tb->update = 1;
						}
						if (sb->buddy_type == GUI_LISTBOX) {
							gui_listbox *lb = (gui_listbox *)sb->buddy;
							lb->update = 1;
						}
					}
				}
			}
		}
		if (state & GS_BTN_LEFT_RELEASED) {
			if (session->child_type[i] == GUI_CHECKBOX) {
				/* check to see if this mouse event is in the button */
				gui_checkbox *checkbox = (gui_checkbox *)session->children[i];

				if (checkbox->visible) {
#ifndef WIN32
#warning ASSUMING CHECKBOX IMAGE IS 11x12
#endif
					if ((x >= checkbox->x) && (x <= (checkbox->x + 11))) {
						if ((y >= checkbox->y) && (y <= (checkbox->y + 12))) {
							/* the click was in this widget */
							checkbox->checked = (checkbox->checked + 1) % 2; /* toggle checkbox */
							checkbox->update = 1;

							if (checkbox->callback)
								(*checkbox->callback) ();

							return; /* only register click for one widget */
						}
					}
				}
			}
			if (session->child_type[i] == GUI_KEYBOX) {
				/* check to see if this mouse event is on the scrollbar */
				gui_keybox *kb = (gui_keybox *)session->children[i];

				if (kb->visible) {
					if ((x >= kb->x) && (x <= (kb->x + 50))) {
						if ((y >= kb->y) && (y <= (kb->y + 16))) {
							/* they clicked the transparent box area */
							/* find out what key they want */
							*kb->associated_key = gui_keybox_prompt_new_key();

							/* redraw the screen to get rid of that box */
							gui_session_show_all(session);
							flip();

							return; /* dont take multiple clicks since this one was handled */
						}
					}
				}
			}
		}
	}
}

void gui_session_update(gui_session *session) {
	int i;
	unsigned char update = 0;

	if (session == NULL)
		return;

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] != GUI_NONE) {
			switch(session->child_type[i]) {
				case GUI_TEXT_ENTRY:
				{
					gui_text_entry *text_entry;

					text_entry = (gui_text_entry *)session->children[i];

					if (!text_entry->visible)
						break;

					if (text_entry->update) {
						gui_show_text_entry(text_entry);
						text_entry->update = 0;
						update = 1;
					}

					break;
				}
				case GUI_FRAME:
				{
					gui_frame *frame;

					frame = (gui_frame *)session->children[i];

					if (!frame->visible)
						break;

					if (frame->update) {
						gui_show_frame(frame);
						frame->update = 0;
						update = 1;
					}

					break;
				}
				case GUI_BUTTON:
				{
					gui_button *btn;

					btn = (gui_button *)session->children[i];

					if (!btn->visible)
						break;

					if (btn->update) {
						gui_update_button(btn); /* gui_update_button() is responsible for turning off the update on the button */
						update = 1;
					}

					break;
				}
				case GUI_CHECKBOX:
				{
					gui_checkbox *checkbox;

					checkbox = (gui_checkbox *)session->children[i];

					if (!checkbox->visible)
						break;

					if (checkbox->update) {
						gui_show_checkbox(checkbox);
						checkbox->update = 0;
						update = 1;
					}

					break;
				}
				case GUI_SCROLLBAR:
				{
					gui_scrollbar *sb;

					sb = (gui_scrollbar *)session->children[i];

					if (!sb->visible)
						break;

					if (sb->update) {
						gui_show_scrollbar(sb);
						sb->update = 0;
						update = 1;
					}

					break;
				}
				case GUI_TEXTBOX:
				{
					gui_textbox *tb;

					tb = (gui_textbox *)session->children[i];

					if (!tb->visible)
						break;

					if (tb->update) {
						gui_show_textbox(tb);
						tb->update = 0;
						update = 1;
					}

					break;
				}
				case GUI_TAB:
				{
					gui_tab *tab;

					tab = (gui_tab *)session->children[i];

					if (!tab->visible)
						break;

					if (tab->update) {
						gui_show_tab(tab);
						tab->update = 0;
						update = 1;
					}

					break;
				}
				case GUI_KEYBOX:
				{
					gui_keybox *kb;

					kb = (gui_keybox *)session->children[i];

					if (!kb->visible)
						break;

					if (kb->update) {
						gui_show_keybox(kb);
						kb->update = 0;
						update = 1;
					}

					break;
				}
				case GUI_LISTBOX:
				{
					gui_listbox *lb;

					lb = (gui_listbox *)session->children[i];

					if (!lb->visible)
						break;

					if (lb->update) {
						gui_show_listbox(lb);
						lb->update = 0;
						update = 1;
					}

					break;
				}
				case GUI_BTAB:
				{
					gui_btab *bt;

					bt = (gui_btab *)session->children[i];

					if (!bt->visible)
						break;

					if (bt->update) {
						gui_show_btab(bt);
						bt->update = 0;
						update = 1;
					}

					break;
				}
				default:
					break;
			}
		}
	}

	if (update)
		flip();
}
