#include "ai/ai.h"
#include "ai/warship.h"
#include "sprite/sprite.h"
#include "system/math.h"

void update_warship(struct _ship *ship) {
	int i;
	
	if (!ship)
		return;

	if (!ship->target) {
		/* find a non-allied target */
		for (i = 0; i < MAX_SHIPS; i++) {
			if (ships[i]) {
				if (ships[i]->alliance != ship->alliance) {
					/* found a ship that is an offender and not from our alliance */
					ship->target = acquire_target(ship, 0, ships[i]);
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
}
