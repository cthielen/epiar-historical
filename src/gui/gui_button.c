#include "com_defs.h"
#include "gui/gui.h"
#include "includes.h"
#include "system/font.h"
#include "system/video/video.h"

static int gui_button_register_session(gui_session *session, gui_button *button);
static SDL_Surface *d_b1, *d_b2, *d_b3, *d_b4, *d_b5, *d_b6, *d_b7, *d_b8, *d_background;
static SDL_Surface *p_b1, *p_b2, *p_b3, *p_b4, *p_b5, *p_b6, *p_b7, *p_b8, *p_background;

/* image naming explaination */
/* p = pressed */
/* d = depressed */
/* 1 --- 2 --- 3 */
/* 4 -- BACK-- 5 */
/* 6 --- 7 --- 8 */

gui_button *gui_create_button(short int x, short int y, short int w, short int h, char *label, gui_session *session) {
	gui_button *btn = (gui_button *)malloc(sizeof(gui_button));

	if (label == NULL)
		return (NULL);
	if (strlen(label) <= 0)
		return (NULL);
	if (session == NULL)
		return (NULL);

	/* set dimensions */
	btn->x = x;
	btn->y = y;
	btn->w = w;
	btn->h = h;
	btn->visible = 1;
	btn->update = 0;
	btn->state = 0;
	btn->callback = NULL;

	/* set label */
	btn->label = (char *)malloc(sizeof(char) * (strlen(label) + 1));
	memset(btn->label, 0, sizeof(char) * (strlen(label) + 1));
	strcpy(btn->label, label);

	if (gui_button_register_session(session, btn) != 0) {
		free(btn->label);
		free(btn);
		return (NULL);
	}

	return (btn);
}

int gui_destroy_button(gui_button *button) {
	if (button == NULL)
		return (-1);

	free(button->label);
	free(button);

	return (0);
}

void gui_show_button(gui_button *button) {
	SDL_Rect src, rect;
	SDL_Surface *upper_left, *top_border, *upper_right, *left_border, *right_border, *lower_left, *bottom_border, *lower_right, *background;
	int i, j, w, h, base;
	int ver_iterations_needed, ver_pixels_needed, hor_iterations_needed, hor_pixels_needed;
	int text_x, text_y;

	assert(button);

	if (!button->visible)
		return;

	if (button->state & BTN_PRESSED) {
		/* make them looked pressed */
		upper_left = p_b1;
		top_border = p_b2;
		upper_right = p_b3;
		left_border = p_b4;
		right_border = p_b5;
		lower_left = p_b6;
		bottom_border = p_b7;
		lower_right = p_b8;
		background = p_background;
	} else {
		/* make them look depressed */
		upper_left = d_b1;
		top_border = d_b2;
		upper_right = d_b3;
		left_border = d_b4;
		right_border = d_b5;
		lower_left = d_b6;
		bottom_border = d_b7;
		lower_right = d_b8;
		background = d_background;
	}

	/* render background */
	src.x = 0;
	src.y = 0;
	src.w = background->w;
	src.h = background->h;

	hor_pixels_needed = button->w - upper_left->w - upper_right->w;
	hor_iterations_needed = hor_pixels_needed / background->w;
	hor_pixels_needed -= (hor_iterations_needed * background->w);

	ver_pixels_needed = button->h - upper_left->h - lower_left->h;
	ver_iterations_needed = ver_pixels_needed / background->h;
	ver_pixels_needed -= (ver_iterations_needed * background->h);

	for (j = 0; j < ver_iterations_needed; j++) {
		for (i = 0; i < hor_iterations_needed; i++) {
			rect.x = button->x + upper_left->w;
			rect.y = button->y + upper_left->h;
			rect.w = button->w - upper_left->w - upper_right->w;
			rect.h = button->h - lower_left->h - upper_left->h;

			rect.x += (i * background->w);
			rect.y += (j * background->h);

			blit_surface(background, &src, &rect, 0);
		}
	}

	/* draw any left over background (the extra pixels_needed) */
	for (j = 0; j < ver_iterations_needed; j++) {
		src.w = hor_pixels_needed;
		rect.x = button->x + upper_left->w;
		rect.y = button->y + upper_left->h;
		rect.w = hor_pixels_needed;
		rect.h = background->h;

		rect.x += (hor_iterations_needed * background->w);
		rect.y += (j * background->h);

		blit_surface(background, &src, &rect, 1);
	}
	for (i = 0; i < hor_iterations_needed; i++) {
		src.w = background->w;
		src.h = ver_pixels_needed;
		rect.x = button->x + upper_left->w;
		rect.y = button->y + upper_left->h;
		rect.w = background->w;
		rect.h = ver_pixels_needed;

		rect.x += (i * background->w);
		rect.y += (ver_iterations_needed * background->h);

		blit_surface(background, &src, &rect, 1);
	}

	/* finish that last square */
	src.w = hor_pixels_needed;
	src.h = ver_pixels_needed;
	rect.x = button->x + upper_left->w;
	rect.y = button->y + upper_left->h;
	rect.w = hor_pixels_needed;
	rect.h = ver_pixels_needed;
	rect.x += (hor_iterations_needed * background->w);
	rect.y += (ver_iterations_needed * background->h);
	blit_surface(background, &src, &rect, 1);

	/* render upper left corner */
	rect.x = button->x;
	rect.y = button->y;
	rect.w = upper_left->w;
	rect.h = upper_left->h;

	blit_surface(upper_left, NULL, &rect, 1);

	/* render upper border, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = button->w - upper_left->w - upper_right->w;

		iterations_needed = pixels_needed / top_border->w;

		pixels_needed -= (iterations_needed * top_border->w);

		src.x = 0;
		src.y = 0;
		src.w = top_border->w;
		src.h = top_border->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = button->x + upper_left->w;
			rect.y = button->y;
			rect.w = top_border->w;
			rect.h = top_border->h;

			rect.x += (i * top_border->w);

			blit_surface(top_border, &src, &rect, 1);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.w = pixels_needed;
			rect.x = button->x + upper_left->w;
			rect.y = button->y;
			rect.w = pixels_needed;
			rect.h = top_border->h;

			rect.x += (i * top_border->w);

			blit_surface(top_border, &src, &rect, 1);
		}
	}

	/* render upper right corner */
	rect.x = button->x + button->w - upper_right->w;
	rect.y = button->y;
	rect.w = upper_right->w;
	rect.h = upper_right->h;

	blit_surface(upper_right, NULL, &rect, 1);

	/* render left border, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = button->h - upper_left->h - lower_left->h;

		iterations_needed = pixels_needed / left_border->h;

		pixels_needed -= (iterations_needed * left_border->h);

		src.x = 0;
		src.y = 0;
		src.w = left_border->w;
		src.h = left_border->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = button->x;
			rect.y = button->y + upper_left->h;
			rect.w = left_border->w;
			rect.h = left_border->h;

			rect.y += (i * left_border->h);

			blit_surface(left_border, &src, &rect, 1);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.h = pixels_needed;
			rect.x = button->x;
			rect.y = button->y + upper_left->h;
			rect.w = left_border->w;
			rect.h = pixels_needed;

			rect.y += (i * left_border->h);

			blit_surface(left_border, &src, &rect, 1);
		}
	}

	/* render right border, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = button->h - upper_right->h - lower_right->h;

		iterations_needed = pixels_needed / right_border->h;

		pixels_needed -= (iterations_needed * right_border->h);

		src.x = 0;
		src.y = 0;
		src.w = right_border->w;
		src.h = right_border->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = button->x + button->w - right_border->w;
			rect.y = button->y + upper_right->h;
			rect.w = right_border->w;
			rect.h = right_border->h;

			rect.y += (i * right_border->h);

			blit_surface(right_border, &src, &rect, 1);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.h = pixels_needed;
			rect.x = button->x + button->w - right_border->w;
			rect.y = button->y + upper_left->h;
			rect.w = right_border->w;
			rect.h = pixels_needed;

			rect.y += (i * right_border->h);

			blit_surface(right_border, &src, &rect, 1);
		}
	}

	/* render lower left corner */
	rect.x = button->x;
	rect.y = button->y + button->h - lower_left->h;
	rect.w = lower_left->w;
	rect.h = lower_left->h;

	blit_surface(lower_left, NULL, &rect, 1);

	/* render lower border, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = button->w - lower_left->w - lower_right->w;

		iterations_needed = pixels_needed / bottom_border->w;

		pixels_needed -= (iterations_needed * bottom_border->w);

		src.x = 0;
		src.y = 0;
		src.w = bottom_border->w;
		src.h = bottom_border->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = button->x + lower_left->w;
			rect.y = button->y + button->h - bottom_border->h;
			rect.w = bottom_border->w;
			rect.h = bottom_border->h;

			rect.x += (i * bottom_border->w);

			blit_surface(bottom_border, &src, &rect, 1);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.w = pixels_needed;
			rect.x = button->x + upper_left->w;
			rect.y = button->y + button->h - bottom_border->h;
			rect.w = pixels_needed;
			rect.h = bottom_border->h;

			rect.x += (i * bottom_border->w);

			blit_surface(bottom_border, &src, &rect, 1);
		}
	}

	/* render lower right corner */
	rect.x = button->x + button->w - lower_right->w;
	rect.y = button->y + button->h - lower_right->h - 1; /* -1 ? it was off .... ? */
	rect.w = lower_right->w;
	rect.h = lower_right->h;

	blit_surface(lower_right, NULL, &rect, 1);

	/* finally, render the text */
	epiar_size_text(gui_font_normal, button->label, &w, &h, &base);
	text_x = button->x + (button->w / 2) - (w/2);
	text_y = button->y + (button->h / 2) - (h/2) + base;
	if (button->state & BTN_PRESSED) {
		text_x += 2;
		text_y += 2;
	}
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, text_x, text_y, button->label);
}

int gui_init_button(void) {

	/* load the depressed images */
	d_b1 = load_image_eaf(epiar_eaf, "gui/btn_dp_ul.png", 0);
	d_b2 = load_image_eaf(epiar_eaf, "gui/btn_dp_tb.png", 0);
	d_b3 = load_image_eaf(epiar_eaf, "gui/btn_dp_ur.png", 0);
	d_b4 = load_image_eaf(epiar_eaf, "gui/btn_dp_lb.png", 0);
	d_b5 = load_image_eaf(epiar_eaf, "gui/btn_dp_rb.png", 0);
	d_b6 = load_image_eaf(epiar_eaf, "gui/btn_dp_ll.png", 0);
	d_b7 = load_image_eaf(epiar_eaf, "gui/btn_dp_bb.png", 0);
	d_b8 = load_image_eaf(epiar_eaf, "gui/btn_dp_lr.png", 0);
	d_background = load_image_eaf(epiar_eaf, "gui/btn_dp_bg.png", 0);

	/* load the pressed images */
	p_b1 = load_image_eaf(epiar_eaf, "gui/btn_p_ul.png", 0);
	p_b2 = load_image_eaf(epiar_eaf, "gui/btn_p_tb.png", 0);
	p_b3 = load_image_eaf(epiar_eaf, "gui/btn_p_ur.png", 0);
	p_b4 = load_image_eaf(epiar_eaf, "gui/btn_p_lb.png", 0);
	p_b5 = load_image_eaf(epiar_eaf, "gui/btn_p_rb.png", 0);
	p_b6 = load_image_eaf(epiar_eaf, "gui/btn_p_ll.png", 0);
	p_b7 = load_image_eaf(epiar_eaf, "gui/btn_p_bb.png", 0);
	p_b8 = load_image_eaf(epiar_eaf, "gui/btn_p_lr.png", 0);
	p_background = load_image_eaf(epiar_eaf, "gui/btn_p_bg.png", 0);

	return (0);	
}

int gui_quit_button(void) {

	SDL_FreeSurface(d_b1);
	SDL_FreeSurface(d_b2);
	SDL_FreeSurface(d_b3);
	SDL_FreeSurface(d_b4);
	SDL_FreeSurface(d_b5);
	SDL_FreeSurface(d_b6);
	SDL_FreeSurface(d_b7);
	SDL_FreeSurface(d_b8);
	SDL_FreeSurface(d_background);
	SDL_FreeSurface(p_b1);
	SDL_FreeSurface(p_b2);
	SDL_FreeSurface(p_b3);
	SDL_FreeSurface(p_b4);
	SDL_FreeSurface(p_b5);
	SDL_FreeSurface(p_b6);
	SDL_FreeSurface(p_b7);
	SDL_FreeSurface(p_b8);
	SDL_FreeSurface(p_background);

	return (0);
}

/* registers widget with given session */
static int gui_button_register_session(gui_session *session, gui_button *button) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (button == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1);

	session->children[slot] = button;
	session->child_type[slot] = GUI_BUTTON;

	return (0);
}

void gui_update_button(gui_button *button) {
	if (button == NULL)
		return;

	if (!button->visible)
		return;

	if (button->state & BTN_REDRAW) {
		button->state ^= BTN_REDRAW; /* turn off redraw */
		gui_show_button(button); /* do the redraw */
	}
}

void gui_button_set_callback(gui_button *button, void (*callback) (void)) {
	if (button == NULL)
		return;
	if (callback == NULL)
		return;

	button->callback = callback;
}
