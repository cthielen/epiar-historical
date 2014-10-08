#include "com_defs.h"
#include "comm/comm.h"
#include "game/game.h"
#include "game/update.h"
#include "gui/gui.h"
#include "hud/hud.h"
#include "includes.h"
#include "sprite/gate.h"
#include "sprite/player.h"
#include "sprite/sprite.h"
#include "system/font.h"
#include "system/math.h"
#include "system/path.h"
#include "system/video/video.h"
#include "system/video/zoom.h"

#define MAX_HUD_MSG 5

#define RADAR_DIST    COMM_DIST
#define RADAR_WIDTH         100
#define RADAR_HEIGHT        100
#define RADAR_X             675
#define RADAR_Y             475

static SDL_Surface *weapon_bar, *main_bar, *radar_bg, *weapon_tab, *wbar_pri, *wbar_sec;
static SDL_Color shield_bar_color;
static SDL_Surface *target_surf = NULL, *gate_view = NULL;

static void draw_progress_bar(SDL_Surface *surface, int x, int y, int w, int h, float progress, SDL_Color color);
static void draw_credits(void);
static void draw_hud_msg(void);
static void draw_target(void);
static void erase_target(void);
static void erase_radar(void);
static void draw_radar(void);

/* hud message variables */
struct _hud_msg {
	char *msg;
	Uint32 length;
} hud_msg[MAX_HUD_MSG];

static Uint32 last_msg = 0;
static afont *hud_font = NULL;

unsigned char show_fps = 0;

int init_hud(void) {
	int i;
	SDL_Rect src;
	
	/* set shield bar color */
	shield_bar_color.r = 100;
	shield_bar_color.g = 100;
	shield_bar_color.b = 205;
	
	/* load hud images */
	weapon_bar = load_image_eaf(epiar_eaf, "hud/weapon_bar.png", 0);
	main_bar = load_image_eaf(epiar_eaf, "hud/main_bar.png", 0);
	radar_bg = load_image_eaf(epiar_eaf, "hud/radar_bg.png", BLUE_COLORKEY);
	weapon_tab = load_image_eaf(epiar_eaf, "hud/weapon_tab.png", BLUE_COLORKEY);
	wbar_pri = load_image_eaf(epiar_eaf, "hud/weapon_bar_pri.png", 0);
	wbar_sec = load_image_eaf(epiar_eaf, "hud/weapon_bar_sec.png", 0);
	gate_view = load_image_eaf(epiar_eaf, "hud/gate_target.png", ALPHA);
  
	/* initalize the hud message buffer */
	for (i = 0; i < MAX_HUD_MSG; i++) {
		hud_msg[i].msg = NULL;
		hud_msg[i].length = 0;
	}
	
	hud_font = epiar_load_font_eaf("fonts/Vera-8.af", epiar_eaf);
	assert(hud_font);
	
	assert(target_surf == NULL);
	target_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, 100, 100, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	assert(target_surf);
	
	/* dra	w default target_surf */
	src.x = 0;
	src.y = 0;
	src.w = target_surf->w;
	src.h = target_surf->h;
	SDL_FillRect(target_surf, &src, black);
	
	SDL_FillRect(target_surf, &src, SDL_MapRGB(target_surf->format, 0, 0, 115));
	src.x++;
	src.y++;
	src.w -= 2;
	src.h -= 2;
	SDL_FillRect(target_surf, &src, SDL_MapRGB(target_surf->format, 0, 0, 65));
	
	SDL_SetAlpha(target_surf, SDL_SRCALPHA, 100);
	
	return (0);
}

void uninit_hud(void) {
	int i;
	
	SDL_FreeSurface(weapon_bar);
	SDL_FreeSurface(main_bar);
	SDL_FreeSurface(radar_bg);
	SDL_FreeSurface(weapon_tab);
	SDL_FreeSurface(wbar_pri);
	SDL_FreeSurface(wbar_sec);
	SDL_FreeSurface(gate_view);
	
	/* ens	ure all hud messages are freed */
	for (i = 0; i < MAX_HUD_MSG; i++) {
		free(hud_msg[i].msg);
		hud_msg[i].msg = NULL;
	}
	
	epiar_free(hud_font);
	hud_font = NULL;
	
	assert(target_surf);
	SDL_FreeSurface(target_surf);
	target_surf = NULL;
}

void draw_hud(unsigned char ignore_redraw) {
	int i, j;
	float progress;
	
	j = 0; /* counter of number of weapons drawn so far */
	
	/* dra	w main bar & the two weapon tabs */
	blit_image(main_bar, 0, 586);
	blit_image(weapon_tab, 2, 577);
	/* draw the weapon name an	d recharge */
	if (player.ship->w_slot[0] != -1) {
		if (player.ship->weapon_mount[player.ship->w_slot[0]]) {
			if (player.ship->weapon_mount[player.ship->w_slot[0]]->weapon) {
				char temp[30] = {0};
				/* draw name */
				if (player.ship->weapon_mount[player.ship->w_slot[0]]->weapon->uses_ammo)
					sprintf(temp, "%s (%d)", player.ship->weapon_mount[player.ship->w_slot[0]]->weapon->name, player.ship->weapon_mount[player.ship->w_slot[0]]->ammo);
				else
					sprintf(temp, "%s", player.ship->weapon_mount[player.ship->w_slot[0]]->weapon->name);
				epiar_render_text(hud_font, white, black, AFONT_RENDER_BLENDED, screen, 12, 585, temp);
				/* draw recharge */
				if ((current_time - player.ship->weapon_mount[player.ship->w_slot[0]]->time) < (unsigned)player.ship->weapon_mount[player.ship->w_slot[0]]->weapon->recharge) {
					/* weapon is recharging, calculate the progress */
					progress = (float)(current_time - player.ship->weapon_mount[player.ship->w_slot[0]]->time) / (float)player.ship->weapon_mount[player.ship->w_slot[0]]->weapon->recharge;
				} else {
					progress = 1.0f;
				}
				
				draw_progress_bar(screen, 51, 590, 66, 7, progress, shield_bar_color);
			}
		}
	}
	blit_image(weapon_tab, 130, 577);
	if (player.ship->w_slot[1] != -1) {
		if (player.ship->weapon_mount[player.ship->w_slot[1]]) {
			if (player.ship->weapon_mount[player.ship->w_slot[1]]->weapon) {
				char temp[30] = {0};
				/* draw weapon name */
				if (player.ship->weapon_mount[player.ship->w_slot[1]]->weapon->uses_ammo)
					sprintf(temp, "%s (%d)", player.ship->weapon_mount[player.ship->w_slot[1]]->weapon->name, player.ship->weapon_mount[player.ship->w_slot[1]]->ammo);
				else
					sprintf(temp, "%s", player.ship->weapon_mount[player.ship->w_slot[1]]->weapon->name);
				epiar_render_text(hud_font, white, black, AFONT_RENDER_BLENDED, screen, 142, 585, temp);
				/* draw recharge */
				if ((current_time - player.ship->weapon_mount[player.ship->w_slot[1]]->time) < (unsigned)player.ship->weapon_mount[player.ship->w_slot[1]]->weapon->recharge) {
					/* weapon is recharging, calculate the progress */
					progress = (float)(current_time - player.ship->weapon_mount[player.ship->w_slot[1]]->time) / (float)player.ship->weapon_mount[player.ship->w_slot[1]]->weapon->recharge;
				} else {
					progress = 1.0f;
				}
				
				draw_progress_bar(screen, 188, 590, 66, 7, progress, shield_bar_color);
			}
		}
	}
	
	/* draw current shield status */
	draw_progress_bar(screen, 456, 590, 82, 7, (float)player.ship->shield_life / (float)player.ship->shield->strength, shield_bar_color);
	
	/* draw current hull status */
	draw_progress_bar(screen, 314, 590, 82, 7, (float)player.ship->hull_strength / (float)player.ship->model->hull_life, shield_bar_color);
	
	/* draw current fuel status */
	draw_progress_bar(screen, 582, 590, 82, 7, (float)player.ship->fuel / (float)player.ship->model->total_fuel, shield_bar_color);
  
	assert(player.ship);
  
	/* draw weapon slots and selections in upper left */
	for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
		if (player.ship->weapon_mount[i]) {
			int w, h, base;
      
			/* blit the weapon bar (background) */
			blit_image(weapon_bar, 0, (j * 20) + 5);
			assert(player.ship->weapon_mount[i]);
			assert(player.ship->weapon_mount[i]->weapon);
			assert(player.ship->weapon_mount[i]->weapon->name);
			/* draw the name of the weapon */
			epiar_size_text(gui_font_normal, player.ship->weapon_mount[i]->weapon->name, &w, &h, &base);
			epiar_render_text(gui_font_normal, white, 0, AFONT_RENDER_BLENDED, screen, 3, (j * 20) + 6 + base, player.ship->weapon_mount[i]->weapon->name);
			/* if selected, draw the color filled in */
			if ((player.ship->w_slot[0] == i) || (player.ship->w_slot[1] == i)) {
				/* this weapon is currently the primary weapon */
				if (player.ship->w_slot[0] == i)
					blit_image(wbar_pri, weapon_bar->w - wbar_pri->w, (j * 20) + 5);
				else
					blit_image(wbar_sec, weapon_bar->w - wbar_pri->w, (j * 20) + 5);
			}
			j++;
		}
	}
  
	/* draw radar bg */
	blit_image(radar_bg, 671, 470);
  
	/* draw amount of credits */
	draw_credits();
  
	/* draw any hud messages */
	draw_hud_msg();

	/* draw player's target */
	draw_target();
  
	/* draw fps if selected */
	if (show_fps) {
		char msg[10] = {0};
		sprintf(msg, "FPS: %f", 1.0f / ((float)loop_length / 1000.0f));
		epiar_render_text(hud_font, white, 0, AFONT_RENDER_BLENDED, screen, 4, 540, msg);
		memset(msg, 0, sizeof(char) * 10);
		sprintf(msg, "Av. FPS: %f", current_fps);
		epiar_render_text(hud_font, white, 0, AFONT_RENDER_BLENDED, screen, 4, 550, msg);
	}

	draw_radar();
}

void erase_hud(unsigned char ignore_redraw) {
	if (show_fps) {
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 530;
		rect.w = 85;
		rect.h = 24;
		fill_rect(&rect, black);
	}

	/* erase player's target */
	erase_target();

	erase_radar();
}

void draw_progress_bar(SDL_Surface *surface, int x, int y, int w, int h, float progress, SDL_Color color) {
	SDL_Rect rect;
	int r, g, b, i;

	if (progress <= 0)
		return;

	if (progress > 1)
		progress = 1;

	r = color.r * (4.0/3.0);
	g = color.g * (4.0/3.0);
	b = color.b * (4.0/3.0);
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = (int)((float)h * (2.0/7.0));
	rect.w *= progress;
	SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, r, g, b));
	rect.y += rect.h;
	SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, color.r, color.g, color.b));
	rect.y += rect.h;
	rect.h = (int)((float)h * (1.0/7.0));
	r = color.r * (2.0/3.0);
	g = color.g * (2.0/3.0);
	b = color.b * (2.0/3.0);
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, r, g, b));
	rect.y += rect.h;
	rect.h = (int)((float)h * (2.0/7.0));
	r = color.r * (1.0/3.0);
	g = color.g * (1.0/3.0);
	b = color.b * (1.0/3.0);
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, r, g, b));

	w *= progress;

	/* draw the curvy sections */
	r = color.r;
	g = color.g;
	b = color.b;
	rect.y = y + h - 5;
	for (i = 0; i < 4; i++) {
		rect.x = x + i;
		rect.w = 1;
		rect.h = 4 - i;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, r, g, b));
	}
	r = color.r * (1.0/3.0);
	g = color.g * (1.0/3.0);
	b = color.b * (1.0/3.0);
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	for (i = w - 4; i < w; i++) {
		rect.x = x + (w - 4) + (i - (w-4));
		rect.w = 1;
		rect.h = (i - (w-4));
		rect.y = y + (int)((float)h / 2.0) - rect.h + 1;
		SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, r, g, b));
	}
}

/* function to call when you want to give the player a msg on the hud */
void hud_message(char *message, Uint32 length) {
	int i;
	int slot = -1;

	if (message == NULL)
		return;
	if (strlen(message) <= 0)
		return;

	/* find a free slot and (if one found) copy this message's data into it */
	for (i = 0; i < MAX_HUD_MSG; i++) {
		if (hud_msg[i].msg == NULL) {
			slot = i;
			break;
		}
	}

	if (slot == -1) {
		printf("cannot accept hud message, no free slots\n");
		return;
	}

	hud_msg[slot].msg = (char *)malloc(sizeof(char) * (strlen(message) + 1));
	memset(hud_msg[slot].msg, 0, sizeof(char) * (strlen(message) + 1));
	strcpy(hud_msg[slot].msg, message);

	hud_msg[slot].length = length;

	if (slot == 0)
		last_msg = current_time; /* without this, most first slot messages would expire as soon as they began */
}

void update_hud(void) {

}

static void draw_credits(void) {
	static SDL_Surface *cached = NULL;
	static int old_credits = -1, base = -1;
	SDL_Rect rect;
	
	if (old_credits != player.credits) {
		/* credits value has changed, render a new surface */
		char money[20] = {0};
		old_credits = player.credits;
		
		if (cached != NULL) {
			/* before changing the look, draw a black rect. over the old credits display */
			black_fill(screen->w - cached->w - 5, 6, cached->w, cached->h, 1);
			SDL_FreeSurface(cached);
		}
    
		sprintf(money, "%d credits", player.credits);
		
		cached = epiar_render_text_surf(gui_font_bold, white, 0, AFONT_RENDER_BLENDED, screen->format, money);
		if (base == -1) {
			int w, h;
			
			epiar_size_text(gui_font_bold, money, &w, &h, &base);
		}
	}
	
	rect.x = screen->w - cached->w - 5;
	rect.y = base - 2;
	rect.w = cached->w;
	rect.h = cached->h;
	
	blit_surface(cached, NULL, &rect, 1);
}

/* handles the drawing of any hud messages */
static void draw_hud_msg(void) {
	static SDL_Surface *msg = NULL;
	SDL_Rect rect;
	int w, h, base;
	
	if (hud_msg[0].msg == NULL)
		return;
	
	if (last_msg == 0)
		last_msg = current_time;
	
	if (msg == NULL)
		msg = epiar_render_text_surf(gui_font_bold, white, 0, AFONT_RENDER_BLENDED, screen->format, hud_msg[0].msg);
	
	epiar_size_text(gui_font_bold, hud_msg[0].msg, &w, &h, &base);
	
	rect.x = 5;
	rect.y = screen->h - 50 + base;
	rect.w = msg->w;
	rect.h = msg->h;
	
	blit_surface(msg, NULL, &rect, 1);
	
	/* check to see if the message's time is up, if so, replace it with the new message */
	if (current_time > (hud_msg[0].length + last_msg)) {
		int i;
		
		last_msg = current_time;
		
		for (i = 0; i < (MAX_HUD_MSG - 1); i++) {
			free(hud_msg[i].msg);
			hud_msg[i].msg = NULL;
			
			if (hud_msg[i+1].msg == NULL)
				break;
			
			hud_msg[i].msg = (char *)malloc(sizeof(char) * (strlen(hud_msg[i+1].msg) + 1));
			memset(hud_msg[i].msg, 0, sizeof(char) * (strlen(hud_msg[i+1].msg) + 1));
			strcpy(hud_msg[i].msg, hud_msg[i+1].msg);
		}
		if (hud_msg[MAX_HUD_MSG-1].msg) {
			/* and finally, free the last one */
			free(hud_msg[MAX_HUD_MSG-1].msg);
			hud_msg[MAX_HUD_MSG-1].msg = NULL;
		}
		
		black_fill(rect.x, rect.y, rect.w, rect.h, 1);
		free(msg);
		msg = NULL;
	}
}

static void draw_target(void) {
	SDL_Rect src, dest;
	int i;
	/* colors for the shield/hull status bars */
	static Uint32 target_shield_dark_blue = 0, target_shield_blue = 0, target_hull_dark_yellow = 0, target_hull_yellow = 0;

	if (target_shield_dark_blue == 0)
		target_shield_dark_blue = SDL_MapRGB(screen->format, 0, 0, 100);
	if (target_shield_blue == 0)
		target_shield_blue = SDL_MapRGB(screen->format, 0, 0, 200);
	if (target_hull_dark_yellow == 0)
		target_hull_dark_yellow = SDL_MapRGB(screen->format, 75, 75, 0);
	if (target_hull_yellow == 0)
		target_hull_yellow = SDL_MapRGB(screen->format, 215, 215, 0);
	
	assert(target_surf);

	if (player.target.type == TARGET_SHIP) {
		struct _ship *ship = NULL;
		SDL_Surface *surf = NULL;
		int w, h, base;
		char buf[55] = {0};
		unsigned char zoomed = 0;
		
		ship = (struct _ship *)player.target.target;
		assert(ship);
		
		if (ship->model->cached[ship->angle / 10])
			surf = ship->model->cached[ship->angle / 10];
		else
			surf = ship->model->image;
		
		if ((surf->w > target_surf->w - 5) || (surf->h > target_surf->h - 5)) {
			/* image is too big, we need to scale it down */
			float zoom_x, zoom_y;
			SDL_Surface *temp;

			zoomed = 1; /* let the code know we zoomed and allocated a new, temporary surface that needs to be freed (if we didn't zoom, we don't want to free the iamge since we borrowed it from other structures) */

			/* calculate by how much it needs to be zoomed */
			zoom_x = (float)target_surf->w / (float)surf->w;
			zoom_y = (float)target_surf->h / (float)surf->h;

			/* maintain 1:1 aspect ratio */
			if (zoom_x < zoom_y)
				zoom_y = zoom_x;
			else
				zoom_x = zoom_y;
			
			temp = zoomSurface(surf, zoom_x, zoom_y, 1);
			assert(temp);
			surf = SDL_DisplayFormat(temp);
			SDL_FreeSurface(temp);
			SDL_SetColorKey(surf, SDL_SRCCOLORKEY | SDL_RLEACCEL, (Uint32)SDL_MapRGB(surf->format, 0, 0, 0));
		}

		/* draw ship */
		src.x = 0;
		src.y = 0;
		src.w = surf->w;
		src.h = surf->h;
		if (src.w > (target_surf->w - 2))
			src.w = target_surf->w - 2;
		if (src.h > (target_surf->h - 2))
			src.h = target_surf->h - 2;
		dest.x = (target_surf->w / 2) - (surf->w / 2);
		dest.y = (target_surf->h / 2) - (surf->h / 2);
		dest.x += 15;
		dest.y += 425;
		dest.w = src.w;
		dest.h = src.h;
		if (dest.w > target_surf->w)
			dest.w = target_surf->w;
		if (dest.h > target_surf->h)
			dest.h = target_surf->h;
		
		if (dest.x < 15)
			dest.x = 15;
		if (dest.w > target_surf->w)
			dest.w = target_surf->w;
		
		blit_surface(surf, &src, &dest, 1);
		
		if (ship->name)
			strcpy(buf, ship->name);
		else
			strcpy(buf, ship->model->name);

		/* draw type label */
		epiar_size_text(hud_font, buf, &w, &h, &base);
		while(w > (target_surf->w - 18)) {
			int len = strlen(buf);
			buf[len - 1] = 0;
			buf[len - 2] = '.';
			buf[len - 3] = '.';
			buf[len - 4] = '.';
			epiar_size_text(hud_font, buf, &w, &h, &base);
		}
		epiar_render_text(hud_font, white, 0, AFONT_RENDER_BLENDED, screen, 63 - (w / 2), 510 - (h/2) + base, buf);

		/* ensure distance update */
		player.target.dist = get_distance(player.ship->world_x, player.ship->world_y, ship->world_x, ship->world_y);

		memset(buf, 0, sizeof(char) * 55);	
		sprintf(buf, "Dist: %d", (int)player.target.dist);
		epiar_size_text(hud_font, buf, &w, &h, &base);
		epiar_render_text(hud_font, white, 0, AFONT_RENDER_BLENDED, screen, 63 - (w / 2), 520 - (h/2) + base, buf);

		if (zoomed)
			SDL_FreeSurface(surf); /* if we did any zooming, we made a temp. surface of the zoomed image that needs to be freed */
    
	} else if (player.target.type == TARGET_GATE) {
		struct _gate *gate = NULL;
		int w, h, base;
		char buf[55] = {0};
		
		gate = (struct _gate *)player.target.target;
		assert(gate);
		
		/* draw gate */
		src.x = 0;
		src.y = 0;
		src.w = gate_view->w;
		src.h = gate_view->h;
		if (src.w > (target_surf->w - 2))
			src.w = target_surf->w - 2;
		if (src.h > (target_surf->h - 2))
			src.h = target_surf->h - 2;
		dest.x = (target_surf->w / 2) - (gate_view->w / 2);
		dest.y = (target_surf->h / 2) - (gate_view->h / 2);
		dest.x += 15;
		dest.y += 425;
		dest.w = src.w;
		dest.h = src.h;
		if (dest.w > target_surf->w)
			dest.w = target_surf->w;
		if (dest.h > target_surf->h)
			dest.h = target_surf->h;
		
		blit_surface(gate_view, &src, &dest, 1);
		
		strcpy(buf, gate->name);
		/* draw gate's name */
		epiar_size_text(hud_font, buf, &w, &h, &base);
		while(w > (target_surf->w - 4)) {
			int len = strlen(buf);
			buf[len - 1] = 0;
			buf[len - 2] = '.';
			buf[len - 3] = '.';
			buf[len - 4] = '.';
			epiar_size_text(hud_font, buf, &w, &h, &base);
		}
		epiar_render_text(hud_font, white, 0, AFONT_RENDER_BLENDED, screen, 63 - (w / 2), 510 - (h/2) + base, buf);

		/* ensure distance update */
		player.target.dist = get_distance(player.ship->world_x, player.ship->world_y, gate->world_x, gate->world_y);
		
		memset(buf, 0, sizeof(char) * 55);
		sprintf(buf, "Dist: %d", (int)player.target.dist);
		epiar_size_text(hud_font, buf, &w, &h, &base);
		epiar_render_text(hud_font, white, 0, AFONT_RENDER_BLENDED, screen, 63 - (w / 2), 520 - (h/2) + base, buf);
	}
	
	if (player.target.type != TARGET_NONE) {
		src.x = 0;
		src.y = 0;
		src.w = target_surf->w;
		src.h = target_surf->h;
		dest.x = 15;
		dest.y = 425;
		dest.w = src.w;
		dest.h = src.h;
		
		blit_surface(target_surf, &src, &dest, 1);
	}

	if (player.target.type == TARGET_SHIP) {
		struct _ship *ship = (struct _ship *)player.target.target;
		SDL_Rect rect;

		/* if we have any hud upgrades, this is where we draw them */
		for (i = 0; i < MAX_OUTFITS; i++) {
			if (player.upgrades[i] == NULL)
				break;
			else if (player.upgrades[i]->specific == HUD) {
				hud_item *hdata = (hud_item *)player.upgrades[i];
				
				if (hdata->show_target_shield) {
					rect.x = 19;
					rect.y = 435;
					rect.w = 1;
					rect.h = target_surf->h - 20;

					rect.h = (int)((float)rect.h * ((float)ship->shield_life / (float)ship->shield->strength));

					if (rect.h > 0) {
						fill_rect(&rect, target_shield_dark_blue);
						rect.x += 1;
						fill_rect(&rect, target_shield_blue);
						rect.x += 1;
						fill_rect(&rect, target_shield_dark_blue);
					}
				}
				if (hdata->show_target_hull) {
					rect.x = 15 + target_surf->w - 7;
					rect.y = 435;
					rect.w = 1;
					rect.h = target_surf->h - 20;
					
					rect.h = (int)((float)rect.h * ((float)	ship->hull_strength / (float)ship->model->hull_life));

					if (rect.h > 0) {
						fill_rect(&rect, target_hull_dark_yellow);
						rect.x += 1;
						fill_rect(&rect, target_hull_yellow);
						rect.x += 1;
						fill_rect(&rect, target_hull_dark_yellow);
					}
				}
			}
		}
	}
}

static void erase_target(void) {
	SDL_Rect rect;

	rect.x = 15;
	rect.y = 425;
	rect.w = target_surf->w;
	rect.h = target_surf->h;

	fill_rect(&rect, black);
}

/* imported from the old radar plugin */
static void erase_radar(void) {
	int i;
	int radar_middle_x, radar_middle_y;
	extern unsigned char view_mode;
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
	
	/* dont flip the erase (unneeded in radar's case since we flip all erased areas with the draw) */
	if (!view_mode) {
		radar_middle_x = RADAR_X + (RADAR_WIDTH / 2);
		radar_middle_y = RADAR_Y + (RADAR_HEIGHT / 2);
		
		for (i = 0; i < num_planets; i++) {
			int blip_x = radar_middle_x - (((player.ship->world_x - planets[i]->world_x) * RADAR_WIDTH) / RADAR_DIST);
			int blip_y = radar_middle_y - (((player.ship->world_y - planets[i]->world_y) * RADAR_HEIGHT) / RADAR_DIST);
			if ((blip_x > RADAR_X) && (blip_x < (RADAR_X + RADAR_WIDTH)) && (blip_y > RADAR_Y) && (blip_y < (RADAR_Y + RADAR_HEIGHT))) {
				if (screen_bpp == 32) {
					((Uint32 *) screen->pixels)[(int)blip_x + ((int)blip_y * pitch_adjust)] = black;
					((Uint32 *) screen->pixels)[(int)(blip_x + 1) + ((int)blip_y * pitch_adjust)] = black;
					((Uint32 *) screen->pixels)[(int)blip_x + ((int)(blip_y + 1) * pitch_adjust)] = black;
					((Uint32 *) screen->pixels)[(int)(blip_x + 1) + ((int)(blip_y + 1) * pitch_adjust)] = black;
					ensure_blitted(blip_x, blip_y, 2, 2);
				} else if (screen_bpp == 16) {
					((Uint16 *) screen->pixels)[(int)blip_x + ((int)blip_y * pitch_adjust)] = black;
					((Uint16 *) screen->pixels)[(int)(blip_x + 1) + ((int)blip_y * pitch_adjust)] = black;
					((Uint16 *) screen->pixels)[(int)blip_x + ((int)(blip_y + 1) * pitch_adjust)] = black;
					((Uint16 *) screen->pixels)[(int)(blip_x + 1) + ((int)(blip_y + 1) * pitch_adjust)] = black;
					ensure_blitted(blip_x, blip_y, 2, 2);
				}
			}
		}
		
		for (i = 0; i < num_gates; i++) {
			int blip_x = radar_middle_x - (((player.ship->world_x - gates[i]->world_x) * RADAR_WIDTH) / RADAR_DIST);
			int blip_y = radar_middle_y - (((player.ship->world_y - gates[i]->world_y) * RADAR_HEIGHT) / RADAR_DIST);
			if ((blip_x > RADAR_X) && (blip_x < (RADAR_X + RADAR_WIDTH)) && (blip_y > RADAR_Y) && (blip_y < (RADAR_Y + RADAR_HEIGHT))) {
				if (screen_bpp == 32) {
					((Uint32 *) screen->pixels)[(int)blip_x + ((int)blip_y * pitch_adjust)] = black;
					((Uint32 *) screen->pixels)[(int)(blip_x + 1) + ((int)blip_y * pitch_adjust)] = black;
					((Uint32 *) screen->pixels)[(int)blip_x + ((int)(blip_y + 1) * pitch_adjust)] = black;
					((Uint32 *) screen->pixels)[(int)(blip_x + 1) + ((int)(blip_y + 1) * pitch_adjust)] = black;
					ensure_blitted(blip_x, blip_y, 2, 2);
				} else if (screen_bpp == 16) {
					((Uint16 *) screen->pixels)[(int)blip_x + ((int)blip_y * pitch_adjust)] = black;
					((Uint16 *) screen->pixels)[(int)(blip_x + 1) + ((int)blip_y * pitch_adjust)] = black;
					((Uint16 *) screen->pixels)[(int)blip_x + ((int)(blip_y + 1) * pitch_adjust)] = black;
					((Uint16 *) screen->pixels)[(int)(blip_x + 1) + ((int)(blip_y + 1) * pitch_adjust)] = black;
					ensure_blitted(blip_x, blip_y, 2, 2);
				}
			}
		}
		
		for (i = 0; i < MAX_SHIPS; i++) {
			if (ships[i]) {
				if ((ships[i]->status != SHIP_CLOAKING) && (ships[i]->status != SHIP_DECLOAKING) && (ships[i]->landed == 0)) {
					int blip_x = radar_middle_x - (((player.ship->world_x - ships[i]->world_x) * RADAR_WIDTH) / RADAR_DIST);
					int blip_y = radar_middle_y - (((player.ship->world_y - ships[i]->world_y) * RADAR_HEIGHT) / RADAR_DIST);
					if ((blip_x > RADAR_X) && (blip_x < (RADAR_X + RADAR_WIDTH)) && (blip_y > RADAR_Y) && (blip_y < (RADAR_Y + RADAR_HEIGHT))) {
						if (screen_bpp == 32) {
							((Uint32 *) screen->pixels)[(int)blip_x + ((int)blip_y * pitch_adjust)] = black;
							ensure_blitted(blip_x, blip_y, 1, 1);
						} else if (screen_bpp == 16) {
							((Uint16 *) screen->pixels)[(int)blip_x + ((int)blip_y * pitch_adjust)] = black;
							ensure_blitted(blip_x, blip_y, 1, 1);
						}
					}
				}
			}
		}
	}
	
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
}

static void draw_radar(void) {
	int i;
	int radar_middle_x, radar_middle_y;
	int pitch_adjust;
	extern unsigned char view_mode;
	extern Uint32 current_time;
	
	if (screen_bpp == 32)
		pitch_adjust = screen->pitch / sizeof(Uint32);
	else if (screen_bpp == 16)
		pitch_adjust = screen->pitch / sizeof(Uint16);
	
	if (SDL_MUSTLOCK(screen)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(screen);
	}
	
	if (!view_mode) {
		
		radar_middle_x = RADAR_X + (RADAR_WIDTH / 2);
		radar_middle_y = RADAR_Y + (RADAR_HEIGHT / 2);
		
		/* draw planets */
		for (i = 0; i < num_planets; i++) {
			int blip_x = radar_middle_x - (((player.ship->world_x - planets[i]->world_x) * RADAR_WIDTH) / RADAR_DIST);
			int blip_y = radar_middle_y - (((player.ship->world_y - planets[i]->world_y) * RADAR_HEIGHT) / RADAR_DIST);
			if ((blip_x > RADAR_X) && (blip_x < (RADAR_X + RADAR_WIDTH)) && (blip_y > RADAR_Y) && (blip_y < (RADAR_Y + RADAR_HEIGHT))) {
				planets[i]->revealed = 1;
				if (screen_bpp == 32) {
					((Uint32 *) screen->pixels)[blip_x + (blip_y * pitch_adjust)] = light_blue;
					((Uint32 *) screen->pixels)[(blip_x + 1) + (blip_y * pitch_adjust)] = light_blue;
					((Uint32 *) screen->pixels)[blip_x + ((blip_y + 1) * pitch_adjust)] = light_blue;
					((Uint32 *) screen->pixels)[(blip_x + 1) + ((blip_y + 1) * pitch_adjust)] = light_blue;
					ensure_blitted(blip_x, blip_y, 2, 2);
				} else if (screen_bpp == 16) {
					((Uint16 *) screen->pixels)[blip_x + (blip_y * pitch_adjust)] = light_blue;
					((Uint16 *) screen->pixels)[(blip_x + 1) + (blip_y * pitch_adjust)] = light_blue;
					((Uint16 *) screen->pixels)[blip_x + ((blip_y + 1) * pitch_adjust)] = light_blue;
					((Uint16 *) screen->pixels)[(blip_x + 1) + ((blip_y + 1) * pitch_adjust)] = light_blue;
					ensure_blitted(blip_x, blip_y, 2, 2);
				}
			}
		}
		
		/* draw gates */
		for (i = 0; i < num_gates; i++) {
			int blip_x = radar_middle_x - (((player.ship->world_x - gates[i]->world_x) * RADAR_WIDTH) / RADAR_DIST);
			int blip_y = radar_middle_y - (((player.ship->world_y - gates[i]->world_y) * RADAR_HEIGHT) / RADAR_DIST);
			static Uint32 last_blink_change = 0;
			static unsigned char blink_on = 0;
			Uint32 blink_color, color;
			
			if ((last_blink_change + 250) < current_time) {
				if (blink_on) {
					blink_color = orange;
					blink_on = 0;
				} else {
					blink_color = white;
					blink_on = 1;
				}
				last_blink_change = current_time;
			} else {
				if (blink_on)
					blink_color = white;
				else
					blink_color = orange;
			}
			
			if ((blip_x > RADAR_X) && (blip_x < (RADAR_X + RADAR_WIDTH)) && (blip_y > RADAR_Y) && (blip_y < (RADAR_Y + RADAR_HEIGHT))) {
				struct _gate *gate = NULL;
				
				if (player.target.type == TARGET_GATE)
					gate = (struct _gate *)player.target.target;
				
				if (gate == gates[i])
					color = blink_color;
				else
					color = orange;
				
				if (screen_bpp == 32) {
					((Uint32 *) screen->pixels)[blip_x + (blip_y * pitch_adjust)] = color;
					((Uint32 *) screen->pixels)[(blip_x + 1) + (blip_y * pitch_adjust)] = color;
					((Uint32 *) screen->pixels)[blip_x + ((blip_y + 1) * pitch_adjust)] = color;
					((Uint32 *) screen->pixels)[(blip_x + 1) + ((blip_y + 1) * pitch_adjust)] = color;
					ensure_blitted(blip_x, blip_y, 2, 2);
				} else if (screen_bpp == 16) {
					((Uint16 *) screen->pixels)[blip_x + (blip_y * pitch_adjust)] = color;
					((Uint16 *) screen->pixels)[(blip_x + 1) + (blip_y * pitch_adjust)] = color;
					((Uint16 *) screen->pixels)[blip_x + ((blip_y + 1) * pitch_adjust)] = color;
					((Uint16 *) screen->pixels)[(blip_x + 1) + ((blip_y + 1) * pitch_adjust)] = color;
					ensure_blitted(blip_x, blip_y, 2, 2);
				}
			}
		}
	}
	
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			static Uint32 last_blink_change = 0;
			static unsigned char blink_on = 0;
			Uint32 blink_color;
			if ((ships[i]->status != SHIP_CLOAKING) && (ships[i]->landed == 0)) {
				int blip_x = radar_middle_x - (((player.ship->world_x - ships[i]->world_x) * RADAR_WIDTH) / RADAR_DIST);
				int blip_y = radar_middle_y - (((player.ship->world_y - ships[i]->world_y) * RADAR_HEIGHT) / RADAR_DIST);
				if ((last_blink_change + 250) < current_time) {
					if (blink_on) {
						blink_color = grey;
						blink_on = 0;
					} else {
						blink_color = white;
						blink_on = 1;
					}
					last_blink_change = current_time;
				} else {
					if (blink_on)
						blink_color = white;
					else
						blink_color = grey;
				}
				if ((blip_x > RADAR_X) && (blip_x < (RADAR_X + RADAR_WIDTH)) && (blip_y > RADAR_Y) && (blip_y < (RADAR_Y + RADAR_HEIGHT))) {
					struct _ship *ship = NULL;
					if (player.target.type == TARGET_SHIP)
						ship = (struct _ship *)player.target.target;
					if (ship == ships[i]) {
						if (screen_bpp == 32)
							((Uint32 *) screen->pixels)[blip_x + (blip_y * pitch_adjust)] = blink_color;
						else if (screen_bpp == 16)
							((Uint16 *) screen->pixels)[blip_x + (blip_y * pitch_adjust)] = blink_color;
					} else {
						if (screen_bpp == 32)
							((Uint32 *) screen->pixels)[blip_x + (blip_y * pitch_adjust)] = grey;
						else if (screen_bpp == 16)
							((Uint16 *) screen->pixels)[blip_x + (blip_y * pitch_adjust)] = grey;
						ensure_blitted(blip_x, blip_y, 1, 1);
					}
				}
			}
		}
	}
	
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
}
