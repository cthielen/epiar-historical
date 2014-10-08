#include "ai/ai.h"
#include "ai/pirate.h"
#include "sprite/sprite.h"
#include "system/math.h"

void update_pirate(struct _ship *ship) {
	float dist = 0.0f;
	
	if (ship == NULL) {
		printf("void update_pirate(struct _ship *ship): Ship is NULL\n");
		return;
	}
	
	/* if pirate is cloaked, get away and decloak, if ship is not cloaked, find somebody and pick on them */
	if (ship->status & SHIP_CLOAKING) {
		/* get away and if far enough, decloak */
		if (!ship->destination)
			ship->destination = get_random_planet();
		
		assert(ship->destination);
		
		ai_turn_towards(ship, ship->destination->world_x, ship->destination->world_y);
		ship->accel++;
		
		/* if we're far enough away, decloak */
		dist = get_distance_to_closest_person_sqrd(ship);
		
		if (dist > 2250000)
			decloak_ship(ship);
	} else {
		/* get a target (if you dont have one) and fight */
		unsigned char facing;
		
		if (!ship->target)
			ship->target = acquire_target(ship, 1, NULL);
		
		assert(ship->target);
		assert(ship->target->offender);
		
		/* if you can face them fast enough to fire, do that */
		if (ship->model->str < 6) {
			maneuver_fly_by(ship);
		} else {
			maneuver_nearest_mount(ship);
		}
	}
}
