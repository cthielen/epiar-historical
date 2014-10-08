#include "com_defs.h"
#include "gui/gui.h"
#include "system/video/video.h"

static SDL_Surface *b1, *b2, *b3, *b4, *b5, *b6, *b7, *b8, *background;

static int gui_frame_register_session(gui_session *session, gui_frame *frame);

gui_frame *gui_create_frame(int x, int y, int w, int h, gui_session *session) {
	gui_frame *frame = (gui_frame *)malloc(sizeof(gui_frame));

	frame->x = x;
	frame->y = y;
	frame->w = w;
	frame->h = h;
	frame->visible = 1;
	frame->update = 0;
	frame->dc = NULL;

	if (gui_frame_register_session(session, frame) != 0) {
		free(frame);
		return (NULL);
	}

	return (frame);
}

int gui_destroy_frame(gui_frame *frame) {

	free(frame);

	return (0);
}

void gui_show_frame(gui_frame *frame) {
	SDL_Rect src, rect;
	int i, j, hor_pixels_needed, hor_iterations_needed, ver_pixels_needed, ver_iterations_needed;

	assert(frame);

	if (!frame->visible)
		return;

	/* render background */
	src.x = 0;
	src.y = 0;
	src.w = background->w;
	src.h = background->h;

	hor_pixels_needed = frame->w - b1->w - b3->w;
	hor_iterations_needed = hor_pixels_needed / background->w;
	hor_pixels_needed -= (hor_iterations_needed * background->w);

	ver_pixels_needed = frame->h - b1->h - b6->h;
	ver_iterations_needed = ver_pixels_needed / background->h;
	ver_pixels_needed -= (ver_iterations_needed * background->h);

	for (j = 0; j < ver_iterations_needed; j++) {
		for (i = 0; i < hor_iterations_needed; i++) {
			rect.x = frame->x + b1->w;
			rect.y = frame->y + b1->h;
			rect.w = frame->w - b1->w - b3->w;
			rect.h = frame->h - b6->h - b1->h;

			rect.x += (i * background->w);
			rect.y += (j * background->h);

			blit_surface(background, &src, &rect, 0);
		}
	}

	/* draw any left over background (the extra pixels_needed) */
	for (j = 0; j < ver_iterations_needed; j++) {
		src.w = hor_pixels_needed;
		rect.x = frame->x + b1->w;
		rect.y = frame->y + b1->h;
		rect.w = hor_pixels_needed;
		rect.h = background->h;

		rect.x += (hor_iterations_needed * background->w);
		rect.y += (j * background->h);

		blit_surface(background, &src, &rect, 0);
	}
	for (i = 0; i < hor_iterations_needed; i++) {
		src.w = background->w;
		src.h = ver_pixels_needed;
		rect.x = frame->x + b1->w;
		rect.y = frame->y + b1->h;
		rect.w = background->w;
		rect.h = ver_pixels_needed;

		rect.x += (i * background->w);
		rect.y += (ver_iterations_needed * background->h);

		blit_surface(background, &src, &rect, 0);
	}

	/* finish that last square */
	src.w = hor_pixels_needed;
	src.h = ver_pixels_needed;
	rect.x = frame->x + b1->w;
	rect.y = frame->y + b1->h;
	rect.w = hor_pixels_needed;
	rect.h = ver_pixels_needed;
	rect.x += (hor_iterations_needed * background->w);
	rect.y += (ver_iterations_needed * background->h);
	blit_surface(background, &src, &rect, 0);

	/* render upper left corner */
	rect.x = frame->x;
	rect.y = frame->y;
	rect.w = b1->w;
	rect.h = b1->h;

	blit_surface(b1, NULL, &rect, 0);

	/* render upper border, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = frame->w - b1->w - b3->w;

		iterations_needed = pixels_needed / b2->w;

		pixels_needed -= (iterations_needed * b2->w);

		src.x = 0;
		src.y = 0;
		src.w = b2->w;
		src.h = b2->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = frame->x + b1->w;
			rect.y = frame->y;
			rect.w = b2->w;
			rect.h = b2->h;

			rect.x += (i * b2->w);

			blit_surface(b2, &src, &rect, 1);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.w = pixels_needed;
			rect.x = frame->x + b1->w;
			rect.y = frame->y;
			rect.w = pixels_needed;
			rect.h = b2->h;

			rect.x += (i * b2->w);

			blit_surface(b2, &src, &rect, 1);
		}
	}

	/* render upper right corner */
	rect.x = frame->x + frame->w - b3->w;
	rect.y = frame->y;
	rect.w = b3->w;
	rect.h = b3->h;

	blit_surface(b3, NULL, &rect, 1);

	/* render left border, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = frame->h - b1->h - b6->h;

		iterations_needed = pixels_needed / b4->h;

		pixels_needed -= (iterations_needed * b4->h);

		src.x = 0;
		src.y = 0;
		src.w = b4->w;
		src.h = b4->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = frame->x;
			rect.y = frame->y + b1->h;
			rect.w = b4->w;
			rect.h = b4->h;

			rect.y += (i * b4->h);

			blit_surface(b4, &src, &rect, 1);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.h = pixels_needed;
			rect.x = frame->x;
			rect.y = frame->y + b1->h;
			rect.w = b4->w;
			rect.h = pixels_needed;

			rect.y += (i * b4->h);

			blit_surface(b4, &src, &rect, 0);
		}
	}

	/* render right border, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = frame->h - b3->h - b8->h;

		iterations_needed = pixels_needed / b5->h;

		pixels_needed -= (iterations_needed * b5->h);

		src.x = 0;
		src.y = 0;
		src.w = b5->w;
		src.h = b5->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = frame->x + frame->w - b5->w;
			rect.y = frame->y + b3->h;
			rect.w = b5->w;
			rect.h = b5->h;

			rect.y += (i * b5->h);

			blit_surface(b5, &src, &rect, 0);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.h = pixels_needed;
			rect.x = frame->x + frame->w - b5->w;
			rect.y = frame->y + b1->h;
			rect.w = b5->w;
			rect.h = pixels_needed;

			rect.y += (i * b5->h);

			blit_surface(b5, &src, &rect, 0);
		}
	}

	/* render lower left corner */
	rect.x = frame->x;
	rect.y = frame->y + frame->h - b6->h;
	rect.w = b6->w;
	rect.h = b6->h;

	blit_surface(b6, NULL, &rect, 0);

	/* render lower border, repeating if necessary */
	{
		int pixels_needed, iterations_needed;
		int i;

		pixels_needed = frame->w - b6->w - b8->w;

		iterations_needed = pixels_needed / b7->w;

		pixels_needed -= (iterations_needed * b7->w);

		src.x = 0;
		src.y = 0;
		src.w = b7->w;
		src.h = b7->h;

		for (i = 0; i < iterations_needed; i++) {
			rect.x = frame->x + b6->w;
			rect.y = frame->y + frame->h - b7->h;
			rect.w = b7->w;
			rect.h = b7->h;

			rect.x += (i * b7->w);

			blit_surface(b7, &src, &rect, 0);
		}

		/* do left over pixels here */
		if (pixels_needed) {
			src.w = pixels_needed;
			rect.x = frame->x + b1->w;
			rect.y = frame->y + frame->h - b7->h;
			rect.w = pixels_needed;
			rect.h = b7->h;

			rect.x += (i * b7->w);

			blit_surface(b7, &src, &rect, 0);
		}
	}

	/* render lower right corner */
	rect.x = frame->x + frame->w - b8->w;
	rect.y = frame->y + frame->h - b8->h;
	rect.w = b8->w;
	rect.h = b8->h;

	blit_surface(b8, NULL, &rect, 0);

	/* finally, if they set a drawing callback, call it now that the drawing is done */
	if (frame->dc)
		(*frame->dc) (frame->x, frame->y, frame->w, frame->h);
}

int gui_init_frame(void) {

	b1 = load_image_eaf(epiar_eaf, "gui/frm_ul.png", 0);
	b2 = load_image_eaf(epiar_eaf, "gui/frm_tb.png", 0);
	b3 = load_image_eaf(epiar_eaf, "gui/frm_ur.png", 0);
	b4 = load_image_eaf(epiar_eaf, "gui/frm_lb.png", 0);
	b5 = load_image_eaf(epiar_eaf, "gui/frm_rb.png", 0);
	b6 = load_image_eaf(epiar_eaf, "gui/frm_ll.png", 0);
	b7 = load_image_eaf(epiar_eaf, "gui/frm_bb.png", 0);
	b8 = load_image_eaf(epiar_eaf, "gui/frm_lr.png", 0);
	background = load_image_eaf(epiar_eaf, "gui/frm_bg.png", 0);

	return (0);	
}

int gui_quit_frame(void) {

	SDL_FreeSurface(b1);
	SDL_FreeSurface(b2);
	SDL_FreeSurface(b3);
	SDL_FreeSurface(b4);
	SDL_FreeSurface(b5);
	SDL_FreeSurface(b6);
	SDL_FreeSurface(b7);
	SDL_FreeSurface(b8);
	SDL_FreeSurface(background);

	return (0);
}

static int gui_frame_register_session(gui_session *session, gui_frame *frame) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (frame == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = frame;
	session->child_type[slot] = GUI_FRAME;

	return (0);
}

void gui_frame_associate_drawing(gui_frame *frame, void (*dc) (int x, int y, int w, int h)) {
	if (frame == NULL)
		return;

	if (dc == NULL)
		return;

	frame->dc = dc;
}
