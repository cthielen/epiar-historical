#include "com_defs.h"
#include "gui/gui.h"
#include "includes.h"
#include "navigation/navigation.h"
#include "osdep/osdep.h"
#include "sprite/planet.h"
#include "sprite/player.h"
#include "system/eaf.h"
#include "system/math.h"
#include "system/path.h"
#include "system/timer.h"
#include "system/video/video.h"

#define NORMAL_RADAR_DIST 16000
#define MAX_PLANETS_ON_RADAR 75

struct {
  int x, y;
  struct _planet *planet;
} nav_blips[MAX_PLANETS_ON_RADAR];

void calculate_nav_blips(int center_x, int center_y, float zoom);
void draw_nav_blips(void);
int outside_of_rect(int x, int y, int rect_x, int rect_y, int rect_w, int rect_h);
void draw_grid_lines(int x, int y, int w, int h, float zoom);
void draw_nav_map(void);

float nm_zoom;
int basic_x, basic_y;
SDL_Surface *basic, *arrows, *zoom, *screen_backup;

void nav_map(void) {
	SDL_Event event;
	unsigned char quit = 0;
	unsigned char update = 0;
	unsigned char n_down = 0;

	pause_timer();

	nm_zoom = 1.0f;

	basic = load_image_eaf(epiar_eaf, "nav/basic.png", ALPHA);
	arrows = load_image_eaf(epiar_eaf, "nav/arrows.png", BLUE_COLORKEY);
	zoom = load_image_eaf(epiar_eaf, "nav/zoom.png", BLUE_COLORKEY);

	/* get the coords of the upper left of the basic nav map background */
	basic_x = 400 - (basic->w / 2);
	basic_y = 300 - (basic->h / 2);

	/* backup the screen to help "erase" the nav map */
	screen_backup = get_surface(screen, basic_x, basic_y, basic->w, basic->h);

	draw_nav_map();

	SDL_ShowCursor(1);

	while(SDL_WaitEvent(&event) && !quit) {
		switch(event.type) {
		case SDL_MOUSEBUTTONDOWN:
			{
				int cursor_x, cursor_y;
				
				SDL_GetMouseState(&cursor_x, &cursor_y);
				
				/* if clicked zoom in ... */
				if (!outside_of_rect(cursor_x, cursor_y, basic_x + 545, basic_y + 340, 18, 18)) {
					if (nm_zoom < 32.0f)
						nm_zoom *= 2.0f;
					update = 1;
				}
				/* if clicked zoom out ... */
				if (!outside_of_rect(cursor_x, cursor_y, basic_x + 568, basic_y + 340, 18, 18)) {
					if (nm_zoom > 0.03125f)
						nm_zoom /= 2.0f;
					update = 1;
				}
			}
		case SDL_KEYDOWN:
			{
				switch(event.key.keysym.sym) {
				case SDLK_n:
					{
						n_down = 1;
						break;
					}
				default:
					break;
				}
			}
		case SDL_KEYUP:
			{
				switch(event.key.keysym.sym) {
				case SDLK_n:
					{
						if (n_down) {
							quit = 1;
							SDL_PushEvent(&event);
						}
						break;
					}
				default:
					break;
				}
				break;
			}
		default:
			break;
		}

		if (update) {
			draw_nav_map();
			update = 0;
		}
	}

	SDL_ShowCursor(0);

	SDL_FreeSurface(basic);
	SDL_FreeSurface(arrows);
	SDL_FreeSurface(zoom);

	blit_image(screen_backup, basic_x, basic_y);
	SDL_FreeSurface(screen_backup);

	unpause_timer();
}

void calculate_nav_blips(int center_x, int center_y, float zoom) {
	int i;
	int num_found = 0;

	/* calculate planet coords */
	for (i = 0; i < num_planets; i++) {
		int x = ((-center_x + planets[i]->world_x) * 309) / NORMAL_RADAR_DIST;
		int y = ((center_y + planets[i]->world_y) * 218) / NORMAL_RADAR_DIST;

		x *= zoom;
		y *= zoom;

		if ((x > -309) && (x < 309)) {
			if ((y > -218) && (y < 218)) {
				if (outside_of_rect(400 + x, 300 + y, 96, 369, 142, 141) && outside_of_rect(400 + x, 300 + y, 233, 399, 376, 110)) {
					if (planets[i]->revealed) {
						nav_blips[num_found].x = 400 + x;
						nav_blips[num_found].y = 300 + y;
						nav_blips[num_found].planet = planets[i];
						num_found++;
					}
				}
			}
		}
	}

	nav_blips[num_found].x = -500; /* just dummy values to know this is the last one */
	nav_blips[num_found].y = -500;
	nav_blips[num_found].planet = NULL;
}

void draw_nav_blips(void) {
	int i;

	if (SDL_MUSTLOCK(screen)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(screen);
	}

	switch(screen_bpp) {
		case 32:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint32);
			/* draw the player */
			((Uint32 *) screen->pixels)[399 + (300 * pitch_adjust)] = white;
			((Uint32 *) screen->pixels)[400 + (300 * pitch_adjust)] = white;
			((Uint32 *) screen->pixels)[401 + (300 * pitch_adjust)] = white;
			((Uint32 *) screen->pixels)[400 + (301 * pitch_adjust)] = white;
			((Uint32 *) screen->pixels)[400 + (299 * pitch_adjust)] = white;
			/* draw ships */
			for (i = 0; (nav_blips[i].x != -500) && (nav_blips[i].y != -500); i++) {
				((Uint32 *) screen->pixels)[nav_blips[i].x + (nav_blips[i].y * pitch_adjust)] = orange;
				((Uint32 *) screen->pixels)[(nav_blips[i].x - 1) + (nav_blips[i].y * pitch_adjust)] = orange;
				((Uint32 *) screen->pixels)[(nav_blips[i].x + 1) + (nav_blips[i].y * pitch_adjust)] = orange;
				((Uint32 *) screen->pixels)[nav_blips[i].x + ((nav_blips[i].y - 1) * pitch_adjust)] = orange;
				((Uint32 *) screen->pixels)[nav_blips[i].x + ((nav_blips[i].y + 1) * pitch_adjust)] = orange;
				ensure_blitted(nav_blips[i].x - 1, nav_blips[i].y - 1, 3, 3);
			}
			break;
		}
		case 16:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint16);
			/* draw the player */
			((Uint16 *) screen->pixels)[399 + (300 * pitch_adjust)] = white;
			((Uint16 *) screen->pixels)[400 + (300 * pitch_adjust)] = white;
			((Uint16 *) screen->pixels)[401 + (300 * pitch_adjust)] = white;
			((Uint16 *) screen->pixels)[400 + (301 * pitch_adjust)] = white;
			((Uint16 *) screen->pixels)[400 + (299 * pitch_adjust)] = white;
			/* draw ships */
			for (i = 0; (nav_blips[i].x != -500) && (nav_blips[i].y != -500); i++) {
				((Uint16 *) screen->pixels)[nav_blips[i].x + (nav_blips[i].y * pitch_adjust)] = orange;
				((Uint16 *) screen->pixels)[(nav_blips[i].x - 1) + (nav_blips[i].y * pitch_adjust)] = orange;
				((Uint16 *) screen->pixels)[(nav_blips[i].x + 1) + (nav_blips[i].y * pitch_adjust)] = orange;
				((Uint16 *) screen->pixels)[nav_blips[i].x + ((nav_blips[i].y - 1) * pitch_adjust)] = orange;
				((Uint16 *) screen->pixels)[nav_blips[i].x + ((nav_blips[i].y + 1) * pitch_adjust)] = orange;
				ensure_blitted(nav_blips[i].x - 1, nav_blips[i].y - 1, 3, 3);
			}
			break;
		}
		default:
		{
			break;
		}
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	/* draw the names of the planets */
	for (i = 0; (nav_blips[i].x != -500) && (nav_blips[i].y != -500); i++) {
	  int w, h, base;

	  epiar_size_text(gui_font_normal, nav_blips[i].planet->name, &w, &h, &base);
	  epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, nav_blips[i].x - (w / 2), nav_blips[i].y + 12, nav_blips[i].planet->name);
	}
}

/* returns true if outside of specified rect */
int outside_of_rect(int x, int y, int rect_x, int rect_y, int rect_w, int rect_h) {
	if (((x > rect_x) && (x < (rect_x + rect_w))) && ((y > rect_y) && (y < (rect_y + rect_h))))
		return (0);

	return (1);
}

void draw_grid_lines(int x, int y, int w, int h, float zoom) {
	float square_w = w / 3; /* 3 squares across */
	float square_h = h / 2; /* 2 squares down */
	float i;

	square_w *= zoom;
	square_h *= zoom;

	/* draw the lines across */
	for (i = y; i < (y + h); i += square_h) {
		SDL_Rect rect;

		rect.x = x;
		rect.y = i;
		rect.w = w;
		rect.h = 1;

		fill_rect(&rect, grey);
	}

	/* draw the lines down */
	for (i = x; i < (x + w); i += square_w) {
		SDL_Rect rect;

		rect.x = i;
		rect.y = y;
		rect.w = 1;
		rect.h = h;

		fill_rect(&rect, grey);
	}
}

void draw_nav_map(void) {

	/* "erase" the screen */
	blit_image(screen_backup, basic_x, basic_y);

	/* recalc nav points */
	calculate_nav_blips(player.ship->world_x, -player.ship->world_y, nm_zoom);

	/* draw the grid lines */
	draw_grid_lines(basic_x, basic_y, basic->w, basic->h, nm_zoom);

	/* draw the nav map */
	blit_image(basic, basic_x, basic_y);
	blit_image(arrows, basic_x + 537, basic_y + 378);
	blit_image(zoom, basic_x + 545, basic_y + 340);

	/* draw already calculated nav blips */
	draw_nav_blips();

	flip();	
}

/* reveals the planet with the given name */
void nav_reveal_planet(char *planet) {
	struct _planet *the_planet = NULL;
		
	the_planet = get_planet_pointer(planet);
	if(!the_planet) {
		printf("Could not reveal planet \"%s\". Please ensure this planet exists.\n", planet);
		return;
	}

	the_planet->revealed = 1;
}
