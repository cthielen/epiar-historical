#include "asteroid/asteroid.h"
#include "includes.h"
#include "com_defs.h"
#include "game/scenario.h"
#include "game/update.h"
#include "hud/hud.h"
#include "network/net_sprite.h"
#include "osdep/osdep.h"
#include "racing/track.h"
#include "sprite/chunk.h"
#include "sprite/fire.h"
#include "sprite/flare.h"
#include "sprite/gate.h"
#include "sprite/particle.h"
#include "sprite/planet.h"
#include "sprite/sprite.h"
#include "system/eaf.h"
#include "system/debug.h"
#include "system/math.h"
#include "system/path.h"
#include "system/plugin.h"
#include "system/rander.h"
#include "system/trig.h"
#include "system/video/backbuffer.h"
#include "system/video/video.h"

#define LINE_WIDTH 2

static struct _starfield {
	float x, y;
	float scaler;
	Uint32 color;
} *starfield = NULL;

#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

#ifndef M_PI_2
#define M_PI_2 1.570796326794896619
#endif

#define MAX_BLIT_RECTS 250

int blits_done;
int num_stars = 200; /* number of stars to draw */
static int thrown_away_blits;
SDL_Rect blitted_rects[MAX_BLIT_RECTS]; /* 600 stars (erase 300, draw 300) plus 200 extra blits for whatever */

#ifdef WIN32
#define tanf(value) (float)tan(value)
#define sinf(value) (float)sin(value)
#define cosf(value) (float)cos(value)
#endif

/* Following code (C) 2002 Jared Minch, released under GNU GPL version 2 (see LICENSE for a copy), thanks aardvark! */
#define itofix(x) ((x) << 16)
#define ftofix(x) ((long)((x) * 65536.0 + ((x) < 0 ? -0.5 : 0.5)))
#define fixtoi(x) (((x) >> 16) + (((x) & 0x8000) >> 15))
#define fixtof(x) ((float) (x) / 65536.0)
#define fixdiv(x,y) ftofix(fixtof(x) / fixtof(y))

typedef long fixed;
/* End of code, thanks aardvark! */

SDL_Surface *screen;
Uint32 black, white, grey, blue, dark_grey, green, dark_green, light_blue, yellow, red, orange, dark_cyan;
Uint8 screen_bpp; /* bpp of the screen the game is using */

unsigned char hud_draw_first_time = 1; /* at the beginning of a game, you need to set this to one */

static void draw_ships(void);
void draw_ship(struct _ship *ship); /* make me static when done debugging */
static void erase_ships(void);
static void draw_gates_bottom(void);
static void draw_gates_top(void);
static void erase_gates(void);
static void draw_planets(void);
static void erase_planets(void);
static void draw_fires(void);
static void erase_fires(void);
static void draw_fire(struct _fire *fire);
static void erase_fire(struct _fire *fire);
static void draw_planet(struct _planet *planet);
static void erase_planet(struct _planet *planet);
static void draw_boxed_selection(int x, int y, int w, int h);

int scr_top = 0, scr_bottom = 599, scr_left = 0, scr_right = 799; /* virtual screen size */
unsigned char fullscreen = 1;

void set_dimensions(int top, int bottom, int left, int right) {
	scr_top = top;
	scr_bottom = bottom;
	scr_left = left;
	scr_right = right;
}

/******************************************************************************      
*
*   Name:
*      int setup_video(int width, int height, int bpp, unsigned char fullscreen);
*
*   Abstract:
*      Initializes dirty_tiles(), creates backbuffer, etc. Call this before
*   any video functions, especially the wrappers.
*
*   Context/Scope:
*      Called from init().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      0 on success, -1 on failure.
*
*   Assumptions:
*      SDL must be initialized.
*
******************************************************************************/
int setup_video(int width, int height, int bpp, unsigned char fullscreen) {
	const SDL_VideoInfo *video_info = NULL;
	Uint8 bpp_received;

	printf("1\n");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		debug_message("\n    Could not initalize SDL: ");
		fprintf(stdout, "Error: %s\n", SDL_GetError());
		debug_message(SDL_GetError());
		return (-1);
	}

	atexit(SDL_Quit);

	printf("2\n");

	screen = SDL_SetVideoMode(width, height, bpp, (fullscreen ? SDL_FULLSCREEN : 0));
	if (screen == NULL) {
		debug_message("    Could not set video mode: ");
		debug_message(SDL_GetError());
		return (-1);
	}

	printf("3\n");

	/* get information about video */
	video_info = SDL_GetVideoInfo();
	bpp_received = video_info->vfmt->BitsPerPixel;

	/* change the video mode to whatever SDL gives us if it's not what we wanted */
	if (bpp_received != bpp) {
		SDL_FreeSurface(screen);
		screen = SDL_SetVideoMode(width, height, bpp_received, (fullscreen ? SDL_FULLSCREEN : 0));
		if (screen == NULL) {
			debug_message("failed.\n");
			debug_message(SDL_GetError());
			return (-1);
		}
		/* make sure we got what we wanted the second time */
		if (screen->format->BitsPerPixel != bpp_received) {
			fprintf(stdout, "Could not establish correct bpp\n");
			return (-1);
		}
	}

	screen_bpp = bpp_received;

	initialize_tiles();

	blits_done = 0;
	thrown_away_blits = 0;

	return (0);
}

/******************************************************************************      
*
*   Name:
*      void cleanup_video(void);
*
*   Abstract:
*      Frees the backbuffer. Call this when prog is quitting.
*
*   Context/Scope:
*      Called from de_init().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      No more drawing after this.
*
******************************************************************************/
void cleanup_video(void) {
	SDL_FreeSurface(screen);
	screen = NULL;
}

/******************************************************************************      
*
*   Name:
*      void flip(void);
*
*   Abstract:
*      Flips the backbuffer onto the main screen (actually displays it).
*
*   Context/Scope:
*      Called from game_loop() once per loop.
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      None.
*
******************************************************************************/
int flip(void) {

	SDL_UpdateRect(screen, 0, 0, 0, 0);

	/*
	if (thrown_away_blits) {
#ifndef NDEBUG
		fprintf(stdout, "%d blits discarded\n", thrown_away_blits);
#endif
		thrown_away_blits = 0;
		
		SDL_UpdateRect(screen, 0, 0, 0, 0);
	} else {
		SDL_UpdateRects(screen, blits_done, &blitted_rects[0]);
	}
	
	blits_done = 0;
	*/
	
	return (0);
}

Uint32 map_rgb(int r, int g, int b) {
	assert(screen != NULL);
	return (SDL_MapRGB(screen->format, r, g, b));
}

void fill_rect(SDL_Rect *dest, Uint32 color) {

	if (dest->w <= 0)
		return;
	if (dest->h <= 0)
		return;
	if ((dest->x + dest->w) < 0)
		return;
	if ((dest->y + dest->h) < 0)
		return;
	if (dest->x < 0) {
		dest->w -= (0 - dest->x);
		dest->x = 0;
	}
	if (dest->y < 0) {
		dest->h -= (0 - dest->y);
		dest->y = 0;
	}
	if (dest->x > 799)
		return;
	if (dest->y > 599)
		return;
	if ((dest->x + dest->w) > 799)
		dest->w = 799 - dest->x;
	if ((dest->y + dest->h) > 599)
		dest->h = 599 - dest->y;
	
	SDL_FillRect(screen, dest, color);
	ensure_blitted(dest->x, dest->y, dest->w, dest->h);
}

void fill_rect4(int x, int y, int w, int h, Uint32 color) {
	SDL_Rect rect;
	
	if (w <= 0)
		return;
	if (h <= 0)
		return;
	if ((x + w) < 0)
		return;
	if ((y + h) < 0)
		return;
	if (x < 0) {
		w -= (0 - x);
		x = 0;
	}
	if (y < 0) {
		h -= (0 - y);
		y = 0;
	}
	if (x > 799)
		return;
	if (y > 599)
		return;
	if ((x + w) > 799)
		w = 799 - x;
	if ((y + h) > 599)
		h = 599 - y;
	
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	
	SDL_FillRect(screen, &rect, color);
	ensure_blitted(x, y, w, h);
}

/******************************************************************************      
*
*   Name:
*      int blit_surface(SDL_Surface *surface, SDL_Rect *src, SDL_Rect *dest, unsigned char hud_blit);
*
*   Abstract:
*      Wrapper for SDL_BlitSurface. Draws to the software backbuffer, not the
*   screen. Should _always_ be used instead of SDL_BlitSurface(). hud_blit is
*   used to know whether the bitmap being blitted is part of the hud, if so
*   it doesn't have to obey the virtual screen size/position boundaries, other-
*   wise, all blits must be within the virtual screen.
*
*   Context/Scope:
*      Called throughout.
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      It has (or had, if I removed it) limited checking of the bounds, but
*   still, pass it valid screen coords, okay? Geesh.
*
******************************************************************************/
int blit_surface(SDL_Surface *surface, SDL_Rect *psrc, SDL_Rect *dest, unsigned char hud_blit) {
	SDL_Rect src;
	assert(surface != NULL);
	assert(dest != NULL);

	if (psrc == NULL) {
		src.x = 0;
		src.y = 0;
		src.w = surface->w;
		src.h = surface->h;
	} else {
		src.x = psrc->x;
		src.y = psrc->y;
		src.w = psrc->w;
		src.h = psrc->h;
	}

	if (!hud_blit) {
		if ((dest->x + dest->w) < scr_left) return (-1);
		if ((dest->y + dest->h) < scr_top) return (-1);
		if (dest->x < scr_left) {
			int cut_off;
			cut_off = scr_left - dest->x;
			src.x = src.x + cut_off;
			src.w = src.w - cut_off;
			dest->w = dest->w - cut_off;
			dest->x = scr_left;
		}
		if (dest->y < scr_top) {
			src.y = (scr_top - dest->y);
			dest->h = dest->y + dest->h;
			dest->y = scr_top;
		}
		if (dest->x > scr_right) return (-1);
		if (dest->y > scr_bottom) return (-1);
		if ((dest->x + dest->w) > scr_right) {
			dest->w = dest->w + (scr_right - (dest->x + dest->w));
			src.w = dest->w;
		}
		if ((dest->y + dest->h) > scr_bottom) {
			dest->h = dest->h + (scr_bottom - (dest->y + dest->h));
			src.h = dest->h;
		}
	}
	if (dest->h <= 0) return (-1);
	if (dest->w <= 0) return (-1);

	assert(src.x >= 0);
	assert(src.y >= 0);
	assert(src.w > 0);
	assert(src.h > 0);
	assert(dest->x >= 0);
	assert(dest->y >= 0);
	assert(dest->x < 800);
	assert(dest->y < 600);
	assert(dest->w > 0);
	assert(dest->h > 0);

	SDL_BlitSurface(surface, &src, screen, dest);
	ensure_blitted(dest->x, dest->y, dest->w, dest->h);

	return (0);
}

/******************************************************************************      
*
*   Name:
*      void draw_line(int x1, int y1, int x2, int y2, Uint32 color);
*
*   Abstract:
*      Draws a line on the screen using Bresenham's line algorithm.
*   NOTE: This really needs updating to support drawing on any surface pointer,
*   but that can be done quickly when it needs to be.
*
*   Context/Scope:
*      Called throughout.
*
*   Side Effects:
*      Very slow. Uses put_pixel! *horrified gasps*
*
*   Return Value:
*      None.
*
*   Assumptions:
*      Assumes the line is being drawn to backbuffer/screen.
*
******************************************************************************/
void draw_line(int x1, int y1, int x2, int y2, Uint32 color) {
	int dx, dy, ix, iy, inc;
	int plotx, ploty;
	int x = 0, y = 0;
	int i;
	int plot = 0;
	int pitch_adjust;

	if (screen_bpp == 32)
		pitch_adjust = screen->pitch / sizeof(Uint32);
	else if (screen_bpp == 16)
		pitch_adjust = screen->pitch / sizeof(Uint16);

	if (SDL_MUSTLOCK(screen)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(screen);
	}

	/* swap values to make the algorithm work in some cases */
	if (x1 > x2) {
		int temp;
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y1 > y2) {
		int temp;
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	dx = x2 - x1;
	dy = y2 - y1;
	if (dx >= 0) {
		ix = dx;
	} else {
		ix = (dx * -1);
	}
	if (dy >= 0) {
		iy = dy;
	} else {
		iy = (dy * -1);
	}
	if (dx > dy) {
		inc = dx;
	} else {
		inc = dy;
	}

	plotx = x1;
	ploty = y1;

	if (plotx >= scr_left)
		if (ploty >= scr_top)
			if (plotx < scr_right)
				if (ploty < scr_bottom) {
					switch(screen_bpp) {
						case 32:
						{
							((Uint32 *) screen->pixels)[plotx + (ploty * pitch_adjust)] = color;
							break;
						}
						case 16:
						{
							((Uint16 *) screen->pixels)[plotx + (ploty * pitch_adjust)] = color;
							break;
						}
						default:
							break;
					}
					ensure_blitted(plotx, ploty, 1, 1);
				}

	for (i = 0; i < inc; i++) {
		x += ix;
		y += iy;

		if (x > inc) {
			plot = 1;
			x -= inc;
			if (dx >= 0) {
					plotx++;
			} else {
				plotx--;
			}
		}

		if (y > inc) {
			plot = 1;
			y -= inc;
			if (dy >= 0) {
				ploty++;
			} else {
				ploty--;
			}
		}

		if (plot) {
			if (plotx >= scr_left)
				if (ploty >= scr_top)
					if (plotx < scr_right)
						if (ploty < scr_bottom) {
							switch(screen_bpp) {
								case 32:
								{
									((Uint32 *) screen->pixels)[plotx + (ploty * pitch_adjust)] = color;
									break;
								}
								case 16:
								{
									((Uint16 *) screen->pixels)[plotx + (ploty * pitch_adjust)] = color;
									break;
								}
								default:
									break;
							}
							ensure_blitted(plotx, ploty, 1, 1);
						}
			plot = 0;
		}
	}

	if (plotx >= 0)
		if (plotx >= scr_left)
			if (ploty >= scr_top)
				if (plotx < scr_right)
					if (ploty < scr_bottom) {
						switch(screen_bpp) {
							case 32:
							{
								((Uint32 *) screen->pixels)[plotx + (ploty * pitch_adjust)] = color;
								break;
							}
							case 16:
							{
								((Uint16 *) screen->pixels)[plotx + (ploty * pitch_adjust)] = color;
								break;
							}
							default:
								break;
						}
						ensure_blitted(plotx, ploty, 1, 1);
					}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
}

/******************************************************************************      
*
*   Name:
*      void erase_frame(void);
*
*   Abstract:
*      Updates the screen by erasing anything that needs drawing.
*
*   Context/Scope:
*      Called from game_loop().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      None.
*
******************************************************************************/
void erase_frame(void) {

	/* follows same order as draw_frame() */
	erase_starfield();
	erase_planets();
	erase_tracks();
	erase_particles();
	erase_chunks();
	erase_fires();
	erase_flares();
	erase_asteroids();
	erase_gates();
	erase_ships();
	erase_net_ships();

	erase_hud(0);

	plugins_erase();
}

/******************************************************************************      
*
*   Name:
*      void draw_frame(void);
*
*   Abstract:
*      Updates the screen by drawing anything that needs drawing.
*
*   Context/Scope:
*      Called from game_loop().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      None.
*
******************************************************************************/
void draw_frame(unsigned char force_full_redraw) {

	draw_starfield();
	draw_planets();
	draw_tracks();
	draw_gates_bottom();
	draw_particles();
	draw_chunks();
	draw_fires();
	draw_flares();
	draw_asteroids();
	draw_ships();
	draw_net_ships();
	draw_gates_top();

	/* draw the hud last */
	if (hud_draw_first_time || force_full_redraw) {
		draw_hud(1);
		hud_draw_first_time = 0;
	} else {
		draw_hud(0);
	}

	plugins_draw();
}

/******************************************************************************      
*
*   Name:
*      static void draw_planets(void);
*
*   Abstract:
*      Draws all on-screen/near-screen planets using draw_planet().
*
*   Context/Scope:
*      Called from draw_frame().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      None.
*
******************************************************************************/
static void draw_planets(void) {
	int i;
	
	for (i = 0; i < num_planets; i++) {
		if ((planets[i]->screen_x > -500) && (planets[i]->screen_x < (screen_width + 500)) && (planets[i]->screen_y > -500) && (planets[i]->screen_y < (screen_height + 500))) {
			draw_planet(planets[i]);
			if ((planets[i] == player.selected_planet) && (planets[i]->image))
				draw_boxed_selection(planets[i]->screen_x, planets[i]->screen_y, planets[i]->image->w, planets[i]->image->h);
		}
	}
}

/******************************************************************************      
*
*   Name:
*      static void draw_ships(void);
*
*   Abstract:
*      Draws all on-screen/near-screen ships, using draw_ship().
*
*   Context/Scope:
*      Called from draw_frame().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      struct _ship is fully initialized and all pointers are valid (non-NULL).
*
******************************************************************************/
static void draw_ships(void) {
	int i;
	extern Uint32 player_dead;
	extern unsigned char view_mode;

	/* draw the tracked ships */
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			if ((current_time >= ships[i]->creation_delay) && (!ships[i]->landed)) {
				if ((ships[i]->screen_x > -300) && (ships[i]->screen_x < (screen_width + 300)) && (ships[i]->screen_y > -300) && (ships[i]->screen_y < (screen_height + 300)))
					draw_ship(ships[i]);
			}
		}
	}

	/* draw the player */
	if (!view_mode)
		if (!player_dead)
			draw_ship(player.ship);
}

/******************************************************************************      
*
*   Name:
*      static void erase_ships(void);
*
*   Abstract:
*      Erases all on-screen/near-screen ships, using erase_ship().
*
*   Context/Scope:
*      Called from erase_frame().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      struct _ship is fully initialized and all pointers are valid (non-NULL).
*
******************************************************************************/
static void erase_ships(void) {
	int i;
	extern unsigned char view_mode;

	/* erase tracked ships */
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			if ((current_time > ships[i]->creation_delay) && (!ships[i]->landed)) {
				if ((ships[i]->screen_x > -500) && (ships[i]->screen_x < (screen_width + 500)) && (ships[i]->screen_y > -500) && (ships[i]->screen_y < (screen_height + 500)))
					erase_ship(ships[i]);
			}
		}
	}

	/* erase the player */
	if (!view_mode)
		erase_ship(player.ship);
}

/******************************************************************************      
*
*   Name:
*      void erase_ship(struct _ship *ship);
*
*   Abstract:
*      Erases ship from the screen using ship->surface struct values.
*
*   Context/Scope:
*      Called from erase_ships().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      struct _ship is valid and ship->screen_x, ship->screen_y have not
*   changed since it was drawn.
*
******************************************************************************/
void erase_ship(struct _ship *ship) {
	SDL_Rect dest;
	int screen_x;
	int screen_y;
	SDL_Surface *surf = ship->model->cached[ship->angle / 10];

	if (!surf) /* surf would exist if ship had been drawn */
		return;

	/* if we're drawing the player, we already know the coords */
	if (ship == player.ship) {
		screen_x = 400;
		screen_y = 300;
	} else {
		screen_x = ship->screen_x;
		screen_y = ship->screen_y;
	}
	
	dest.w = surf->w;
	dest.h = surf->h;
	dest.x = screen_x - (dest.w / 2);
	dest.y = screen_y - (dest.h / 2);
	
	black_fill(dest.x, dest.y, dest.w, dest.h, 0);
	
	/* erase the recticle around the ship if it's the player's target */
	if (player.target.type == TARGET_SHIP) {
		struct _ship *target = (struct _ship *)player.target.target;
		
		if (target == ship) {
			/* this is the player's target, so erase the recticle */
			erase_boxed_selection(ship->screen_x - (surf->w / 2), ship->screen_y - (surf->h / 2), surf->w, surf->h);
		}
	}
	
	if (ship == player.ship) {
		if (player.target.type != TARGET_NONE) {
			/* erase the little follow dot if they targetted a ship */
			float angle_to = 0.0f;
			int dot_x, dot_y;
			
			if (player.target.type == TARGET_SHIP) {
				struct _ship *target = (struct _ship *)player.target.target;
				
				angle_to = atan((float)(player.ship->world_y - target->world_y) / (float)(player.ship->world_x - target->world_x));
				if ((player.ship->world_x - target->world_x) < 0)
					angle_to += M_PI;
			} else if (player.target.type == TARGET_GATE) {
				struct _gate *target = (struct _gate *)player.target.target;
				
				angle_to = atan((float)(player.ship->world_y - target->world_y) / (float)(player.ship->world_x - target->world_x));
				if ((player.ship->world_x - target->world_x) < 0)
					angle_to += M_PI;
			}
			
			dot_x = cos(angle_to) * (player.ship->model->radius + 4); /* move it out slightly farther in case of a shield flare */
			dot_y = sin(angle_to) * (player.ship->model->radius + 4);
			
			dot_x = player.ship->screen_x - dot_x;
			dot_y = player.ship->screen_y - dot_y;
			
			fill_rect4(dot_x, dot_y, 1, 1, black);
		}
	}
}

/******************************************************************************      
*
*   Name:
*      static void draw_ship(struct _ship *ship);
*
*   Abstract:
*      Draws the ship->surface to the screen, and if that surface doesn't exist
*   or needs to be created, it calls rotate_surface to do so.
*
*   Context/Scope:	
*      Called from draw_ships().
*
*   Side Effects:
*      Modifies ship->old_angle to match ship->angle once an update has been
*   performed.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      ship must be a valid struct _ship w/ model not NULL (aka, must be fully
*   initialized).
*
******************************************************************************/
void draw_ship(struct _ship *ship) {
	SDL_Rect src, dest;
	int screen_x;
	int screen_y;
	SDL_Surface *surf = ship->model->cached[ship->angle / 10];
	
	assert(ship);

	if (!surf) {
		surf = ship->model->cached[ship->angle / 10] = rotate(ship->model->image, (ship->angle / 10) * 10);
		if (!surf)
			printf("rotate returned null\n");
		ship->model->cache_expiration = current_time + 10000;
	}

	assert(surf);

	/* if we're drawing the player, we already know the coords */
	if (ship == player.ship) {
		screen_x = 400;
		screen_y = 300;
	} else {
		screen_x = ship->screen_x;
		screen_y = ship->screen_y;
	}
	
	src.x = 0;
	src.y = 0;
	src.w = surf->w;
	src.h = surf->h;
	dest.x = screen_x - (src.w / 2);
	dest.y = screen_y - (src.h / 2);
	dest.w = src.w;
	dest.h = src.h;
	if ((ship->status & SHIP_CLOAKING) || (ship->status & SHIP_DECLOAKING)) {
		/* if a ship is cloaking we have to set alpha, but if the alpha is 0, dont even bother (you cant see it) */
		if (ship->cloak > 0) {
			SDL_SetAlpha(surf, SDL_SRCALPHA | SDL_RLEACCEL, ship->cloak);
			blit_surface(surf, &src, &dest, 0);
			SDL_SetAlpha(surf, SDL_SRCALPHA | SDL_RLEACCEL, 255);
		}
	} else
		blit_surface(surf, &src, &dest, 0);
	
	/* draw a recticle around the ship if it's the player's target */
	if (player.target.type == TARGET_SHIP) {
		struct _ship *target = (struct _ship *)player.target.target;
		
		if (target == ship) {
			/* this is the player's target, so draw the recticle */
			draw_boxed_selection(ship->screen_x - (surf->w / 2), ship->screen_y - (surf->h / 2), surf->w, surf->h);
		}
	}
	
	if (ship == player.ship) {
		if (player.target.type != TARGET_NONE) {
			/* erase the little follow dot if they targetted a ship */
			float angle_to = 0.0f;
			int dot_x, dot_y;
			
			if (player.target.type == TARGET_SHIP) {
				struct _ship *target = (struct _ship *)player.target.target;
				
				angle_to = atan((float)(player.ship->world_y - target->world_y) / (float)(player.ship->world_x - target->world_x));
				if ((player.ship->world_x - target->world_x) < 0)
					angle_to += M_PI;
			} else if (player.target.type == TARGET_GATE) {
				struct _gate *target = (struct _gate *)player.target.target;
				
				angle_to = atan((float)(player.ship->world_y - target->world_y) / (float)(player.ship->world_x - target->world_x));
				if ((player.ship->world_x - target->world_x) < 0)
					angle_to += M_PI;
			}
			
			dot_x = cos(angle_to) * (player.ship->model->radius + 4); /* move it out slightly farther in case of a shield flare */
			dot_y = sin(angle_to) * (player.ship->model->radius + 4);
			
			dot_x = player.ship->screen_x - dot_x;
			dot_y = player.ship->screen_y - dot_y;
			
			fill_rect4(dot_x, dot_y, 1, 1, red);
		}
	}
}

void black_fill(int x, int y, int width, int height, unsigned char hud_blit) {
	SDL_Rect blacker;

	if (!hud_blit) {
		if ((x + width) < scr_left)
			return;
		if ((y + height) < scr_top)
			return;
		if (x < scr_left) {
			int cut_off;
			cut_off = scr_left - x;
			width = width - cut_off;
			x = scr_left;
		}
		if (y < scr_top) {
			height = y + height;
			y = scr_top;
		}
		if (x > scr_right)
			return;
		if (y > scr_bottom)
			return;
		if ((x + width) > scr_right)
			width = width + (scr_right - (x + width));
		if ((y + height) > scr_bottom) {
			height = height + (scr_bottom - (y + height));
		}
	}
	if (height <= 0) return;
	if (width <= 0) return;

	blacker.x = x;
	blacker.y = y;
	blacker.w = width;
	blacker.h = height;

	SDL_FillRect(screen, &blacker, black);
	ensure_blitted(blacker.x, blacker.y, blacker.w, blacker.h);
}

void black_fill_rect(SDL_Rect *rect, unsigned char hud_blit) {

	if (!hud_blit) {
		if ((rect->x + rect->w) < scr_left)
			return;
		if ((rect->y + rect->h) < scr_top)
			return;
		if (rect->x < scr_left) {
			int cut_off;
			cut_off = scr_left - rect->x;
			rect->w = rect->w - cut_off;
			rect->x = scr_left;
		}
		if (rect->y < scr_top) {
			rect->h = rect->y + rect->h;
			rect->y = scr_top;
		}
		if (rect->x > scr_right)
			return;
		if (rect->y > scr_bottom)
			return;
		if ((rect->x + rect->w) > scr_right)
			rect->w = rect->w + (scr_right - (rect->x + rect->w));
		if ((rect->y + rect->h) > scr_bottom) {
			rect->h = rect->h + (scr_bottom - (rect->y + rect->h));
		}
	}
	if (rect->h <= 0) return;
	if (rect->w <= 0) return;

	SDL_FillRect(screen, rect, black);
	ensure_blitted(rect->x, rect->y, rect->w, rect->h);
}

void draw_starfield(void) {
	int i;
	
	assert(starfield);
	
	if (SDL_MUSTLOCK(screen)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(screen);
	}
	
	switch(screen_bpp) {
	case 32:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint32);
			for (i = 0; i < num_stars; i++) {
				((Uint32 *) screen->pixels)[(int)starfield[i].x + ((int)starfield[i].y * pitch_adjust)] = starfield[i].color;
				ensure_blitted((int)starfield[i].x, (int)starfield[i].y, 1, 1);
			}
			break;
		}
	case 16:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint16);
			for (i = 0; i < num_stars; i++) {
				((Uint16 *) screen->pixels)[(int)starfield[i].x + ((int)starfield[i].y * pitch_adjust)] = starfield[i].color;
				ensure_blitted((int)starfield[i].x, (int)starfield[i].y, 1, 1);
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
}

void erase_starfield(void) {
	int i;
	
	if (!starfield)
		return;
	
	if (SDL_MUSTLOCK(screen)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(screen);
	}
	
	switch(screen_bpp) {
	case 32:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint32);
			for (i = 0; i < num_stars; i++) {
				((Uint32 *) screen->pixels)[(int)starfield[i].x + ((int)starfield[i].y * pitch_adjust)] = black;
				ensure_blitted((int)starfield[i].x, (int)starfield[i].y, 1, 1);
			}
			break;
		}
	case 16:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint16);
			for (i = 0; i < num_stars; i++) {
				((Uint16 *) screen->pixels)[(int)starfield[i].x + ((int)starfield[i].y * pitch_adjust)] = black;
				ensure_blitted((int)starfield[i].x, (int)starfield[i].y, 1, 1);
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
}

int init_starfield(void) {
  int i;

  if (starfield)
    free(starfield);

  starfield = calloc(num_stars, sizeof(struct _starfield));
  assert(starfield);

  for (i = 0; i < num_stars; i++) {
    Uint16 color;
    starfield[i].x = (rand() % (scr_right - scr_left)) + scr_left;
    starfield[i].y = (rand() % (scr_bottom - scr_top)) + scr_top;
    assert(starfield[i].x >= scr_left);
    assert(starfield[i].x <= scr_right);
    assert(starfield[i].y >= scr_top);
    assert(starfield[i].y <= scr_bottom);
    starfield[i].scaler = rand() * 0.9 / RAND_MAX + 0.1;
    if (starfield[i].scaler >= 0.5)
      color = (Uint32)rander(150, 250);
    else
      color = (Uint32)rander(50, 150);
    starfield[i].color = SDL_MapRGB(screen->format, color, color, color);
  }

  return (0);
}

int uninit_starfield(void) {
  int i;

  if (!starfield)
    return (-1);

  for (i = 0; i < num_stars; i++) {
    starfield[i].x = 0;
    starfield[i].y = 0;
    starfield[i].scaler = 0;
    starfield[i].color = 0;
  }

  free(starfield);
  starfield = NULL;

  return (0);
}

void update_starfield(void) {
	static int last_camera_x = 0, last_camera_y = 0;
	float dx, dy;
	int i;

	dx = last_camera_x - camera_x;
	dy = last_camera_y - camera_y;
	last_camera_x = camera_x;
	last_camera_y = camera_y;

	for (i = 0; i < num_stars; i++) {
		starfield[i].x += dx * starfield[i].scaler;
		starfield[i].y += dy * starfield[i].scaler;

		while(starfield[i].x < scr_left) {
			int iterator = (scr_left - starfield[i].x) / (scr_right - scr_left);
			iterator++;
			starfield[i].x += (float)((scr_right - scr_left) * iterator);
		}
		while(starfield[i].y < scr_top) {
			int iterator = (scr_top - starfield[i].y) / (scr_bottom - scr_top);
			iterator++;
			starfield[i].y += (float)((scr_bottom - scr_top) * iterator);
		}
		while(starfield[i].x > scr_right) {
			int iterator = (starfield[i].x - scr_right) / (scr_right - scr_left);
			iterator++;
			starfield[i].x -= (float)((scr_right - scr_left) * iterator);
		}
		while(starfield[i].y > scr_bottom) {
			int iterator = (starfield[i].y - scr_bottom) / (scr_bottom - scr_top);
			iterator++;
			starfield[i].y -= (float)((scr_bottom - scr_top) * iterator);
		}
	}
}

static void draw_planet(struct _planet *planet) {
	SDL_Rect src, dest;

	src.x = 0;
	src.y = 0;
	src.w = planet->image->w;
	src.h = planet->image->h;
	dest.x = planet->screen_x;
	dest.y = planet->screen_y;
	dest.w = src.w;
	dest.h = src.h;

	blit_surface(planet->image, &src, &dest, 0);
}

static void erase_planets(void) {
  int i;
  
  for (i = 0; i < num_planets; i++) {
    if ((planets[i]->screen_x > -500) && (planets[i]->screen_x < (screen_width + 500)) && (planets[i]->screen_y > -500) && (planets[i]->screen_y < (screen_height + 500))) {
      erase_planet(planets[i]);
      if ((planets[i] == player.selected_planet) && (planets[i]->image))
	erase_boxed_selection(planets[i]->screen_x, planets[i]->screen_y, planets[i]->image->w, planets[i]->image->h);
    }
  }
}

static void erase_planet(struct _planet *planet) {
	SDL_Rect dest;

	dest.x = planet->screen_x;
	dest.y = planet->screen_y;
	dest.w = planet->image->w;
	dest.h = planet->image->h;

	black_fill(dest.x, dest.y, dest.w, dest.h, 0);
}

static void draw_fires(void) {
	int i;

	for (i = 0; i < MAX_FIRES; i++) {
		if ((fires[i].screen_x > -75) && (fires[i].screen_x < (screen_width + 75)) && (fires[i].screen_y > -75) && (fires[i].screen_y < (screen_height + 75)))
			if (fires[i].surface) draw_fire(&fires[i]);
	}
}

/* fixme: i dont erase the last frame of a fire that just died out */
static void erase_fires(void) {
	int i;

	for (i = 0; i < MAX_FIRES; i++) {
		if ((fires[i].screen_x > -75) && (fires[i].screen_x < (screen_width + 75)) && (fires[i].screen_y > -75) && (fires[i].screen_y < (screen_height + 75)))
			if (fires[i].surface) erase_fire(&fires[i]);
	}
}

static void draw_fire(struct _fire *fire) {
	SDL_Rect src, dest;

	assert(fire->surface);

	src.x = 0;
	src.y = 0;
	src.w = fire->surface->w;
	src.h = fire->surface->h;
	dest.x = fire->screen_x;
	dest.y = fire->screen_y;
	dest.w = src.w;
	dest.h = src.h;

	blit_surface(fire->surface, &src, &dest, 0);
}

static void erase_fire(struct _fire *fire) {
	SDL_Rect dest;

	dest.x = fire->screen_x;
	dest.y = fire->screen_y;
	dest.w = fire->surface->w;
	dest.h = fire->surface->h;

	black_fill(dest.x, dest.y, dest.w, dest.h, 0);
}

/* bresenham's oval algorithm, converted from a dos graphics book (note: designed for circles only (very last statement, ensure_blitted is the limitator on this) */
void draw_oval(int x, int y, int radius, float aspect, Uint32 color) {
	int pitch_adjust;
	int pixels[4][2];
	int col, i, row;
	float aspect_square;
	long a_square, b_square, two_a_square, two_b_square, four_a_square, four_b_square, d;

	if (screen_bpp == 32)
		pitch_adjust = screen->pitch / sizeof(Uint32);
	else if (screen_bpp == 16)
		pitch_adjust = screen->pitch / sizeof(Uint16);

	if (SDL_MUSTLOCK(screen)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(screen);
	}

	/* draw an oval */
	aspect_square = aspect * aspect;
	radius -= LINE_WIDTH / 2;
	for (i = 1; i < LINE_WIDTH; i++) {
		b_square = radius * radius;
		a_square = b_square / aspect_square;
		row = radius;
		col = 0;
		two_a_square = a_square << 1;
		four_a_square = a_square << 2;
		four_b_square = b_square << 2;
		two_b_square = b_square << 1;
		d = two_a_square * ((row - 1) * (row)) + a_square + two_b_square * (1 - a_square);
		while(a_square * (row) > b_square * (col)) {
			/* plot the pixels */
			pixels[0][0] = col+x;
			pixels[0][1] = row+y;
			pixels[1][0] = col+x;
			pixels[1][1] = y-row;
			pixels[2][0] = x-col;
			pixels[2][1] = row+y;
			pixels[3][0] = x-col;
			pixels[3][1] = y-row;
			if (screen_bpp == 32) {
				ENSURE_ON_SCREEN(pixels[0][0], pixels[0][1]) {
					((Uint32 *) screen->pixels)[pixels[0][0] + (pixels[0][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[1][0], pixels[1][1]) {
					((Uint32 *) screen->pixels)[pixels[1][0] + (pixels[1][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[2][0], pixels[2][1]) {
					((Uint32 *) screen->pixels)[pixels[2][0] + (pixels[2][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[3][0], pixels[3][1]) {
					((Uint32 *) screen->pixels)[pixels[3][0] + (pixels[3][1] * pitch_adjust)] = color;
				}
			} else if (screen_bpp == 16) {
				ENSURE_ON_SCREEN(pixels[0][0], pixels[0][1]) {
					((Uint16 *) screen->pixels)[pixels[0][0] + (pixels[0][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[1][0], pixels[1][1]) {
					((Uint16 *) screen->pixels)[pixels[1][0] + (pixels[1][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[2][0], pixels[2][1]) {
					((Uint16 *) screen->pixels)[pixels[2][0] + (pixels[2][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[3][0], pixels[3][1]) {
					((Uint16 *) screen->pixels)[pixels[3][0] + (pixels[3][1] * pitch_adjust)] = color;
				}
			}
			if (d >= 0) {
				row--;
				d -= four_a_square * (row);
			}
			d += two_b_square * (3 + (col << 1));
			col++;
		}
		d = two_b_square * (col + 1) * col + two_a_square * (row * (row - 2) + 1) + (1 - two_a_square) * b_square;
		while((row) + 1) {
			/* plot the pixels */
			pixels[0][0] = col+x;
			pixels[0][1] = row+y;
			pixels[1][0] = col+x;
			pixels[1][1] = y-row;
			pixels[2][0] = x-col;
			pixels[2][1] = row+y;
			pixels[3][0] = x-col;
			pixels[3][1] = y-row;
			if (screen_bpp == 32) {
				ENSURE_ON_SCREEN(pixels[0][0], pixels[0][1]) {
					((Uint32 *) screen->pixels)[pixels[0][0] + (pixels[0][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[1][0], pixels[1][1]) {
					((Uint32 *) screen->pixels)[pixels[1][0] + (pixels[1][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[2][0], pixels[2][1]) {
					((Uint32 *) screen->pixels)[pixels[2][0] + (pixels[2][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[3][0], pixels[3][1]) {
					((Uint32 *) screen->pixels)[pixels[3][0] + (pixels[3][1] * pitch_adjust)] = color;
				}
			} else if (screen_bpp == 16) {
				ENSURE_ON_SCREEN(pixels[0][0], pixels[0][1]) {
					((Uint16 *) screen->pixels)[pixels[0][0] + (pixels[0][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[1][0], pixels[1][1]) {
					((Uint16 *) screen->pixels)[pixels[1][0] + (pixels[1][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[2][0], pixels[2][1]) {
					((Uint16 *) screen->pixels)[pixels[2][0] + (pixels[2][1] * pitch_adjust)] = color;
				}
				ENSURE_ON_SCREEN(pixels[3][0], pixels[3][1]) {
					((Uint16 *) screen->pixels)[pixels[3][0] + (pixels[3][1] * pitch_adjust)] = color;
				}
			}
			/* correct above code for book */
			if (d <= 0) {
				col++;
				d += four_b_square * col;
			}
			row--;
			d += two_a_square * (3 - (row << 1));
		}
		radius++;
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	ensure_blitted(x - radius, y - radius, radius * 2, radius * 2);
}

void init_colors(void) {
	black = map_rgb(0, 0, 0);
	white = map_rgb(255, 255, 255);
	grey = map_rgb(130, 130, 130);
	blue = map_rgb(0, 0, 255);
	dark_grey = map_rgb(77, 77, 77);
	green = map_rgb(0, 255, 0);
	dark_green = map_rgb(0, 150, 0);
	light_blue = map_rgb(0, 119, 220);
	yellow = map_rgb(255, 255, 0);
	red = map_rgb(255, 0, 0);
	orange = map_rgb(255, 180, 0);
	dark_cyan = map_rgb(0, 103, 125);
}

/* Following function (C) 2002 Jared Minch, released under GNU GPL version 2 (see LICENSE for a copy), thanks aardvark! */
/* Rotates 8-, 16-, and 32-bit sprites.  24-bit sprites aren't handled; you
 * must convert to one of these color depths before calling this function.
 * */
SDL_Surface *rotate( SDL_Surface *s, float ang )
{
	/* Quadrant */
	short q;
	float offset_ang;

	/* Destination surface */
	SDL_Surface *dst;
	/* Destination surface size */
	short dstw, dsth;
	fixed dstwf, dsthf;
	fixed srcwf, srchf;

	/* Precalculated cos and sin */
	float cosang, sinang;

	/* Corner points */
	short corner_top, corner_left, corner_right, corner_bottom;

	/* source step values for each side (How far to move along the source side
	 * for each vertical pixel) */
	fixed src_step_top, src_step_side;

	/* Source step values per horizontal pixel */
	fixed src_step_x, src_step_y;

	/* destination step values (how far to move horizontally for each vertical
	 * pixel) */
	fixed dst_step_0, dst_step_1;

	/* Fixed and integer location source pixel locations */
	fixed src_left;
	fixed xf, yf;
	int xi, yi, xnew, ynew, dx, dy;

	/* Pointer step values */
	int psx, psy;

	/* Destination line and remaining line length */
	fixed dstl_left, dstl_right;
	int dsty, dstx;

	/* Maximum line length (Calculated to prevent accumulated error to cause
	 * an overrun) */
	int maxx;

	/* Source and destination pointers */
	char *sp;
	char *dp, *dlp, *dfp;

	/* Current side segments: = 0 for first segment, = 1 for second */
	short seg_left, seg_right;

	char bpp;

	bpp = s->format->BytesPerPixel;

	ang = ((ang * M_PI) / 180.0); /* convert to radians, line added by Chris Thielen under author's permission */

	cosang = cosf(ang);
	sinang = sinf(ang);

	/* Determine the quadrant (0 - 3) */
	if(sinang >= 0) {
		q = (cosang >= 0) ? 0 : 1;
	} else {
		q = (cosang >= 0) ? 3 : 2;
	}

	offset_ang = ang - (M_PI_2 * (float)q);

	/* Determine the size of the rotated surface, and the corner points */
	if(q == 0 || q == 2) {
		corner_top = (s->w - 1) * cosf(offset_ang);
		corner_left = (s->w - 1) * sinf(offset_ang);
		corner_right = (s->h - 1) * cosf(offset_ang);
		corner_bottom = (s->h - 1) * sinf(offset_ang);
	} else {
		corner_top = (s->h - 1) * cosf(offset_ang);
		corner_left = (s->h - 1) * sinf(offset_ang);
		corner_right = (s->w - 1) * cosf(offset_ang);
		corner_bottom = (s->w - 1) * sinf(offset_ang);
	}

	/* Width = w cos * h sin; Height = h cos * w sin */
	dstw = corner_top + corner_bottom + 1;
	dsth = corner_left + corner_right + 1;
	dstwf = itofix(dstw - 1);
	dsthf = itofix(dsth - 1);

	srcwf = itofix(s->w - 1);
	srchf = itofix(s->h - 1);

	/* Create the destination surface */
	dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstw, dsth, 
								s->format->BitsPerPixel,
								s->format->Rmask, s->format->Gmask,
								s->format->Bmask, s->format->Amask);
	SDL_SetColorKey(dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(dst->format, 0, 0, 0));

//      aotf_assert(ERR_FATAL, dst,
//                                      "Allocation of new zoomed surface failed.");

	/* Calculate step values */
	src_step_x = ftofix(cosang);
	src_step_y = ftofix(sinang);
	src_step_top = -ftofix(1.0 / sinang);
	src_step_side = ftofix(1.0 / cosang);
	dst_step_0 = ftofix(1.0 / tanf(offset_ang));
	dst_step_1 = ftofix(tanf(offset_ang));

	psx = bpp;
	psy = s->pitch;

	/* All right; do the actual rotation copy */
	SDL_LockSurface(dst);
	SDL_LockSurface(s);
	seg_left = seg_right = 0;
	dlp = dst->pixels;
	dstl_left = dstl_right = itofix(corner_top);

	switch(q) {
		case 0:
			src_left = srcwf;
			break;
		case 1:
			src_left = srchf;
		break;
		case 2:
			src_left = 0;
		break;
		case 3:
			src_left = 0;
		break;
	}

	for(dsty = 0; dsty < dsth; dsty++, dlp += dst->pitch) {

		/* If at a corner point, reset pointers and update seg_foo */
		if(dsty == corner_left) {
			dstl_left = 0;
			seg_left = 1;
			switch(q) {
				case 0:
					src_left = 0;
					break;
				case 1:
					src_left = srcwf;
					break;
				case 2:
					src_left = srchf;
					break;
				case 3:
					src_left = 0;
					break;
			}
		}

		if(dsty == corner_right) {
			dstl_right = dstwf;
			seg_right = 1;
		}

		/* Set the destination pointer */
		dp = dlp + fixtoi(dstl_left) * bpp;

		/* Set up source pointer and values */
		if(!seg_left) {
			switch(q) {
				case 0:
					xf = src_left;
					yf = 0;
					break;
				case 1:
					xf = srcwf;
					yf = src_left;
					break;
				case 2:
					xf = src_left;
					yf = srchf;
					break;
				case 3:
					xf = 0;
					yf = src_left;
					break;
			}
		} else {
			switch(q) {
				case 0:
					xf = 0;
					yf = src_left;
					break;
				case 1:
					xf = src_left;
					yf = 0;
					break;
				case 2:
					xf = srcwf;
					yf = src_left;
					break;
				case 3:
					xf = src_left;
					yf = srchf;
					break;
			}
		}

		/* Determine the line length */
		dstx = fixtoi(dstl_right - dstl_left) + 1;

		if(src_step_x) {
			if(src_step_x > 0)
				maxx = fixdiv((srcwf - xf), src_step_x) >> 16;
			else
				maxx = fixdiv(xf, -src_step_x) >> 16;

			if(maxx < dstx) dstx = maxx;
		}

		if(src_step_y) {
			if(src_step_y > 0)
				maxx = fixdiv((srchf - yf), src_step_y) >> 16;
			else
				maxx = fixdiv(yf, -src_step_y) >> 16;

			if(maxx < dstx) dstx = maxx;
		}

		xi = fixtoi(xf);  yi = fixtoi(yf);

		sp = (char *)s->pixels + xi * psx + yi * psy;

		/* Find the boundary bytes for the destination */
		dfp = dp + dstx * bpp;

		/* Copy from the source to the destination */
		while(dp < dfp) {
			switch(bpp) {
				case 1:
					*(Uint8 *)dp = *(Uint8 *)sp;
					break;
				case 2:
					*(Uint16 *)dp = *(Uint16 *)sp;
					break;
				case 4:
					*(Uint32 *)dp = *(Uint32 *)sp;
					break;
			}

			xf += src_step_x;  yf += src_step_y;
			xnew = fixtoi(xf);  ynew = fixtoi(yf);
			dx = xnew - xi;  dy = ynew - yi;
			sp += (dx * psx) + (dy * psy);
			xi = xnew;  yi = ynew;
			dp += bpp;
		}

		/* The following if block is identical to the following commented out
		 * block.  This is wordier but more efficient. */
		if(q == 0 || q == 2) {
			if(seg_left) {
				dstl_left += dst_step_1;
				src_left += src_step_side;
			} else {
				dstl_left -= dst_step_0;
				src_left += src_step_top;
			}
		} else {
			if(seg_left) {
				dstl_left += dst_step_1;
				src_left += src_step_top;
			} else {
				dstl_left -= dst_step_0;
				src_left += src_step_side;
			}
		}

#if 0
		dstl_left += seg_left ? (dst_step_1) : (-dst_step_0);
		if(q == 0 || q == 2)
			src_left += seg_left ? src_step_side : src_step_top;
		else 
			src_left += seg_left ? src_step_top : src_step_side;
#endif
		dstl_right += seg_right ? (-dst_step_0) : (dst_step_1);

		/* Make sure we're staying on the surface */
		/* OPTIMIZEME */
#if 0
		if(dstl_left < 0) dstl_left = 0;
		if(dstl_right < 0) dstl_right = 0;
		if(dstl_left > dstwf) dstl_left = dstwf;
		if(dstl_right > dstwf) dstl_right = dstwf;
		if(src_left < 0) src_left = 0;
		if(src_left > srcwf) src_left = srcwf;
#endif

	}

	/* Finished; unlock surfaces */
	SDL_UnlockSurface(dst);
	SDL_UnlockSurface(s);

	return dst;
}

void draw_rect(int x, int y, int w, int h, Uint32 color) {
	SDL_Rect rect;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	fill_rect(&rect, color);
}

void erase_boxed_selection(int x, int y, int w, int h) {
  SDL_Rect rect;
  
  /* check to make sure planet is on-screen first */
  if ((x + w) < scr_left)
    return;
  if (x > scr_right)
    return;
  if ((y + h) < scr_top)
    return;
  if (y > scr_bottom)
    return;
  
  /* erase the planet selection cursor */
  /* draw upper left cursor */
  rect.x = x;
  rect.y = y;
  rect.w = w / 6;
  rect.h = 1;
  fill_rect(&rect, black);
  rect.w = 1;
  rect.h = h / 6;
  fill_rect(&rect, black);
  
  /* draw upper right cursor */
  rect.x = x + w - (w / 6);
  rect.w = w / 6;
  rect.h = 1;
  fill_rect(&rect, black);
  rect.x = x + w;
  rect.w = 1;
  rect.h = h / 6;
  fill_rect(&rect, black);
  
  /* draw lower left cursor */
  rect.x = x;
  rect.y = y + h;
  rect.w = w / 6;
  rect.h = 1;
  fill_rect(&rect, black);
  rect.y -= h / 6;
  rect.w = 1;
  rect.h = h / 6;
  fill_rect(&rect, black);
  
  /* draw lower right cursor */
  rect.x = x + w - (w / 6);
  rect.y = y + h;
  rect.w = w / 6;
  rect.h = 1;
  fill_rect(&rect, black);
  rect.x = x + w;
  rect.y = y + h - (h / 6);
  rect.w = 1;
  rect.h = h / 6;
  fill_rect(&rect, black);
}

static void draw_boxed_selection(int x, int y, int w, int h) {
  SDL_Rect rect;
  
  /* check to make it is on-screen first */
  if ((x + w) < scr_left)
    return;
  if (x > scr_right)
    return;
  if ((y + h) < scr_top)
    return;
  if (y > scr_bottom)
    return;
  
  /* draw the planet selection cursor */
  /* draw upper left cursor */
  rect.x = x;
  rect.y = y;
  rect.w = w / 6;
  rect.h = 1;
  fill_rect4(rect.x, rect.y, rect.w, rect.h, green);
  rect.w = 1;
  rect.h = h / 6;
  fill_rect4(rect.x, rect.y, rect.w, rect.h, green);
  
  /* draw upper right cursor */
  rect.x = x + w - (w / 6);
  rect.w = w / 6;
  rect.h = 1;
  fill_rect4(rect.x, rect.y, rect.w, rect.h, green);
  rect.x = x + w;
  rect.w = 1;
  rect.h = h / 6;
  fill_rect4(rect.x, rect.y, rect.w, rect.h, green);
  
  /* draw lower left cursor */
  rect.x = x;
  rect.y = y + h;
  rect.w = w / 6;
  rect.h = 1;
  fill_rect4(rect.x, rect.y, rect.w, rect.h, green);
  rect.y -= h / 6;
  rect.w = 1;
  rect.h = h / 6;
  fill_rect4(rect.x, rect.y, rect.w, rect.h, green);
  
  /* draw lower right cursor */
  rect.x = x + w - (w / 6);
  rect.y = y + h;
  rect.w = w / 6;
  rect.h = 1;
  fill_rect4(rect.x, rect.y, rect.w, rect.h, green);
  rect.x = x + w;
  rect.y = y + h - (h / 6);
  rect.w = 1;
  rect.h = h / 6;
  fill_rect4(rect.x, rect.y, rect.w, rect.h, green);
}

static void draw_gates_bottom(void) {
	int i;
	SDL_Rect src, dest;

	for (i = 0; i < num_gates; i++) {
		if ((gates[i]->screen_x > -120) && (gates[i]->screen_x < (screen_width + 120)) && (gates[i]->screen_y > -120) && (gates[i]->screen_y < (screen_height + 120))) {
			/* jump gate is onscreen */
			if (gates[i]->top_surface == NULL) {
				/* the temporary picture, only in ram while the gate is nearby, isnt available, so fetch it */
				SDL_Surface *temp;
				SDL_Surface *gate_bottom, *gate_top;

				/* load in the bottom portion */
				temp = eaf_load_png(main_eaf, gates[i]->bottom_image);
				if (temp == NULL)
					temp = eaf_load_png(loaded_eaf, gates[i]->bottom_image);

				if (temp == NULL) {
					fprintf(stdout, "could not load %s for on-the-fly gate image\n", gates[i]->bottom_image);
					break;
				}

				gate_bottom = SDL_DisplayFormatAlpha(temp);
				SDL_FreeSurface(temp);

				/* rotate the gate image to face the planet it needs to be pointing towards */
				gates[i]->bottom_surface = rotate(gate_bottom, gates[i]->angle);
				SDL_FreeSurface(gate_bottom);

				/* load in the top portion */
				temp = eaf_load_png(main_eaf, gates[i]->top_image);
				if (temp == NULL)
					temp = eaf_load_png(loaded_eaf, gates[i]->top_image);

				if (temp == NULL) {
					fprintf(stdout, "could not load %s for on-the-fly gate image\n", gates[i]->top_image);
					break;
				}

				gate_top = SDL_DisplayFormatAlpha(temp);
				SDL_FreeSurface(temp);

				/* rotate the gate image to face the planet it needs to be pointing towards */
				gates[i]->top_surface = rotate(gate_top, gates[i]->angle);
				SDL_FreeSurface(gate_top);
			}

			/* now that we know the gate is on-screen and available in ram (the two surfaces), let's blit! */

			/* blit the bottom portion */
			src.x = 0;
			src.y = 0;
			src.w = gates[i]->bottom_surface->w;
			src.h = gates[i]->bottom_surface->h;
			dest.x = gates[i]->screen_x - (src.w / 2); /* be sure to center the image on the coords */
			dest.y = gates[i]->screen_y - (src.h / 2);
			dest.w = src.w;
			dest.h = src.h;
			blit_surface(gates[i]->bottom_surface, &src, &dest, 0);
		}
	}
}

static void draw_gates_top(void) {
	int i;
	SDL_Rect src, dest;

	for (i = 0; i < num_gates; i++) {
		if ((gates[i]->screen_x > -120) && (gates[i]->screen_x < (screen_width + 120)) && (gates[i]->screen_y > -120) && (gates[i]->screen_y < (screen_height + 120))) {
			/* jump gate is onscreen */
			if (gates[i]->top_surface == NULL) {
				/* the temporary picture, only in ram while the gate is nearby, isnt available, so fetch it */
				SDL_Surface *temp;
				SDL_Surface *gate_bottom, *gate_top;

				/* load in the bottom portion */
				temp = eaf_load_png(main_eaf, gates[i]->bottom_image);
				if (temp == NULL)
					temp = eaf_load_png(loaded_eaf, gates[i]->bottom_image);

				if (temp == NULL) {
					fprintf(stdout, "could not load %s for on-the-fly gate image\n", gates[i]->bottom_image);
					break;
				}

				gate_bottom = SDL_DisplayFormatAlpha(temp);
				SDL_FreeSurface(temp);

				/* rotate the gate image to face the planet it needs to be pointing towards */
				gates[i]->bottom_surface = rotate(gate_bottom, gates[i]->angle);
				SDL_FreeSurface(gate_bottom);

				/* load in the top portion */
				temp = eaf_load_png(main_eaf, gates[i]->top_image);
				if (temp == NULL)
					temp = eaf_load_png(loaded_eaf, gates[i]->top_image);

				if (temp == NULL) {
					fprintf(stdout, "could not load %s for on-the-fly gate image\n", gates[i]->top_image);
					break;
				}

				gate_top = SDL_DisplayFormatAlpha(temp);
				SDL_FreeSurface(temp);

				/* rotate the gate image to face the planet it needs to be pointing towards */
				gates[i]->top_surface = rotate(gate_top, gates[i]->angle);
				SDL_FreeSurface(gate_top);
			}

			/* now that we know the gate is on-screen and available in ram (the two surfaces), let's blit! */

			/* blit the top portion */
			src.x = 0;
			src.y = 0;
			src.w = gates[i]->top_surface->w;
			src.h = gates[i]->top_surface->h;
			dest.x = gates[i]->screen_x - (src.w / 2);
			dest.y = gates[i]->screen_y - (src.h / 2);
			dest.w = src.w;
			dest.h = src.h;
			blit_surface(gates[i]->top_surface, &src, &dest, 0);
		}

		/* if this is the player's target, we need to draw a recticle */
		if (player.target.type == TARGET_GATE) {
		  struct _gate *gate = NULL;

		  gate = (struct _gate *)player.target.target;

		  if (gate == gates[i]) {
		    /* this gate is the player's target, so draw the reticle */
		    if (gate->top_surface)
		      draw_boxed_selection(gate->screen_x - (gate->top_surface->w / 2), gate->screen_y - (gate->top_surface->h / 2), gate->top_surface->w, gate->top_surface->h);
		  }
		}
	}	
}

static void erase_gates(void) {
	int i;

	for (i = 0; i < num_gates; i++) {
		if ((gates[i]->screen_x > -120) && (gates[i]->screen_x < (screen_width + 120)) && (gates[i]->screen_y > -120) && (gates[i]->screen_y < (screen_height + 120))) {
			SDL_Rect rect;
			/* if erase frame is called before draw frame, which occurs once in the game, top_surface wont exist, so skip it for that loop */
			if (gates[i]->top_surface != NULL) {
				/* jump gate is onscreen */
				rect.w = gates[i]->top_surface->w;
				rect.h = gates[i]->top_surface->h;
				rect.x = gates[i]->screen_x - (rect.w / 2);
				rect.y = gates[i]->screen_y - (rect.h / 2);
				black_fill(rect.x, rect.y, rect.w, rect.h, 0);
			}
		} else {
			/* make sure gate images arent loaded; if they are, free them */
			if (gates[i]->top_surface != NULL) {
				SDL_FreeSurface(gates[i]->top_surface);
				gates[i]->top_surface = NULL;
			}
			if (gates[i]->bottom_surface != NULL) {
				SDL_FreeSurface(gates[i]->bottom_surface);
				gates[i]->bottom_surface = NULL;
			}
		}

		/* if this is the player's target, we need to erase the recticle */
		if (player.target.type == TARGET_GATE) {
		  struct _gate *gate = NULL;

		  gate = (struct _gate *)player.target.target;

		  if (gate == gates[i]) {
		    /* this gate is the player's target, so erase the reticle */
		    if (gate->top_surface)
		      erase_boxed_selection(gate->screen_x - (gate->top_surface->w / 2), gate->screen_y - (gate->top_surface->h / 2), gate->top_surface->w, gate->top_surface->h);
		  }
		}
	}
}

void blit_image(SDL_Surface *surface, int x, int y) {
	SDL_Rect src, dest;

	if (surface == NULL)
		return;

	src.x = 0;
	src.y = 0;
	src.w = surface->w;
	src.h = surface->h;
	dest.x = x;
	dest.y = y;
	dest.w = src.w;
	dest.h = src.h;

	blit_surface(surface, &src, &dest, 1);
}

/* setup various parameters that need setting up for new games/scenarios */
void init_video_new_game(void) {
	hud_draw_first_time = 1;
}

/* returns 0 on success, -1 on failure (too many rects already usually) */
/*
int ensure_blitted(short int x, short int y, short int w, short int h) {

	if (blits_done < MAX_BLIT_RECTS) {

		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (x >= 800)
			return (-1);
		if (y >= 600)
			return (-1);
		if ((x + w) >= 800)
			w = 799 - x;
		if ((y + h) >= 600)
			h = 599 - y;

		blitted_rects[blits_done].x = x;
		blitted_rects[blits_done].y = y;
		blitted_rects[blits_done].w = w;
		blitted_rects[blits_done].h = h;
		blits_done++;

		return (0);
	} else {
		thrown_away_blits++;
	}

	return (-1);
}
*/

/* copies a portion of a surface onto a new surface and returns that new surface */
SDL_Surface *get_surface(SDL_Surface *surface, int x, int y, int w, int h) {
	SDL_Surface *returned = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	SDL_Rect src, dest;

	assert(returned);

	src.x = x;
	src.y = y;
	src.w = w;
	src.h = h;
	dest.x = 0;
	dest.y = 0;
	dest.w = w;
	dest.h = h;

	SDL_BlitSurface(surface, &src, returned, &dest);

	return (returned);
}

/* functions to draw an arc, drawArc() is the only one that should be called */

/* note: assumes surface is already locked if need be */
#ifdef WIN32
_inline void plotArc1(SDL_Surface *surface, int pitch_adjust, int x, int y, int sector, Uint32 color, int arcTest[], int x_start_test, int x_end_test) {
#else
inline void plotArc1(SDL_Surface *surface, int pitch_adjust, int x, int y, int sector, Uint32 color, int arcTest[], int x_start_test, int x_end_test) {
#endif
	if (arcTest[sector] == 0)
		return;
	if (arcTest[sector] == 2) {
		if (surface->format->BitsPerPixel == 32) {
			ENSURE_ON_SCREEN(x, y)
				((Uint32 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		} else if (surface->format->BitsPerPixel == 16) {
			ENSURE_ON_SCREEN(x, y)
				((Uint16 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		}
	}
	if ((arcTest[sector] == 1) && (x >= x_start_test)) {
		if (surface->format->BitsPerPixel == 32) {
			ENSURE_ON_SCREEN(x, y)
				((Uint32 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		} else if (surface->format->BitsPerPixel == 16) {
			ENSURE_ON_SCREEN(x, y)
				((Uint16 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		}
	}
	if ((arcTest[sector] == 3) && (x <= x_end_test)) {
		if (surface->format->BitsPerPixel == 32) {
			ENSURE_ON_SCREEN(x, y)
				((Uint32 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		} else if (surface->format->BitsPerPixel == 16) {
			ENSURE_ON_SCREEN(x, y)
				((Uint16 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		}
	}
	if ((arcTest[sector] == 4) && (x >= x_start_test) && (x <= x_end_test)) {
		if (surface->format->BitsPerPixel == 32) {
			ENSURE_ON_SCREEN(x, y)
				((Uint32 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		} else if (surface->format->BitsPerPixel == 16) {
			ENSURE_ON_SCREEN(x, y)
				((Uint16 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		}
	}
}

#ifdef WIN32
_inline void plotArc2(SDL_Surface *surface, int pitch_adjust, int x, int y, int sector, Uint32 color, int arcTest[], int x_start_test, int x_end_test) {
#else
inline void plotArc2(SDL_Surface *surface, int pitch_adjust, int x, int y, int sector, Uint32 color, int arcTest[], int x_start_test, int x_end_test) {
#endif
	if (arcTest[sector] == 0)
		return;
	if (arcTest[sector] == 2) {
		if (surface->format->BitsPerPixel == 32) {
			ENSURE_ON_SCREEN(x, y)
				((Uint32 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		} else if (surface->format->BitsPerPixel == 16) {
			ENSURE_ON_SCREEN(x, y)
				((Uint16 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		}
	}
	if ((arcTest[sector] == 1) && (x <= x_start_test)) {
		if (surface->format->BitsPerPixel == 32) {
			ENSURE_ON_SCREEN(x, y)
				((Uint32 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		} else if (surface->format->BitsPerPixel == 16) {
			ENSURE_ON_SCREEN(x, y)
				((Uint16 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		}
	}
	if ((arcTest[sector] == 3) && (x >= x_end_test)) {
		if (surface->format->BitsPerPixel == 32) {
			ENSURE_ON_SCREEN(x, y)
				((Uint32 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		} else if (surface->format->BitsPerPixel == 16) {
			ENSURE_ON_SCREEN(x, y)
				((Uint16 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		}
	}
	if ((arcTest[sector] == 4) && (x >= x_end_test) && (x <= x_start_test)) {
		if (surface->format->BitsPerPixel == 32) {
			ENSURE_ON_SCREEN(x, y)
				((Uint32 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		} else if (surface->format->BitsPerPixel == 16) {
			ENSURE_ON_SCREEN(x, y)
				((Uint16 *) surface->pixels)[x + (y * pitch_adjust)] = color;
		}
	}
}

void drawArc(SDL_Surface *surface, int xc, int yc, int b, int start_angle, int end_angle, Uint32 color, float aspect) {
	const float pi = 3.14159265;
	int col, i, j, row, start_sector, end_sector, x_start_test, x_end_test;
	int arcTest[9] = {0};
	float aspect_square;
	long a_square, b_square, two_a_square, two_b_square, four_a_square, four_b_square, d;
	int pitch_adjust;

	return;

	start_angle *= 10;
	end_angle *= 10;

	if (surface->format->BitsPerPixel == 32)
		pitch_adjust = screen->pitch / sizeof(Uint32);
	else if (surface->format->BitsPerPixel == 16)
		pitch_adjust = screen->pitch / sizeof(Uint16);

	if (SDL_MUSTLOCK(surface)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(surface);
	}

	aspect_square = aspect * aspect;
	b -= LINE_WIDTH / 2;

	for (j = 1; j <= LINE_WIDTH; j++) {
		b_square = b * b;
		a_square = b_square / aspect_square;
		row = b;
		col = 0;
		two_a_square = a_square << 1;
		four_a_square = a_square << 2;
		four_b_square = b_square << 2;
		two_b_square = b_square << 1;
		d = two_a_square * ((row - 1)*(row)) + a_square + two_b_square * (1 - a_square);
		start_sector = start_angle / 450;
		end_sector = end_angle / 450;
		x_start_test = xc + sqrt(a_square) * cos(start_angle * pi/1800);
		x_end_test = xc + sqrt(a_square) * cos(end_angle * pi / 1800);
		if (start_sector == end_sector)
			arcTest[start_sector] = 4;
		else
		{
			arcTest[start_sector] = 1;
			arcTest[end_sector] = 3;
			for (i = start_sector + 1; i != end_sector; i++) {
				arcTest[i] = 2;
				if (i == 8)
					i = -1;
			}
		}
		while(a_square*(row) > b_square * (col)) {
			plotArc1(surface, pitch_adjust, xc+col, yc+row, 6, color, arcTest, x_start_test, x_end_test);
			plotArc2(surface, pitch_adjust, xc+col, yc-row, 1, color, arcTest, x_start_test, x_end_test);
			plotArc1(surface, pitch_adjust, xc-col, yc+row, 5, color, arcTest, x_start_test, x_end_test);
			plotArc2(surface, pitch_adjust, xc-col, yc-row, 2, color, arcTest, x_start_test, x_end_test);
			if (d >= 0) {
				row--;
				d -= four_a_square * (row);
			}
			d += two_b_square * (3 + (col << 1));
			col++;
		}
		d = two_b_square * (col + 1) * col + two_a_square * (row * (row - 2) + 1) + (1-two_a_square) * b_square;
		while((row) + 1) {
			plotArc1(surface, pitch_adjust, xc+col, yc+row, 7, color, arcTest, x_start_test, x_end_test);
			plotArc2(surface, pitch_adjust, xc+col, yc-row, 0, color, arcTest, x_start_test, x_end_test);
			plotArc1(surface, pitch_adjust, xc-col, yc+row, 4, color, arcTest, x_start_test, x_end_test);
			plotArc2(surface, pitch_adjust, xc-col, yc-row, 3, color, arcTest, x_start_test, x_end_test);
			if (d <= 0) {
				col++;
				d += four_b_square*col;
			}
			row--;
			d += two_a_square * (3 - (row << 1));
		}
		b++;
	}

	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);

	/* ensure it gets blitted, just, grab a rectangle around the entire thing */
	ensure_blitted(xc - b, yc - b, xc + b, yc + b);
}

SDL_Surface *load_image(char *filename, Uint32 bitfield) {
  SDL_Surface *temp, *final;
  
  if (filename == NULL) {
    fprintf(stdout, "Could not load NULL file!\n");
    return (NULL);
  }
  
  temp = IMG_Load(filename);
  if (temp == NULL) {
    fprintf(stdout, "Could not load \"%s\".\n", filename);
    exit(-1);
  }
  
  if (bitfield == BLUE_COLORKEY)
    SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(temp->format, 0, 0, 255));
  if (bitfield == BLACK_COLORKEY)
    SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(temp->format, 0, 0, 0));

  if (bitfield == ALPHA)
    final = SDL_DisplayFormatAlpha(temp);
  else
    final = SDL_DisplayFormat(temp);
  
  SDL_FreeSurface(temp);
  
  return (final);
}

/* loads an image out of a eaf file */
SDL_Surface *load_image_eaf(FILE *eaf, char *filename, Uint32 bitfield) {
	SDL_Surface *temp, *final;

	if (filename == NULL) {
		fprintf(stdout, "Could not load NULL file!\n");
		return (NULL);
	}

	temp = eaf_load_png(eaf, filename);
	if (temp == NULL) {
		return (NULL);
	}

	if (bitfield == BLUE_COLORKEY)
		SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(temp->format, 0, 0, 255));
	if (bitfield == BLACK_COLORKEY)
		SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(temp->format, 0, 0, 0));

	if (bitfield == ALPHA)
		final = SDL_DisplayFormatAlpha(temp);
	else
		final = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	return (final);
}
