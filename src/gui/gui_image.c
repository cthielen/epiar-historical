#include "gui/gui.h"
#include "system/eaf.h"
#include "system/video/video.h"

static int gui_image_register_session(gui_session *session, gui_image *image);

gui_image *gui_create_image(short int x, short int y, char *filename, gui_session *session) {
	gui_image *image = (gui_image *)malloc(sizeof(gui_image));
	SDL_Surface *temp;

	image->x = x;
	image->y = y;
	image->visible = 1;
	image->update = 0;

	temp = IMG_Load(filename);
	if (temp == NULL) {
		free(image);
		return (NULL);
	}
	image->image = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	if (gui_image_register_session(session, image) != 0) {
		SDL_FreeSurface(image->image);
		free(image);
		return (NULL);
	}

	return (image);
}

gui_image *gui_create_image_eaf(FILE *eaf_file, short int x, short int y, char *filename, gui_session *session) {
	gui_image *image = (gui_image *)malloc(sizeof(gui_image));
	SDL_Surface *temp;

	image->x = x;
	image->y = y;
	image->visible = 1;
	image->update = 0;

	temp = eaf_load_png(eaf_file, filename);
	if (temp == NULL) {
		free(image);
		return (NULL);
	}
	image->image = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	if (gui_image_register_session(session, image) != 0) {
		SDL_FreeSurface(image->image);
		free(image);
		return (NULL);
	}

	return (image);
}

int gui_destroy_image(gui_image *image) {
	if (image == NULL)
		return (-1);

	SDL_FreeSurface(image->image);
	free(image);

	return (0);
}

void gui_show_image(gui_image *image) {
	SDL_Rect dest;

	if (image == NULL)
		return;

	if (!image->visible)
		return;

	dest.x = image->x;
	dest.y = image->y;
	dest.w = image->image->w;
	dest.h = image->image->h;

	blit_surface(image->image, NULL, &dest, 0);
}

int gui_init_image(void) {

	return (0);
}

int gui_quit_image(void) {

	return (0);	
}

static int gui_image_register_session(gui_session *session, gui_image *image) {
	int i, slot = -1;

	if (session == NULL)
		return (-1);
	if (image == NULL)
		return (-1);

	for (i = 0; i < MAX_SESSION_CHILDREN; i++) {
		if (session->child_type[i] == GUI_NONE) {
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return (-1); /* session full */

	session->children[slot] = image;
	session->child_type[slot] = GUI_IMAGE;

	return (0);
}
