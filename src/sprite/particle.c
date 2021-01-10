/* NOTE: contains particle _and_ chunk (ship pieces code) */

#include "includes.h"
#include "osdep/osdep.h"
#include "sprite/particle.h"
#include "system/rander.h"
#include "system/trig.h"
#include "system/video/backbuffer.h"
#include "system/video/video.h"

struct _particle particles[NUM_PARTICLES];

void init_particles(void) {
	int i;
  
	for (i = 0; i < NUM_PARTICLES; i++) {
		particles[i].world_x = particles[i].world_y = 0;
		particles[i].screen_x = particles[i].screen_y = 0;
		particles[i].time_left = 0;
		particles[i].length = 0;
		particles[i].momentum_x = 0;
		particles[i].momentum_y = 0;
	}
}

void uninit_particles(void) {
	int i;
  
	for (i = 0; i < NUM_PARTICLES; i++) {
		particles[i].world_x = particles[i].world_y = 0;
		particles[i].screen_x = particles[i].screen_y = 0;
		particles[i].time_left = 0;
		particles[i].length = 0;
		particles[i].momentum_x = 0;
		particles[i].momentum_y = 0;
	}
}

/* angle or momentums determine particle heading, angle takes presendence */
void new_particle(float world_x, float world_y, float momentum_x, float momentum_y, Uint32 length, int angle) {
	int i, slot = -1;

	for (i = 0; i < NUM_PARTICLES; i++) {
		if (particles[i].time_left == 0) {
			slot = i;
			break;
		}
	}

	/* have an open slot, so, create a new particle */
	if (slot != -1) {
		particles[slot].world_x = world_x;
		particles[slot].world_y = world_y;
		particles[slot].screen_x = 0;
		particles[slot].screen_y = 0;
		particles[slot].time_left = length;
		particles[slot].length = length;
		if (angle) {
			particles[slot].momentum_x = get_cos(angle) * (float)((float)(rand() % 10) / (float)10);
			particles[slot].momentum_y = get_neg_sin(angle) * (float)((float)(rand() % 10) / (float)10);
		} else {
			particles[slot].momentum_x = (float)((float)(rand() % 10) / (float)10) + momentum_x;
			particles[slot].momentum_y = (float)((float)(rand() % 10) / (float)10) + momentum_y;
		}
		particles[slot].angle = angle;
	}
}

/* blasts a few particles away from a certain point in a general angle */
/* note: the opposite of the angle passed is the direction the particles fly in */
void chunk_blast(int world_x, int world_y, int density, Uint32 length, int angle, int spread) {
	int i, fly_angle, real_spread;
	
	if (spread == -1)
		real_spread = 45;
	else
		real_spread = spread;
	
	/* fly_angle is opposite of angle */
	fly_angle = angle + 180;
	fly_angle %= 360;
	
	for (i = 0; i < density; i++) {
		new_particle(world_x, world_y, 0, 0, length, rander(fly_angle - real_spread, fly_angle + real_spread));
	}
}

void new_explosion(int world_x, int world_y, float momentum_x, float momentum_y, int density, Uint32 length) {
	int i;
	
	for (i = 0; i < (density / 2); i++) {
		new_particle(world_x, world_y, momentum_x, momentum_y, length, rand() % 180);
	}
	
	for (i = (density / 2); i < density; i++) {
		new_particle(world_x, world_y, momentum_x, momentum_y, length, (rand() % 180) + 180);
	}
}

void erase_particles(void) {
	int i;
	
	/* get a surface lock */
	if (SDL_MUSTLOCK(screen)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(screen);
	}
	
	switch(screen_bpp) {
	case 32:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint32);
			for (i = 0; i < NUM_PARTICLES; i++) {
				if (particles[i].time_left)
					if (particles[i].screen_x >= scr_left)
						if (particles[i].screen_y >= scr_top)
							if (particles[i].screen_x < scr_right)
								if (particles[i].screen_y < scr_bottom) {
									((Uint32 *) screen->pixels)[(int)particles[i].screen_x + ((int)particles[i].screen_y * pitch_adjust)] = black;
									ensure_blitted((int)particles[i].screen_x, (int)particles[i].screen_y, 1, 1);
								}
			}
			break;
		}
	case 16:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint16);
			for (i = 0; i < NUM_PARTICLES; i++) {
				if (particles[i].time_left)
					if (particles[i].screen_x >= scr_left)
						if (particles[i].screen_y >= scr_top)
							if (particles[i].screen_x < scr_right)
								if (particles[i].screen_y < scr_bottom) {
									((Uint16 *) screen->pixels)[(int)particles[i].screen_x + ((int)particles[i].screen_y * pitch_adjust)] = black;
									ensure_blitted((int)particles[i].screen_x, (int)particles[i].screen_y, 1, 1);
								}
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

void draw_particles(void) {
	int i;
	
	/* get a surface lock */
	if (SDL_MUSTLOCK(screen)) {
		unsigned char unlocked = 1;
		while (unlocked)
			unlocked = SDL_LockSurface(screen);
	}
	
	switch(screen_bpp) {
	case 32:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint32);
			for (i = 0; i < NUM_PARTICLES; i++) {
				if (particles[i].time_left)
					if (particles[i].screen_x >= scr_left)
						if (particles[i].screen_y >= scr_top)
							if (particles[i].screen_x < scr_right)
								if (particles[i].screen_y < scr_bottom) {
									((Uint32 *) screen->pixels)[(int)particles[i].screen_x + ((int)particles[i].screen_y * pitch_adjust)] = particles[i].color;
									ensure_blitted((int)particles[i].screen_x, (int)particles[i].screen_y, 1, 1);
								}
			}
			break;
		}
	case 16:
		{
			int pitch_adjust = screen->pitch / sizeof(Uint16);
			for (i = 0; i < NUM_PARTICLES; i++) {
				if (particles[i].time_left)
					if (particles[i].screen_x >= scr_left)
						if (particles[i].screen_y >= scr_top)
							if (particles[i].screen_x < scr_right)
								if (particles[i].screen_y < scr_bottom) {
									((Uint16 *) screen->pixels)[(int)particles[i].screen_x + ((int)particles[i].screen_y * pitch_adjust)] = particles[i].color;
									ensure_blitted((int)particles[i].screen_x, (int)particles[i].screen_y, 1, 1);
								}
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
