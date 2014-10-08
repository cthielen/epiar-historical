#include "asteroid/asteroid.h"
#include "com_defs.h"
#include "force/force.h"
#include "game/update.h"
#include "includes.h"
#include "sprite/chunk.h"
#include "sprite/particle.h"
#include "sprite/sprite.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/math.h"
#include "system/path.h"
#include "system/video/video.h"

#define MAX_ASTEROID_POINTS      15
#define MAX_ASTEROIDS_PER_POINT  15
#define MS_PER_FRAME             95
#define ASTEROID_WIDTH           65
#define ASTEROID_HEIGHT          65

struct _asteroids asteroids[MAX_ASTEROIDS];

struct _asteroid_point {
	int x, y, density;
	short int num_asteroids_spawned;
} *asteroid_points[MAX_ASTEROID_POINTS];

short int num_asteroid_points;

SDL_Surface *asteroid_surface[1];

static int create_asteroid(int x, int y, int which_point);
static void free_asteroid(int which);

void init_asteroids(void) {
	int i;
	
	/* set all asteroids cleared */
	for (i = 0; i < MAX_ASTEROIDS; i++) {
		asteroids[i].x = 0;
		asteroids[i].y = 0;
		asteroids[i].momentum_x = 0.0f;
		asteroids[i].momentum_y = 0.0f;
		asteroids[i].which_point = -1;
		asteroids[i].on = 0;
	}
	
	/* set all asteroid points null */
	for (i = 0; i < MAX_ASTEROID_POINTS; i++) {
		asteroid_points[i] = NULL;
	}
	
	asteroid_surface[0] = load_image_eaf(epiar_eaf, "asteroids/1.png", BLUE_COLORKEY);
	
	num_asteroid_points = 0;
}

void uninit_asteroids(void) {
	int i;
	
	/* turn off all asteroids */
	for (i = 0; i < MAX_ASTEROIDS; i++)
		asteroids[i].on = 0;
	
	/* free any existing asteroid points */
	for (i = 0; i < MAX_ASTEROID_POINTS; i++) {
		if (asteroid_points[i]) {
			free(asteroid_points[i]);
			asteroid_points[i] = NULL;
		}
	}
	
	assert(asteroid_surface[0]);
	SDL_FreeSurface(asteroid_surface[0]);
	
	num_asteroid_points = 0;
}

void update_asteroids(void) {
	int i, j;
	static Uint32 last_time = 0;
	
	/* check to see if we're near an asteroid point and need to spawn asteroids */
	for (i = 0; i < num_asteroid_points; i++) {
		assert(asteroid_points[i]);
		
		if (get_distance_from_player_sqrd(asteroid_points[i]->x, asteroid_points[i]->y) < 1000000) {
			if (asteroid_points[i]->num_asteroids_spawned < asteroid_points[i]->density) {
				/* possibly create an asteroid */
				if ((rand() % 300) > 250) {
					if (create_asteroid(asteroid_points[i]->x, asteroid_points[i]->y, i) == 0)
						asteroid_points[i]->num_asteroids_spawned++;
				}
			}
		}
	}
	
	/* update various asteroid information */
	for (i = 0; i < MAX_ASTEROIDS; i++) {
		if (asteroids[i].on) {
			int dist = get_distance_from_player_sqrd(asteroids[i].x, asteroids[i].y);
			/* update the positions of all valid asteroids */
			asteroids[i].x += (int)asteroids[i].momentum_x;
			asteroids[i].y += (int)asteroids[i].momentum_y;
			
			if (current_time > (last_time + MS_PER_FRAME)) {
				asteroids[i].frame_num++;
				asteroids[i].frame_num %= 45;
			}
			
			/* check for collisions with the player */
			if (dist < (33 + (player.ship->model->radius * player.ship->model->radius))) {
				int hit_angle;
				/* player has collided with asteroid */
				
				hit_angle = atan((asteroids[i].y - player.ship->world_y) / (asteroids[i].x - player.ship->world_x)); /* calculate hit angle based on coordinates */
				hit_angle = (hit_angle * 180.0) / 3.14159265; /* convert radians to degrees */
				
				damage_ship_non_fire(player.ship, 15, hit_angle);
				damage_asteroid(i, 15);
			}
			
			/* check for collisions with the sprites */
			for (j = 0; j < MAX_SHIPS; j++) {
				if (ships[j]) {
					if (get_distance_sqrd(asteroids[i].x, asteroids[i].y, ships[j]->world_x, ships[j]->world_y) < (33 + (ships[j]->model->radius * ships[j]->model->radius))) {
						float hit_angle;
						hit_angle = atan((asteroids[i].y - ships[j]->world_y) / (asteroids[i].x - ships[j]->world_x));
						hit_angle = (hit_angle * 180.0) / 3.14159265;
						
						damage_ship_non_fire(ships[j], 15, hit_angle);
						damage_asteroid(i, 15);
					}
				}
			}
			
			/* if it is too far away, delete it */
			if (dist > 2000000) {
				if ((rand() % 300) > 285)
					free_asteroid(i);
			}
		}
	}
	
	if (current_time > (last_time + MS_PER_FRAME))
		last_time = current_time;
}

void draw_asteroids(void) {
	int i;
	
	for (i = 0; i < MAX_ASTEROIDS; i++) {
		if (asteroids[i].on) {
			short int screen_x, screen_y;
			
			screen_x = asteroids[i].x - camera_x;
			screen_y = asteroids[i].y - camera_y;
			screen_x -= 33; /* center image on coords for blitting */
			screen_y -= 33;
			
			if ((screen_x >= -ASTEROID_WIDTH) && (screen_y >= -ASTEROID_HEIGHT) && (screen_x < (800 + ASTEROID_WIDTH)) && (screen_y < (600 + ASTEROID_HEIGHT))) {
				SDL_Rect src, dest;
				
				src.w = ASTEROID_WIDTH;
				src.h = ASTEROID_HEIGHT;
				src.y = 0;
				src.x = (asteroids[i].frame_num * ASTEROID_WIDTH);
				dest.x = screen_x;
				dest.y = screen_y;
				dest.w = ASTEROID_WIDTH;
				dest.h = ASTEROID_HEIGHT;
				
				blit_surface(asteroid_surface[0], &src, &dest, 0);
			}
		}
	}
}

void erase_asteroids(void) {
	int i;
	
	for (i = 0; i < MAX_ASTEROIDS; i++) {
		if (asteroids[i].on) {
			short int screen_x, screen_y;
			
			screen_x = asteroids[i].x - camera_x;
			screen_y = asteroids[i].y - camera_y;
			screen_x -= 33; /* center image on coords for blitting */
			screen_y -= 33;
			
			if ((screen_x >= -ASTEROID_WIDTH) && (screen_y >= -ASTEROID_HEIGHT) && (screen_x < (800 + ASTEROID_WIDTH)) && (screen_y < (600 + ASTEROID_HEIGHT)))
				black_fill(screen_x, screen_y, ASTEROID_WIDTH, ASTEROID_HEIGHT, 0);
		}
	}
}

/* creates an asteroid near given point (but garunteed nearby and offscreen) */
static int create_asteroid(int x, int y, int which_point) {
	int i, slot = -1;
	int screen_x, screen_y;
	
	/* find a free slot */
	for (i = 0; i < MAX_ASTEROIDS; i++) {
		if (!asteroids[i].on) {
			slot = i;
			break;
		}
	}
	
	if (slot == -1)
		return (-1); /* couldn't find a free slot in which to create the asteroid */
	
	/* create the asteroid in the slot */
	/* give it semi-random coordinates */
	x += (rand() % 2000) - 1000;
	y += (rand() % 2000) - 1000;
	
	screen_x = x - camera_x;
	screen_y = y - camera_y;
	
	/* x is onscreen, so adjust it to be somewhere offscreen */
	if ((screen_x > -25) && (screen_x < 825)) {
		if (rand() % 2) {
			x -= 900;
		} else {
			x += 900;
		}
	}
	/* y is onscreen, so adjust it to be somewhere offscreen */
	if ((screen_y > -25) && (screen_y < 625)) {
		if (rand() % 2) {
			y -= 700;
		} else {
			y += 700;
		}
	}
	asteroids[slot].x = x;
	asteroids[slot].y = y;
	asteroids[slot].frame_num = 0;
	asteroids[slot].momentum_x = 0;
	while(asteroids[slot].momentum_x == 0)
		asteroids[slot].momentum_x = (int)((float)(rand() % 5) - 2.5f);
	asteroids[slot].momentum_y = 0;
	while(asteroids[slot].momentum_y == 0)
		asteroids[slot].momentum_y = (int)((float)(rand() % 5) - 2.5f);
	asteroids[slot].which_point = which_point;
	asteroids[slot].life = (rand() % 6) + 4;
	asteroids[slot].on = 1;
	
	return (0);
}

/* frees an asteroid and notifies the cooresponding asteroid point that it has one less asteroid */
static void free_asteroid(int which) {
	
	asteroids[which].on = 0;
	if (asteroid_points[asteroids[which].which_point])
		asteroid_points[asteroids[which].which_point]->num_asteroids_spawned--;
}

void damage_asteroid(int which, int amount) {
	if (!asteroids[which].on)
		return;
	
	asteroids[which].life -= amount;

	if (asteroids[which].life <= 0) {
		SDL_Surface *temp;
		SDL_Rect src, dest;

		new_explosion(asteroids[which].x, asteroids[which].y, 0, 0, 25, 4500);
		new_force(RADIATING, asteroids[which].x, asteroids[which].y, 5, 60, 500);

		/* create a temp surface of the asteroid cell */
		src.w = ASTEROID_WIDTH;
		src.h = ASTEROID_HEIGHT;
		src.y = 0;
		src.x = (asteroids[which].frame_num * ASTEROID_WIDTH);
		dest.x = 0;
		dest.y = 0;
		dest.w = ASTEROID_WIDTH;
		dest.h = ASTEROID_HEIGHT;
		
		temp = SDL_CreateRGBSurface(SDL_SWSURFACE, dest.w, dest.h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
		assert(temp);
		SDL_BlitSurface(asteroid_surface[0], &src, temp, &dest);
		
		create_blast(temp, asteroids[which].x, asteroids[which].y, asteroids[which].momentum_x, asteroids[which].momentum_y);
		SDL_FreeSurface(temp);
		free_asteroid(which);
	}
}

void add_asteroid_field(int x, int y) {
	if (num_asteroid_points < MAX_ASTEROID_POINTS) {
		int i = num_asteroid_points;
		
		asteroid_points[i] = malloc(sizeof(struct _asteroid_point));
		assert(asteroid_points[i]);
		asteroid_points[i]->x = x;
		asteroid_points[i]->y = y;
		asteroid_points[i]->num_asteroids_spawned = 0;
		num_asteroid_points++;
	}
}

int load_asteroids_eaf(FILE *eaf, char *filename) {
	parsed_file *asteroids_esf = NULL;
	
	if (!eaf)
		return (-1);
	if (!filename)
		return (-1);
	
	asteroids_esf = esf_new_handle();
	if (!asteroids_esf) {
		printf("Error while loading asteroids\n");
		return (-1);
	}
	
	esf_set_filter(asteroids_esf, "asteroids");
	
	if (esf_parse_file_eaf(eaf, asteroids_esf, filename) != 0) {
		printf("Error while parsing asteroids\n");
		esf_close_handle(asteroids_esf);
		return (-1);
	} else {
		int i, j;
		
		for (i = 0; i < asteroids_esf->num_items; i++) {
			asteroid_points[num_asteroid_points] = (struct _asteroid_point *)malloc(sizeof(asteroid_points));
			asteroid_points[num_asteroid_points]->num_asteroids_spawned = 0;
			for (j = 0; j < asteroids_esf->items[i].num_keys; j++) {
				char *name = asteroids_esf->items[i].keys[j].name;
				union _esf_value value = asteroids_esf->items[i].keys[j].value;
				
				if (!strcmp(name, "X"))
					asteroid_points[num_asteroid_points]->x = value.i;
				else if (!strcmp(name, "Y"))
					asteroid_points[num_asteroid_points]->y = value.i;
				else if (!strcmp(name, "Density"))
					asteroid_points[num_asteroid_points]->density = value.i;
			}
			num_asteroid_points++;
		}
	}
	
	if (esf_close_handle(asteroids_esf) != 0)
		assert(0);
	
	return (0);
}

int unload_asteroids(void) {
	int i;
	
	for (i = 0; i < num_asteroid_points; i++) {
		free(asteroid_points[i]);
		asteroid_points[i] = NULL;
	}
	
	num_asteroid_points = 0;
	
	return (0);
}
