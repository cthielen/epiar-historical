#ifndef H_VIDEO
#define H_VIDEO

#include "sprite/planet.h"

/* common macros */
#define ENSURE_ON_SCREEN(x, y) if (((x) >= 0) && ((y) >= 0) && ((x) < 800) && ((y) < 600))

#define BLUE_COLORKEY  1
#define ALPHA          2
#define BLACK_COLORKEY 3

extern SDL_Surface *screen;

extern Uint32 black, white, grey, blue, dark_grey, green, dark_green, light_blue, yellow, red, orange, dark_cyan;
extern Uint8 screen_bpp;
extern int scr_top, scr_bottom, scr_left, scr_right; /* virtual screen size */
extern unsigned char fullscreen;
extern int num_stars; /* number of stars in the starfield */

/* video system routines */
/* basic routines */
int setup_video(int width, int height, int bpp, unsigned char fullscreen);
void cleanup_video(void);
int blit_surface(SDL_Surface *surface, SDL_Rect *psrc, SDL_Rect *dest, unsigned char hud_blit);
int flip(void);
void black_fill(int x, int y, int width, int height, unsigned char hud_blit);
void black_fill_rect(SDL_Rect *rect, unsigned char hud_blit);
void fill_rect4(int x, int y, int w, int h, Uint32 color);
void init_colors(void);
void set_dimensions(int top, int bottom, int left, int right); /* set the virtual screen dimensions */
Uint32 map_rgb(int r, int g, int b);
void fill_rect(SDL_Rect *dest, Uint32 color);
void blit_image(SDL_Surface *surface, int x, int y);
SDL_Surface *get_surface(SDL_Surface *surface, int x, int y, int w, int h);

/* complex routines */
SDL_Surface *rotate_surface(SDL_Surface *surface, float angle); /* deprecated - buggy */
SDL_Surface *rotate( SDL_Surface *s, float ang ); /* function contributed by Jared Minch */

/* common routines */
void erase_frame(void);
void draw_frame(unsigned char force_full_redraw);
void init_video_new_game(void); /* does init for video on a new game/scenario */

/*
int ensure_blitted(short int x, short int y, short int w, short int h);
*/

#define ensure_blitted(x, y, w, h)

SDL_Surface *load_image(char *filename, Uint32 bitfield);
SDL_Surface *load_image_eaf(FILE *eaf, char *filename, Uint32 bitfield);

/* graphics primitives routines */
void draw_line(int x1, int y1, int x2, int y2, Uint32 color);
void draw_oval(int x, int y, int radius, float aspect, Uint32 color);
void draw_rect(int x, int y, int w, int h, Uint32 color);
void drawArc(SDL_Surface *surface, int xc, int yc, int b, int start_angle, int end_angle, Uint32 color, float aspect);

/* specialized routines */
int init_starfield(void);
int uninit_starfield(void);
void update_starfield(void);
void draw_starfield(void);
void erase_starfield(void);

/* the routines formerly known as functions */
void erase_boxed_selection(int x, int y, int w, int h);

/* exported functions (video-only functions that need calling rarely in other places) */
void erase_ship(struct _ship *ship);

#endif /* H_VIDEO */
