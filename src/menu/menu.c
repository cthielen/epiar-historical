#include "audio/music.h"
#include "com_defs.h"
#include "game/game.h"
#include "game/scenario.h"
#include "gui/gui.h"
#include "includes.h"
#include "input/input.h"
#include "menu/menu.h"
#include "menu/options.h"
#include "system/eaf.h"
#include "system/path.h"
#include "system/video/video.h"
#include "tutorial/tutorial.h"

SDL_Surface *background, *new_game, *load_game, *tutorial, *configure, *leave_epiar, *bak, *bak_text, *new_game_h, *load_game_h, *tutorial_h, *configure_h, *leave_epiar_h;

Uint8 alphas[5] = { 0 };

int mouse_x, mouse_y;
typedef enum _menu_selection {NOTHING, NEW_GAME, LOAD_GAME, TUTORIAL, CONFIGURE, LEAVE} menu_selection;
unsigned char menu_update;

void load_background(void);
void load_menu_text(void);
menu_selection menu_get_input(void);
void menu_update_frame(void);
void unload_menu_text(void);
unsigned char mouse_over(SDL_Surface *surface, int x, int y);
void adjust_alpha(int which);
void set_initial_alpha(void);
void fade_in(SDL_Surface *surface, Uint32 length);
static void display_logo_eaf(char *file);

void menu(void) {
  unsigned char iterate = 1;
  
  menu_selection loop_cond;
  
  load_music("audio/music/menu.ogg");
  start_music();
  
  if (!skip_intro) {
    /* display the entropy logo */
    display_logo_eaf("misc/entropy.png");
    
    SDL_Delay(200);
  }
  
  load_background();
  load_menu_text();
  
  /* fade in background image */
  fade_in(background, 500);
  
  set_initial_alpha();
  
  menu_update = 1;
  
  draw_background();
  draw_menu_text();
  
  SDL_ShowCursor(1);
  
  clear_events();
  
  /* menu main loop */
  while(iterate) {
    
    loop_cond = NOTHING;
    
    while(!loop_cond) {
      if (menu_update) {
	menu_draw_frame();
	flip();
	menu_update = 0;
      }
      loop_cond = menu_get_input();
      menu_update_frame();
      SDL_Delay(30);
    }
    
    if (loop_cond == NEW_GAME) {
      ep_scenario *scen = NULL;
      scen = do_scenario_select();
      if (scen) {
	do_scenario(scen); /* do_scenario() will close the ep_scenario struct, we dont have to */
      } else {
	gui_alert("Could not load scenario file!");
      }
      
      draw_background();
    } else if (loop_cond == LOAD_GAME) {
      gui_alert("Loading feature not implemented yet.");
      draw_background();
    } else if (loop_cond == TUTORIAL) {
      gui_alert("Tutorial not implemented yet.");
      draw_background();
      do_tutorial();
    } else if (loop_cond == CONFIGURE) {
      options(0);
      draw_background();
    } else if (loop_cond == LEAVE)
      iterate = 0;
    
    menu_update = 1;
    
    SDL_ShowCursor(1);
  }
  
  unload_menu_text();
  SDL_FreeSurface(background);
  SDL_FreeSurface(bak);
  SDL_FreeSurface(bak_text);
}

/* loads the menu background */
void load_background(void) {
	char bg[23];
	SDL_Surface *temp;
	SDL_Rect src, dest;

	sprintf(bg, "menu/%d.png", (rand() % 5) + 1);
	temp = eaf_load_png(epiar_eaf, bg);
	if (temp == NULL) {
		fprintf(stdout, "Couldn't load image \"%s\"\n", bg);
		exit(0);
	}
	background = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	bak = SDL_CreateRGBSurface(SDL_SWSURFACE, 225, 32, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

	assert(bak != NULL);

	src.x = 50;
	src.y = 400;
	src.w = 225;
	src.h = 32;
	dest.x = 0;
	dest.y = 0;
	dest.w = src.w;
	dest.h = src.h;

	SDL_BlitSurface(background, &src, bak, &dest);

	bak_text = SDL_CreateRGBSurface(SDL_SWSURFACE, 242, 230, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

	assert(bak_text != NULL);

	src.x = 500;
	src.y = 125;
	src.w = 242;
	src.h = 300;
	dest.x = 0;
	dest.y = 0;
	dest.w = src.w;
	dest.h = src.h;

	SDL_BlitSurface(background, &src, bak_text, &dest);
}

void load_menu_text(void) {
	SDL_Surface *temp;

	temp = eaf_load_png(epiar_eaf, "menu/new_game.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/new_game.png\"\n");
	new_game = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/load_game.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/load_game.png\"\n");
	load_game = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/tutorial.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/tutorial.png\"\n");
	tutorial = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/configure.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/configure.png\"\n");
	configure = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/leave_epiar.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/leave_epiar.png\"\n");
	leave_epiar = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/new_game_h.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/new_game_h.png\"\n");
	SDL_SetColorKey(temp,
	                SDL_SRCCOLORKEY | SDL_RLEACCEL,
	                (Uint32) SDL_MapRGB(temp->format, 0, 0, 255));
	new_game_h = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/load_game_h.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/load_game_h.png\"\n");
	SDL_SetColorKey(temp,
	                SDL_SRCCOLORKEY | SDL_RLEACCEL,
	                (Uint32) SDL_MapRGB(temp->format, 0, 0, 255));
	load_game_h = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/tutorial_h.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/tutorial_h.png\"\n");
	SDL_SetColorKey(temp,
	                SDL_SRCCOLORKEY | SDL_RLEACCEL,
	                (Uint32) SDL_MapRGB(temp->format, 0, 0, 255));
	tutorial_h = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/configure_h.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/configure_h.png\"\n");
	SDL_SetColorKey(temp,
	                SDL_SRCCOLORKEY | SDL_RLEACCEL,
	                (Uint32) SDL_MapRGB(temp->format, 0, 0, 255));
	configure_h = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = eaf_load_png(epiar_eaf, "menu/leave_epiar_h.png");
	if (temp == NULL)
		fprintf(stdout, "Could not load \"data/images/menu/leave_epiar_h.png\"\n");
	SDL_SetColorKey(temp,
	                SDL_SRCCOLORKEY | SDL_RLEACCEL,
	                (Uint32) SDL_MapRGB(temp->format, 0, 0, 255));
	leave_epiar_h = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
}

void draw_background(void) {
	SDL_Rect rect;

	rect.x = 0;
	rect.y = 0;
	rect.w = background->w;
	rect.h = background->h;

	blit_surface(background, &rect, &rect, 1);
}

void draw_menu_text(void) {
	SDL_Rect src, dest;

	if (menu_update) {
		src.x = 0;
		src.y = 0;
		src.w = new_game->w;
		src.h = new_game->h;
		dest.x = 500;
		dest.y = 125;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(new_game, &src, &dest, 1);

		src.x = 0;
		src.y = 0;
		src.w = load_game->w;
		src.h = load_game->h;
		dest.x = 500;
		dest.y = 173;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(load_game, &src, &dest, 1);

		src.x = 0;
		src.y = 0;
		src.w = tutorial->w;
		src.h = tutorial->h;
		dest.x = 500;
		dest.y = 221;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(tutorial, &src, &dest, 1);

		src.x = 0;
		src.y = 0;
		src.w = configure->w;
		src.h = configure->h;
		dest.x = 500;
		dest.y = 269;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(configure, &src, &dest, 1);

		src.x = 0;
		src.y = 0;
		src.w = leave_epiar->w;
		src.h = leave_epiar->h;
		dest.x = 500;
		dest.y = 317;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(leave_epiar, &src, &dest, 1);

		/* blit the highlights */
		src.x = 0;
		src.y = 0;
		src.w = new_game_h->w;
		src.h = new_game_h->h;
		dest.x = 500;
		dest.y = 125;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(new_game_h, &src, &dest, 1);

		src.x = 0;
		src.y = 0;
		src.w = load_game_h->w;
		src.h = load_game_h->h;
		dest.x = 500;
		dest.y = 173;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(load_game_h, &src, &dest, 1);

		src.x = 0;
		src.y = 0;
		src.w = tutorial_h->w;
		src.h = tutorial_h->h;
		dest.x = 500;
		dest.y = 221;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(tutorial_h, &src, &dest, 1);

		src.x = 0;
		src.y = 0;
		src.w = configure_h->w;
		src.h = configure_h->h;
		dest.x = 500;
		dest.y = 269;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(configure_h, &src, &dest, 1);

		src.x = 0;
		src.y = 0;
		src.w = leave_epiar_h->w;
		src.h = leave_epiar_h->h;
		dest.x = 500;
		dest.y = 317;
		dest.w = src.w;
		dest.h = src.h;

		blit_surface(leave_epiar_h, &src, &dest, 1);
	}
}

menu_selection menu_get_input(void) {
	SDL_Event event;
	Uint8 btn_state;

	SDL_PollEvent(&event);

	switch (event.type) {

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEMOTION:
		{
			btn_state = SDL_GetMouseState(&mouse_x, &mouse_y);
			if (btn_state == SDL_BUTTON(1)) {
				if (mouse_over(new_game, 500, 125))
					return (NEW_GAME);
				if (mouse_over(load_game, 500, 173))
					return (LOAD_GAME);
				if (mouse_over(tutorial, 500, 221))
					return (TUTORIAL);
				if (mouse_over(configure, 500, 269))
					return (CONFIGURE);
				if (mouse_over(leave_epiar, 500, 317))
					return (LEAVE);
			}
			break;
		}

			/*
		case SDL_KEYUP:
			switch( event.key.keysym.sym ){
				case SDLK_ESCAPE:
					return (LEAVE);
					break;
				default:
					break;
			}

		case SDL_QUIT:
			return (LEAVE);
			break;
			*/

		default:
			break;
	}

	return (NOTHING);
}

void menu_update_frame(void) {
	if (mouse_over(new_game, 500, 125))
		adjust_alpha(0);
	else if (mouse_over(load_game, 500, 173))
		adjust_alpha(1);
	else if (mouse_over(tutorial, 500, 221))
		adjust_alpha(2);
	else if (mouse_over(configure, 500, 269))
		adjust_alpha(3);
	else if (mouse_over(leave_epiar, 500, 317))
		adjust_alpha(4);
	else
		adjust_alpha(-1);
}

void menu_draw_frame(void) {
	SDL_Rect src, dest;

	src.x = 0;
	src.y = 0;
	src.w = bak->w;
	src.h = bak->h;
	dest.x = 50;
	dest.y = 400;
	dest.w = src.w;
	dest.h = src.h;

	blit_surface(bak, &src, &dest, 1);

	src.x = 0;
	src.y = 0;
	src.w = bak_text->w;
	src.h = bak_text->h;
	dest.x = 500;
	dest.y = 125;
	dest.w = src.w;
	dest.h = src.h;

	blit_surface(bak_text, &src, &dest, 1);

	draw_menu_text();
}

void unload_menu_text(void) {
	SDL_FreeSurface(new_game);
	SDL_FreeSurface(load_game);
	SDL_FreeSurface(tutorial);
	SDL_FreeSurface(configure);
	SDL_FreeSurface(leave_epiar);
	SDL_FreeSurface(new_game_h);
	SDL_FreeSurface(load_game_h);
	SDL_FreeSurface(tutorial_h);
	SDL_FreeSurface(configure_h);
	SDL_FreeSurface(leave_epiar_h);
}

unsigned char mouse_over(SDL_Surface *surface, int x, int y) {
	int w = surface->w, h = surface->h;

	if (mouse_x >= x)
		if (mouse_y >= y)
			if (mouse_x <= (x + w))
				if (mouse_y <= (y + h))
					return (1);

	return (0);
}

/* passing something invalid, like -1, simply makes all the alphas decrease */
void adjust_alpha(int which) {
	int i;

	for (i = 0; i < 5; i++) {
		if (which == i) {
			if ((alphas[i] + 45) > 165)
				alphas[i] = 155;
			else {
				alphas[i] += 45;
				menu_update = 1;
			}
		} else {
			if ((alphas[i] - 8) < 0)
				alphas[i] = 0;
			else {
				alphas[i] -= 8;
				menu_update = 1;
			}
		}
	}

	SDL_SetAlpha(new_game_h, SDL_SRCALPHA | SDL_RLEACCEL, alphas[0]);
	SDL_SetAlpha(load_game_h, SDL_SRCALPHA | SDL_RLEACCEL, alphas[1]);
	SDL_SetAlpha(tutorial_h, SDL_SRCALPHA | SDL_RLEACCEL, alphas[2]);
	SDL_SetAlpha(configure_h, SDL_SRCALPHA | SDL_RLEACCEL, alphas[3]);
	SDL_SetAlpha(leave_epiar_h, SDL_SRCALPHA | SDL_RLEACCEL, alphas[4]);
}

void set_initial_alpha(void) {
	int i;

	for (i = 0; i < 5; i++) {
		alphas[i] = 0;
	}

	SDL_SetAlpha(new_game_h, SDL_SRCALPHA, alphas[0]);
	SDL_SetAlpha(load_game_h, SDL_SRCALPHA, alphas[1]);
	SDL_SetAlpha(tutorial_h, SDL_SRCALPHA, alphas[2]);
	SDL_SetAlpha(configure_h, SDL_SRCALPHA, alphas[3]);
	SDL_SetAlpha(leave_epiar_h, SDL_SRCALPHA, alphas[4]);
}

void fade_in(SDL_Surface *surface, Uint32 length) {
	SDL_Rect src, dest;
	int alpha = 0;
	Uint32 current_time, old_time;

	assert(surface != NULL);

	src.x = 0;
	src.y = 0;
	src.w = surface->w;
	src.h = surface->h;
	dest.x = 400 - (src.w / 2);
	dest.y = 300 - (src.h / 2);
	dest.w = src.w;
	dest.h = src.h;

	current_time = SDL_GetTicks();
	old_time = current_time;

	SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, 0);

	while(alpha < 255) {
		black_fill(0, 0, 800, 600, 1);
		blit_surface(surface, &src, &dest, 1);
		flip();
		alpha += 255 * ((float)(current_time - old_time) / length);
		SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
		old_time = current_time;
		current_time = SDL_GetTicks();
	}

	SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, 255);
}

/* where 'file' is a file inside an already opened eaf file */
static void display_logo_eaf(char *file) {
	SDL_Surface *temp, *surface;
	SDL_Rect src, dest;
	Uint32 old_time, current_time;
	Uint8 alpha = 0;
	Uint32 fading_time = 2350;

	black_fill(0, 0, 800, 600, 1);

	temp = eaf_load_png(epiar_eaf, file);
	if (temp == NULL) {
		fprintf(stdout, "Could not load PNG file in EAF, PNG file '%s'.\n", file);
		return;
	}
	surface = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	src.x = 0;
	src.y = 0;
	src.w = surface->w;
	src.h = surface->h;
	dest.x = 400 - (src.w / 2);
	dest.y = 300 - (src.h / 2);
	dest.w = src.w;
	dest.h = src.h;

	SDL_SetAlpha(surface, SDL_SRCALPHA, alpha);

	current_time = SDL_GetTicks();
	old_time = current_time;

	while(alpha < 255) {
		black_fill(dest.x, dest.y, dest.w, dest.h, 1);
		SDL_SetAlpha(surface, SDL_SRCALPHA, alpha);
		blit_surface(surface, &src, &dest, 1);
		flip();
		if (((float)alpha + (255.0f * ((float)(current_time - old_time) / (float)fading_time))) < 255.0f)
			alpha += (int)(255.0f * ((float)(current_time - old_time) / (float)fading_time));
		else
			alpha = 255;
		old_time = current_time;
		current_time = SDL_GetTicks();
		SDL_Delay(5);
	}

	SDL_Delay(850);

	current_time = SDL_GetTicks();
	old_time = current_time;
	SDL_SetAlpha(surface, SDL_SRCALPHA, 255);
	
	while(alpha > 0) {
		black_fill(0, 0, 800, 600, 1);
		SDL_SetAlpha(surface, SDL_SRCALPHA, alpha);
		blit_surface(surface, &src, &dest, 1);
		flip();
		if (((float)alpha - (255.0f * ((float)(current_time - old_time) / (float)fading_time))) > 0.0f)
			alpha -= (int)(255.0f * ((float)(current_time - old_time) / (float)fading_time));
		else
			alpha = 0;
		old_time = current_time;
		current_time = SDL_GetTicks();
		SDL_Delay(5);
	}

	SDL_FreeSurface(surface);
}
