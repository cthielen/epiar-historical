#include "game/game.h"
#include "gui/gui.h"
#include "menu/status.h"
#include "sprite/sprite.h"
#include "system/font.h"
#include "system/math.h"
#include "system/video/video.h"

#define PLANET_PICT_CENTERED_X 425
#define PLANET_PICT_CENTERED_Y 205

gui_session *status_session;

static void okay_btn_handle(void);
static void draw_status_screen(void);
static struct _planet *get_nearest_planet(int x, int y);
static SDL_Surface *display = NULL; /* screen backup */

void ship_status(void) {
	gui_button *okay_btn;
	SDL_Rect rect;

	return; /* disabled for now */

	status_session = gui_create_session();
	
	okay_btn = gui_create_button(700, 570, 85, 22, "Okay", status_session);
	gui_button_set_callback(okay_btn, okay_btn_handle);

	/* back up the screen */
	display = SDL_CreateRGBSurface(SDL_SWSURFACE, 800, 600, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	rect.x = 0;
	rect.y = 0;
	rect.w = 800;
	rect.h = 600;
	SDL_BlitSurface(screen, &rect, display, &rect);

	draw_status_screen();

	gui_session_show_all(status_session);

	gui_main(status_session);
	SDL_ShowCursor(1);

	gui_session_destroy_all(status_session);
	gui_destroy_session(status_session);

	/* "erase" the status display by drawing what was there before we drew */
	blit_surface(display, &rect, &rect, 1);
	SDL_FreeSurface(display);
	display = NULL;

	SDL_ShowCursor(0);
}

static void okay_btn_handle(void) {
	status_session->active = 0;
}

static void draw_status_screen(void) {
	SDL_Surface *temp, *wireframe;
	SDL_Rect src, dest;
	int w;
	struct _planet *nearest_planet = NULL;
	int temp_x, temp_y; /* for misc. variables, changes often */

	/* cover the screen in black */
	black_fill(0, 0, 800, 600, 1);

	/* load the ship's wireframe */
	temp = IMG_Load(player.ship->model->wireframe);
	if (temp == NULL) {
		fprintf(stdout, "Could not load %s\n", player.ship->model->wireframe);
		return;
	}
	wireframe = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	/* draw the ship's wireframe */
	src.x = 0;
	src.y = 0;
	src.w = wireframe->w;
	src.h = wireframe->h;
	dest.x = 10;
	dest.y = 35;
	dest.w = src.w;
	dest.h = src.h;
	blit_surface(wireframe, &src, &dest, 0);

	/* draw the class of the ship at the top, above the wireframe */
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 10 + (wireframe->w / 2), 5, player.ship->model->name);

	/* draw the ship information */
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 550, 12, "Statistics");
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 533, 100, "Nearest System");

	/* draw the four labels but save the weapon 2 type width, as it dictates how far over to draw the names */
	/* draw the engine,shield,weapon information */
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 465, 30, "Engine Type:");
	epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 465, 44, "Shield Type:");

	/* draw the nearest planet */
	nearest_planet = get_nearest_planet(player.ship->world_x, player.ship->world_y);
	src.x = 0;
	src.y = 0;
	src.w = nearest_planet->image->w;
	src.h = nearest_planet->image->h;
	dest.x = PLANET_PICT_CENTERED_X - (src.w / 2);
	dest.y = PLANET_PICT_CENTERED_Y - (src.h / 2);
	dest.w = src.w;
	dest.h = src.h;
	blit_surface(nearest_planet->image, &src, &dest, 1);

	/* draw the nearest planet information */
	temp_x = PLANET_PICT_CENTERED_X + (src.w / 2) + 5;
	temp_y = PLANET_PICT_CENTERED_Y - (src.h / 2) + 10;
	/*
		w = draw_text(font, temp_x, temp_y, "System Name:", NULL, 0, -1, -1);
		draw_text(font, temp_x, temp_y + 14, "Alliance:", NULL, 0, -1, -1);
		draw_text(font, temp_x, temp_y + 28, "Legal Status:", NULL, 0, -1, -1);
		draw_text(font, temp_x + w + 5, temp_y, nearest_planet->name, NULL, 0, -1, -1);
		draw_text(font, temp_x + w + 5, temp_y + 14, nearest_planet->alliance->name, NULL, 0, -1, -1);
		draw_text(font, temp_x + w + 5, temp_y + 28, "Clean", NULL, 0, -1, -1);
	*/

	/* draw the "how long you've been playing" text */
	/*
		draw_text(font, 5, 585, "Total Play Time: Unknown", NULL, 0, -1, -1);
	*/

	SDL_FreeSurface(wireframe);
}

static struct _planet *get_nearest_planet(int x, int y) {
	struct _planet *nearest = NULL;
	int i;
	int nearest_dist = get_distance_sqrd(x, y, planets[0]->world_x, planets[0]->world_y);

	/* first planet is closest unless the for loop proves otherwise */
	nearest = planets[0];

	for (i = 0; i < num_planets; i++) {
		int dist;
		dist = get_distance_sqrd(x, y, planets[i]->world_x, planets[i]->world_y);
		if (dist < nearest_dist) {
			/* found a planet that's closer */
			nearest_dist = dist;
			nearest = planets[i];
		}
	}

	return (nearest);
}
