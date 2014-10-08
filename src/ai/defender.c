#include "ai/ai.h"
#include "ai/defender.h"
#include "sprite/fire.h"
#include "system/math.h"

void update_defender(struct _ship *ship) {
	unsigned char in_battle = 0;
	int i;
	
	if (ship == NULL)
		return;
	
	if (ship->offenders[0] != NULL)
		in_battle = 1;
	
	if (in_battle) {
		if (ship->target == NULL) {
			/* find a non-allied target from list of offenders */
			for (i = 0; i < MAX_OFFENDERS; i++) {
				if (ship->offenders[i]) {
					if (ship->offenders[i]->alliance != ship->alliance) {
						/* found a ship that is an offender and not from our alliance */
						ship->target = acquire_target(ship, 0, ship->offenders[i]);
						break;
					}
				}
			}
		}
		
		/* temp. fix. for some reason target _can_ be null at this point in the code, although it shouldnt be */
		if (ship->target) {
			if (ship->model->str > 6)
				maneuver_nearest_mount(ship);
			else
				maneuver_fly_by(ship);
		}
	} else {
		/* defender simply patrols, however, if it has been fired upon or it's planet has any offenders in the area, it follows or attacks them */
		goto_dest_point(ship, 0.5);
		if (near_point(ship, ship->ai.dest_point[ship->ai.dest_heading][0], ship->ai.dest_point[ship->ai.dest_heading][1])) {
			ship->ai.dest_heading++;
			if (ship->ai.dest_heading > 3)
				ship->ai.dest_heading = 0;
		}
	}
}
