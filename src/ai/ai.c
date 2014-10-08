/* includes main update_ai() call as well as general ai functions */

#include "ai/ai.h"
#include "ai/defender.h"
#include "ai/explorer.h"
#include "ai/gate_defender.h"
#include "ai/pirate.h"
#include "ai/trader.h"
#include "ai/warship.h"
#include "game/update.h"
#include "network/network.h"
#include "sprite/sprite.h"
#include "system/math.h"

/* main ai function */
void update_ai(void) {
	int i;
	
	/* check the type of all non-disabled ships and call their a.i. function */
	for (i = 0; i < MAX_SHIPS; i++) {
		if (ships[i]) {
			if ((!ships[i]->disabled) && (current_time >= ships[i]->creation_delay)) {
				if (ships[i]->num_objectives <= 0) {
					/* no objectives, run the standard a.i. */
					if (!strcmp(ships[i]->class, "Defender")) update_defender(ships[i]);
					if (!strcmp(ships[i]->class, "Trader")) update_trader(ships[i]);
					if (!strcmp(ships[i]->class, "Explorer")) update_explorer(ships[i]);
					if (!strcmp(ships[i]->class, "Pirate")) update_pirate(ships[i]);
					if (!strcmp(ships[i]->class, "Warship")) update_warship(ships[i]);
					if (!strcmp(ships[i]->class, "Gate Defender")) update_gate_defender(ships[i]);
				} else if (ships[i]->current_objective < ships[i]->num_objectives) {
					/* it has something specific to do */
					int cur = ships[i]->current_objective;
					
					if (ships[i]->obj_types[cur] == FLY_TO) {
						ship_obj_fly_to *obj = (ship_obj_fly_to *)ships[i]->objectives[cur];
						
						assert(obj);
						
						if (!ships[i]->destination)
							ships[i]->destination = obj->planet;
						
						assert(obj->planet);
						
						/* face the planet and fly towards it */
						ai_turn_towards(ships[i], ships[i]->destination->world_x, ships[i]->destination->world_y);
						ships[i]->accel++;
						
						if (get_distance_sqrd(ships[i]->world_x, ships[i]->world_y, ships[i]->destination->world_x, ships[i]->destination->world_y) < 10000) {
							if ((cur + 1) < ships[i]->num_objectives)
								ships[i]->current_objective++;
							else
								printf("cannot increment ship objective, there are no more objectives!\n");
						}
					} else if (ships[i]->obj_types[cur] == DESTROY) {
						ship_obj_destroy *obj = (ship_obj_destroy *)ships[i]->objectives[cur];
						
						if (obj->ship == NULL)
							obj->ship = get_ship_pointer(obj->who);
						
						assert(obj->ship);
						
						if (ships[i]->target == NULL)
							ships[i]->target = acquire_target(ships[i], 0, obj->ship);
						
						if (ships[i]->target != NULL) {
							/* face the offender and if facing then fire */
							if (ai_turn_towards(ships[i], ships[i]->target->offender->world_x, ships[i]->target->offender->world_y))
								fire(ships[i], 0);
							
							/* if we're too far away then get closer */
							if (get_distance_sqrd(ships[i]->world_x, ships[i]->world_y, ships[i]->target->offender->world_x, ships[i]->target->offender->world_y) > 40000)
								ships[i]->accel += 1;
						}
					} else if (ships[i]->obj_types[cur] == LAND_ON) {
						ship_obj_land_on *obj = (ship_obj_land_on *)ships[i]->objectives[cur];
						
						ships[i]->landed = 1;
						ships[i]->landed_on = obj->planet;
						ships[i]->current_objective++;
						
						destroy_and_respawn(ships[i], 1, 1);
					}
				}
			}
		}
	}
}

/* general ai functions */

/* returns 1 if still moving, 0 if stopped */
unsigned char slow_down(struct _ship *ship) {
	if (ship->momentum_x > 0)
		ship->momentum_x--;
	else if (ship->momentum_x < 0)
		ship->momentum_x++;
	if (ship->momentum_y > 0)
		ship->momentum_y--;
	else if (ship->momentum_y < 0)
		ship->momentum_y++;

	if((int)ship->momentum_x == 0)
		ship->momentum_x = 0;
	if((int)ship->momentum_y == 0)
		ship->momentum_y = 0;

	if ((ship->momentum_x == 0) && (ship->momentum_y == 0))
		return (0);
	else
		return (1);
}

/* returns 0 if turning, returns 1 if facing that direction */
unsigned char ai_turn_towards(struct _ship *who, int x, int y) {
	double angle;
	double opposite_angle; /* "who"'s opposite angle */
	float who_angle = who->angle;
	
	opposite_angle = (double)who_angle + 180.0f;
	if (opposite_angle > 360.0f) opposite_angle = opposite_angle - 360.0f;
	
	/* invert the y's because we're mathematically incorrect (the in-game coords are relfected over the x axis) */
	angle = (double)atan2((double)((double)(-1 * y) - (double)(-1 * who->world_y)), (double)(x - who->world_x));
	angle = ((double)angle * 180.0f) / (3.1415926f);

	if(who_angle > (angle + 180))
		who_angle -= 360.0f;

	if (((who_angle - 15) < angle) && ((who_angle + 15) > angle))
		return (1);
	else {
		/* turn towards 'em */
		int angle_at;
		int angle_want;
		int x;
		angle_at = who_angle;
		angle_want = angle;
		if (angle_at < angle_want)
			angle_at += 360;
		x = angle_at - angle_want;
		/* avoid jerkiness in turning */
		if ((x > 10) && (x < 350)) {
			if (x > 180)
				turn_ship(who, 0);
			else
				turn_ship(who, 1);
		}
	}
	
	return (0);
}

/* caps momentum to specified value */
void cap_momentum(struct _ship *ship, float value) {
	if (ship->momentum_x > value) ship->momentum_x = value;
	if (ship->momentum_y > value) ship->momentum_y = value;
	if (ship->momentum_x < -1 * value) ship->momentum_x = -1 * value;
	if (ship->momentum_y < -1 * value) ship->momentum_y = -1 * value;
}

/* returns true if within 150 units of (x, y) */
unsigned char near_point(struct _ship *ship, int x, int y) {
	int dist = get_distance_sqrd(ship->world_x, ship->world_y, x, y);

	if (dist < 22500)
		return (1);

	return (0);
}

/* general targetting functions */

/* if closest set, returns closest. else return target struct to target_him */
struct _target *acquire_target(struct _ship *who, unsigned char closest, struct _ship *target_him) {
	struct _target *target = NULL;
	int i;
	struct _ship *ship = NULL;
	
	target = malloc(sizeof(struct _target));
	
	if (target == NULL) {
		fprintf(stdout, "Failed to get enough memory to acquire a target\n");
		return (NULL);
	}
	
	target->been_warned = 0;
	target->shots_fired = 0;
	
	/* find closest ship */
	if (closest) {
		for (i = 0; i < MAX_TARGETS; i++) {
			if (targets[i].type == TARGET_SHIP) {
				struct _ship *ship = (struct _ship *)targets[i].target;
				
				if (ship != who) /* let's not target ourselves now */
					break;
			}
		}
		if (who == player.ship)
			if (i == MAX_TARGETS) /* player has run out of targets */
				return (NULL);
		
		if (i == MAX_TARGETS) {
			/* they didn't find anybody, and due to the above check, it isnt the player, so let's target the player */
			target->offender = player.ship;
		} else {
			/* target the ship */
			ship = (struct _ship *)targets[i].target;
			target->offender = ship;
			/* this function should really be a little better, like, who wants the target so a.i. can use it? */
			if (!target->offender)
				target->offender = player.ship;
		}
		return (target);
	} else {
		if (target_him) {
			target->offender = target_him;
			return (target);
		}
	}
	
	fprintf(stdout, "acquire_target(): requested ship is NULL\n");
	
	/* just return null i guess */
	free(target);
	target = NULL;
	
	return (target);
}

/* removes target */
void loose_target(struct _target *target) {
	if (target)
		free(target);
}

/* function to let ships keep track of the last ships that shot them */
void add_offender(struct _ship *ship, struct _ship *offender) {
	int i;

	for (i = MAX_OFFENDERS - 1; i > 0; i--) {
		ship->offenders[i] = ship->offenders[i-1];
	}
	ship->offenders[0] = offender;
}

 /* ship is who, speed is % of max speed (1 = max) */
void goto_dest_point(struct _ship *ship, float speed) {
	if (ship == NULL)
		return;

	if (ai_turn_towards(ship, ship->ai.dest_point[ship->ai.dest_heading][0], ship->ai.dest_point[ship->ai.dest_heading][1])) {
		/* if() returned true, so we are facing the point, so speed up */
		ship->accel++;
		cap_momentum(ship, ship->engine->top_speed * speed);
	} else {
		slow_down(ship);
	}
}

/* if no planet found, returns the planet in the first planet slot */
struct _planet *get_friendly_planet(struct _alliance *alliance) {
	int i, rand_planet;

	rand_planet = (int)rand() % num_planets;

	for (i = rand_planet; i < num_planets; i++) {
		if (alliance == planets[i]->alliance)
			return (planets[i]);
	}
	for (i = 0; i < rand_planet; i++) {
		if (alliance == planets[i]->alliance)
			return (planets[i]);
	}

	/* found nothing, so return the first planet (eh, whatever) */
	return (planets[0]);
}

struct _planet *get_random_planet(void) {
	int rand_planet;

	rand_planet = (int)rand() % num_planets;

	return (planets[rand_planet]);
}

/* inform all a.i. (in case of objective or target) that 'ship' was destroyed */
void ai_ship_destroyed(struct _ship *ship) {
	int i, j;

	for (i = 0; i < MAX_SHIPS; i++) {
		/* if this is any ship's target, clear that */
		if (ships[i]) {
			if (ships[i]->target) {
				if (ships[i]->target->offender == ship) {
					loose_target(ships[i]->target);
					ships[i]->target = NULL;
				}
			}

			/* if any of this ship's objectives depend on 'ship', inform them */
			for (j = 0; j < ships[i]->num_objectives; j++) {
				if (ships[i]->obj_types[j] == DESTROY) {
					ship_obj_destroy *obj = (ship_obj_destroy *)ships[i]->objectives[j];
					
					if (obj->ship == ship)
						ships[i]->current_objective++; /* accomplished, congrats */
				}
			}
		}
	}
}

/* alerts a.i. if they've been fired upon */
void ship_was_fired_upon(struct _ship *ship) {
	if (ship) {
		if (!strcmp(ship->class, "Pirate")) {
			if (!(ship->status & SHIP_CLOAKING)) {
				cloak_ship(ship);
			}
		}
	}
}

float get_distance_to_closest_person_sqrd(struct _ship *ship) {
	float closest, dist;
	int i;

	if (!ship)
		return (0.0f);

	if (num_ships < 1)
		return (0.0f);

	closest = get_distance_from_player_sqrd((float)ship->world_x, (float)ship->world_y);

	for (i = 0; i < MAX_SHIPS; i++) {
		dist = 0.0f;

		if (ships[i]) {
			if (ships[i] != ship) {
				dist = get_distance_sqrd((float)ship->world_x, (float)ship->world_y, (float)ships[i]->world_x, (float)ships[i]->world_y);
				
				if (dist < closest)
					closest = dist;
			}
		}
	}

	return (closest);
}

struct _ship *get_closest_person(struct _ship *ship) {
	float closest, dist;
	int i;
	struct _ship *closest_ship = NULL;

	if (!ship)
		return (NULL);

	if (num_ships < 1)
		return (NULL);

	closest = get_distance_from_player_sqrd((float)ship->world_x, (float)ship->world_y);

	for (i = 0; i < num_ships; i++) {
		dist = 0.0f;

		if (ships[i] != ship) {
			dist = get_distance_sqrd((float)ship->world_x, (float)ship->world_y, (float)ships[i]->world_x, (float)ships[i]->world_y);

			if (dist < closest) {
				closest = dist;
				closest_ship = ships[i];
			}
		}
	}

	return (closest_ship);
}
