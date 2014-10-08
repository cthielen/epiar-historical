#include "ai/ai.h"
#include "ai/defender.h"
#include "ai/maneuvers.h"
#include "sprite/fire.h"
#include "system/math.h"

void update_trader(struct _ship *ship) {
	unsigned char in_battle = 0;
  
	if (!ship)
		return;
	
	if (ship->offenders[0])
		in_battle = 1;
	
	if (in_battle) {
		if (!ship->target) {
			int i;
			
			/* find a non-allied target from list of offenders (if the ships are both independent, theyll attack anyway) */
			for (i = 0; i < MAX_OFFENDERS; i++) {
				if (ship->offenders[i]) {
					if ((ship->offenders[i]->alliance != ship->alliance) || (!strcmp(ship->offenders[i]->alliance->name, "Independent"))) {
						/* found a ship that is an offender and not from our alliance */
						ship->target = acquire_target(ship, 0, ship->offenders[i]);
						break;
					} else {
						printf("found a ship but they're from our alliance\n");
					}
				}
			}

			if (!ship->target) {
				/* no non-friendly offenders, clear the list */
				for (i = 0; i < MAX_OFFENDERS; i++) {
					if (ship->offenders[i])
						ship->offenders[i] = NULL;
				}
			}
		}
		
		if (ship->target) {
			if (ship->hull_strength > (ship->model->hull_life / 2)) {
				float dist;

				dist = get_distance_sqrd(ship->world_x, ship->world_y, ship->target->offender->world_x, ship->target->offender->world_y);

				/* attack, but if we're too close (on them), move away */
				if (dist < 10000) {
					maneuver_fly_away(ship);
				} else {
					/* attack if above 50% hull strength */
					if (ship->model->str < 3)
						maneuver_fly_by(ship);
					else
						maneuver_nearest_mount(ship);
				}
			} else {
				/* too weak, run away! */
				maneuver_fly_away(ship);
			}
		}
#ifndef NDEBUG
		else
			printf("couldnt do maneuver - no target (trader ai)\n");
#endif
	} else {
		/* trader flies between planets */
		if (!ship->destination)
			ship->destination = planets[get_closest_planet(ship->world_x, ship->world_y)];

		if (ship->destination) {
			/* if the closest planet is very close (under us), find another */
			if (get_distance_sqrd(ship->world_x, ship->world_y, ship->destination->world_x, ship->destination->world_y) < 10000)
				ship->destination = get_friendly_planet(ship->alliance);

			/* face the planet and fly towards it */
			ai_turn_towards(ship, ship->destination->world_x, ship->destination->world_y);
			ship->accel++;
			
			if (get_distance_sqrd(ship->world_x, ship->world_y, ship->destination->world_x, ship->destination->world_y) < 10000)
				ship->destination = NULL; /* get a new one */
		} else {
#ifndef NDEBUG
			printf("trader a.i. error, couldnt get a planet after a few tries\n");
#endif
		}
	}
}
