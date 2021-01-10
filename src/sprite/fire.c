#include "ai/ai.h"
#include "asteroid/asteroid.h"
#include "audio/audio.h"
#include "game/game.h"
#include "game/update.h"
#include "includes.h"
#include "sprite/fire.h"
#include "sprite/particle.h"
#include "system/debug.h"
#include "system/math.h"
#include "system/rander.h"
#include "system/trig.h"
#include "system/video/video.h"

#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

struct _fire *ordered_fires[MAX_FIRES];
struct _fire fires[MAX_FIRES];

int num_fires = 0;
static int get_free_weapon_slot(void);
static void free_fire(short int which);

/******************************************************************************      
*
*   Name:
*      void fire(struct _ship *ship, unsigned char primary);
*
*   Abstract:
*      Creates new "fire" sprite in the game universe (fires a weapon). Second
*   parameter is true if the primary weapon is being fired, otherwise, the
*   secondary is fired.
*
*   Context/Scope:
*      Called from get_input().
*
*   Side Effects:
*      None.
*
*   Return Value:
*      None.
*
*   Assumptions:
*      struct _ship *ship is fully initialized (including all pointers within).
*
******************************************************************************/
/* pass player struct and set ship to null if the player is firing, otherwise, keep player at null and pass ship */
void fire(struct _ship *ship, short int which_mount) {
	int i;
	
	assert(ship != NULL);
	
	if (which_mount == -1)
		return;
	
	if (ship->weapon_mount[which_mount] == NULL) {
		fprintf(stdout, "WARNING: fire() which_mount = (%d) but that weapon mount is invalid\n", which_mount);
		fprintf(stdout, "WARNING: model is \"%s\"\n", ship->model->name);
		return;
	}
	
	if ((ship->weapon_mount[which_mount]->ammo <= 0) && (ship->weapon_mount[which_mount]->weapon->uses_ammo))
		return;
	
	if (ship->weapon_mount[which_mount]->time > current_time)
		return;

	if (ship->hull_strength <= 0)
		return; /* dead ships can't fire */
	
	i = get_free_weapon_slot();
	if (i == -1) {
		return; /* no free weapon slots */
	} else {
		/* fire the weapon */
		int angle, fire_x, fire_y;
		
		angle = (ship->angle + ship->weapon_mount[which_mount]->angle) % 360;
		
		assert(ship);
		assert(ship->weapon_mount[which_mount]);
		assert(ship->weapon_mount[which_mount]->weapon);
		fires[i].surface = rotate(ship->weapon_mount[which_mount]->weapon->ammo_image, angle); /* cannot be void if creaated, set this to an image */
		fires[i].weapon = ship->weapon_mount[which_mount]->weapon;
		fires[i].velocity = fires[i].weapon->velocity;
		fires[i].owner = ship;
		/* assign the world x & y values */
		fire_x = (int)fires[i].world_x;
		fire_y = (int)fires[i].world_y;
		rotate_point(ship->weapon_mount[which_mount]->x, ship->weapon_mount[which_mount]->y, angle, &fire_x, &fire_y);
		fires[i].world_x = (float)fire_x;
		fires[i].world_y = (float)fire_y;
		fires[i].world_x += ship->world_x;
		fires[i].world_y = ship->world_y - fires[i].world_y;
		fires[i].screen_x = -1500;
		fires[i].screen_y = -1500;
		fires[i].angle = ship->angle + ship->weapon_mount[which_mount]->angle;
		if (ship->weapon_mount[which_mount]->range != 0) {
			/* the weapon has a range, meaning it can turn, let's apply this by aiming toward the target if there is one */
			if (ship->target) {
				float angle_to = get_angle_to(ship->world_x, ship->world_y, ship->target->offender->world_x, ship->target->offender->world_y);
				int range = ship->weapon_mount[which_mount]->range / 2; /* divided by 2 because the range is counting both left and right */
				int angle = fires[i].angle;

				/* if the angle is within our range, aim straight at the target */
				if (((angle + range) > angle_to) && ((angle - range) < angle_to))
					fires[i].angle = angle_to;
				else {
					/* it's not within range, so aim closest to it */
					if (angle_to > (angle + range))
						fires[i].angle = angle + range;
					else
						fires[i].angle = angle - range;
				}
			}
		}
		fires[i].angle %= 360;
		
		/* if it has auto-aim, fire towards the target if possible, if not, it fires at the closest angle */
		if (ship->weapon_mount[which_mount]->range) {
			/* somewhat diff. variables for player */
			if (ship == player.ship) {
				if (player.target.type != TARGET_NONE) {
					float new_angle;
					float world_x, world_y;
					int aim_angle = (int)player.ship->weapon_mount[which_mount]->angle;
					int range = (int)player.ship->weapon_mount[which_mount]->range;
					
					if (player.target.type == TARGET_GATE) {
						struct _gate *gate = (struct _gate *)player.target.target;
						world_x = gate->world_x;
						world_y = gate->world_y;
					} else if (player.target.type == TARGET_SHIP) {
						struct _ship *ship = (struct _ship *)player.target.target;
						world_x = ship->world_x;
						world_y = ship->world_y;
					}
					
					new_angle = (float)atan2((float)((-1 * world_y) - (-1 * fires[i].world_y)), (float)(world_x - fires[i].world_x));
					
					/* convert aim_angle and range to radians :-) */
					aim_angle = (int)(((float)aim_angle * M_PI) / 180.0f);
					range = (int)(((float)range * M_PI) / 180.0f);
					
					/* see if the angle is in the range, if not, get it as close as possible */
					if ((new_angle < (angle + range)) && (new_angle > (angle - range))) {
						angle = (int)((new_angle * 180.0f) / M_PI);
					} else {
						/* the angle is not in the range, but get it as close as possible */
						if (new_angle > (angle + range))
							angle = (int)(((float)(aim_angle + range) * 180.0f) / M_PI);
						else
							angle = (int)(((float)(aim_angle - range) * 180.0f) / M_PI);
					}
				}
			} else {
				if ((ship->target != NULL) && (ship->target->offender != NULL)) {
					float new_angle;
					int aim_angle = (int)ship->weapon_mount[which_mount]->angle;
					int range = (int)ship->weapon_mount[which_mount]->range;
					new_angle = (float)atan2((float)((-1 * ship->target->offender->world_y) - (-1 * fires[i].world_y)), (float)(ship->target->offender->world_x - fires[i].world_x));
					
					/* convert aim_angle and range to radians :-) */
					aim_angle = (int)(((float)aim_angle * M_PI) / 180.0f);
					range = (int)(((float)range * M_PI) / 180.0f);
					
					/* see if the angle is in the range, if not, get it as close as possible */
					if ((new_angle < (angle + range)) && (new_angle > (angle - range))) {
						angle = (int)((new_angle * 180.0f) / M_PI);
					} else {
						/* the angle is not in the range, but get it as close as possible */
						if (new_angle > (angle + range))
							angle = (int)(((float)(aim_angle + range) * 180.0f) / M_PI);
						else
							angle = (int)(((float)(aim_angle - range) * 180.0f) / M_PI);
					}
				}
			}
		}
		
		fires[i].expire_time = current_time + (Uint32)ship->weapon_mount[which_mount]->weapon->lifetime;
		
		/* subtract the fire from the ship's amount to fire and set their next firing time */
		ship->weapon_mount[which_mount]->ammo--;
		
		ship->weapon_mount[which_mount]->time = current_time + (Uint32)ship->weapon_mount[which_mount]->weapon->recharge;
		
		num_fires++;
	}
}

/******************************************************************************      
*
*   Name:
*      void init_fire(void);
*
*   Abstract:
*      Sets all fire "on" statuses to false and sets SDL_Surface pointers to
*   NULL.
*
*   Context/Scope:
*      Called from init().
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
void init_fire(void) {
	int i;

	for (i = 0; i < MAX_FIRES; i++)
		fires[i].surface = NULL;

	num_fires = 0;
}

void uninit_fire(void) {
	int i;

	for (i = 0; i < MAX_FIRES; i++)
		free_fire(i);

	num_fires = 0;
}

/* updates positions of fires and also checks for collisions */
void update_fire(void) {
	int i, j;

	/* first loop to update all fires */
	for (i = 0; i < MAX_FIRES; i++) {
		if (fires[i].surface) {
			/* if the laser has expired, free the surface and set it as off, otherwise, update its information */
			if (current_time > fires[i].expire_time) {
				/* fire has expired */
				free_fire(i);
			} else {
				float factor = (((float)loop_length / 1000.0f) * (float)fires[i].velocity);
				fires[i].world_x += get_cos(fires[i].angle) * factor;
				fires[i].world_y += get_neg_sin(fires[i].angle) * factor;

				/* update the fire, hasnt expired yet */
				fires[i].screen_x = (short int)((int)fires[i].world_x - camera_x);
				fires[i].screen_y = (short int)((int)fires[i].world_y - camera_y);
			}
		}
	}

	/* second loop to check for collisions */
	for (i = 0; i < MAX_FIRES; i++) {
		if (fires[i].surface) {
			for (j = 0; j < MAX_SHIPS; j++) {
				if (ships[j]) {
					if ((current_time >= ships[j]->creation_delay) && (!ships[j]->landed) && (ships[j]->hull_strength > 0)) {
						if (fires[i].owner != ships[j]) {
							if (get_distance_sqrd(fires[i].world_x, fires[i].world_y, ships[j]->world_x, ships[j]->world_y) < (ships[j]->model->radius * ships[j]->model->radius)) {
								/* collision has occured */
								damage_ship(ships[j], &fires[i]);
								free_fire(i);
								/* do a check, it's possible the ship was destroyed and freed (and now null) in damage_ship() */
								if (ships[j])
									ship_was_fired_upon(ships[j]);
								j = num_ships; /* dont need to continue checking for more collisions, one is enough */
							}
						}
					}
				}
			}
		}
		/* need to recheck as a destroyed fire could have occured and 'i' may have been incremented */
		if (fires[i].surface) {
			for (j = 0; j < MAX_ASTEROIDS; j++) {
				if (asteroids[j].on) {
					if (get_distance_sqrd(fires[i].world_x, fires[i].world_y, asteroids[j].x, asteroids[j].y) < 2500) {
						/* collision has occured */
						damage_asteroid(j, fires[i].weapon->strength);
						free_fire(i);
						j = MAX_SHIPS; /* dont need to continue checking for more collisions, one is enough */
					}
				}
			}
		}
		/* need to recheck as a destroyed fire could have occured and 'i' may have been incremented */
		if (fires[i].surface) {
			if (fires[i].owner != player.ship) {
				if (get_distance_from_player_sqrd(fires[i].world_x, fires[i].world_y) < (player.ship->model->radius * player.ship->model->radius)) {
					damage_ship(player.ship, &fires[i]);
					free_fire(i);
				}
			}
		}
	}
}

void sort_fires(void) {
	int n, m, k, j, i;
	struct _fire *t;
	int length;

	length = num_fires; /* dont use NUM_FIRES or else it will try and use data from un-init'd structs */

	/* set all struct pointers in ordered_ships[] */
	for (i = 0; i < MAX_FIRES; i++) {
		if (i > (num_fires - 1))
			ordered_fires[i] = NULL;
		else
			ordered_fires[i] = &fires[i];
	}

	n = length;
	n = n - 1;
	m = n;

	do {
		m = (int)(m / 2);
		if (m == 0) return;
		k = n - m;
		j = 0;
		do {
			i = j;
			do {
				if (get_distance_from_player_sqrd(fires[i].world_x, fires[i].world_y) > get_distance_from_player_sqrd(fires[i + m].world_x, fires[i + m].world_y)) {
					t = ordered_fires[i];
					ordered_fires[i] = ordered_fires[i + m];
					ordered_fires[i + m] = t;
					i = i - m;
				} else
					break;
			} while (!(i < 0));
			j = j + 1;
		} while (!(j > k));
	} while (j > k);
}

static int get_free_weapon_slot(void) {
	int i;

	for (i = 0; i < MAX_FIRES; i++) {
		if (!fires[i].surface)
			return (i);
	}

	return (-1);
}

static void free_fire(short int which) {
	if (fires[which].surface) {
		SDL_FreeSurface(fires[which].surface);
		fires[which].surface = NULL;
		num_fires--;
	}
}
