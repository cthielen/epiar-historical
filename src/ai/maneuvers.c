#include "ai/ai.h"
#include "ai/maneuvers.h"
#include "sprite/sprite.h"
#include "system/math.h"

/* turns ship so that the cloest weapon mount is facing the ship's target and fires that mount - good for heavy, slow ships with lots of turrets */
void maneuver_nearest_mount(struct _ship *ship) {
	int i;
	float angle_to;
	int closest_angle = 360;
	int closest_mount = -1;

	if (!ship->target) {
#ifndef NDEBUG
		printf("nearest_mount() called but ship has no target\n");
#endif
		return;
	}

	angle_to = get_angle_to(ship->world_x, ship->world_y, ship->target->offender->world_x, ship->target->offender->world_y);
	
	/* figure out which mount is closest to hitting the target, and make decisions for that mount */
	for (i = 0; i < MAX_WEAPON_SLOTS; i++) {
		if (ship->weapon_mount[i]) {
			int angle = angle_to - (ship->angle + ship->weapon_mount[i]->angle);
			angle %= 360;
			if (abs(angle) < closest_angle) {
				closest_angle = abs(angle_to - (ship->angle + ship->weapon_mount[i]->angle));
				closest_mount = i;
			}
		}
	}
	
	if (closest_mount != -1) {
		assert(ship->weapon_mount[closest_mount]);
		if (ship->weapon_mount[closest_mount]->range != 0) {
			/* okay, range weapon, let's turn just enough to get them in range (er, well in range) */
			/* if they're in range, fire, else, turn to get them in range */
			int weap_angle = ship->angle + ship->weapon_mount[closest_mount]->angle;
			int range = ship->weapon_mount[closest_mount]->range / 2;
			if (((weap_angle + range) > angle_to) && ((weap_angle - range) < angle_to)) {
				/* in range, fire away */
				fire(ship, closest_mount);
			} else {
				/* not in range, turn */
				if ((angle_to > weap_angle) && (angle_to < (weap_angle + 180))) {
					turn_ship(ship, 0);
				} else {
					turn_ship(ship, 1);
				}
			}
		}
	}
}

/* fly by attack, flies at the enemy firing and turns around once past and does it again */
void maneuver_fly_by(struct _ship *ship) {
	float dist;

	if (!ship->target) {
#ifndef NDEBUG
		printf("maneuver_fly_by() called but ship has no target\n");
#endif
		return;
	}

	/* we do this maneuver by checking distances - if we're a certain distance from the target, turn and face it, if we're within distance, aim and fire, however, if we're within distance but the target is generally behind us (like we just passed it), then we fly out of distance. */
	dist = get_distance_sqrd(ship->world_x, ship->world_y, ship->target->offender->world_x, ship->target->offender->world_y);

	/* 500 real world units but we're using squared distances for speed */
	if (dist > 250000.0f) {
		int facing;

		/* turn and go for the enemy */
		facing = ai_turn_towards(ship, ship->target->offender->world_x, ship->target->offender->world_y);

		/* slow down to turn */
		if (facing)
			ship->accel++;
		else
			slow_down(ship);

	} else {
		/* we're within range, find our angle to the target */
		float angle;

		angle = get_angle_to(ship->world_x, ship->world_y, ship->target->offender->world_x, ship->target->offender->world_y);

		/* whether we're escaping or attacking, we should go fast */
		ship->accel++;

		if (((ship->angle + 60) > angle) && (ship->angle - 60) < angle) {
			/* we're heading toward the ship, continue to do so and fire if close enough */
			/* turn toward the enemy unless we're close (within 175 real units) */
			if (dist > 40000)
				ai_turn_towards(ship, ship->target->offender->world_x, ship->target->offender->world_y);
			if (dist < 360000) {
				if (ship->w_slot[0] != -1)
					fire(ship, 0);
				if (ship->w_slot[1] != -1)
					fire(ship, 1);
			}
		} else {
			/* we're within range, but not heading toward the target, so it's most likely we just passed it, in which case keep heading away until we get out of range (so this maneuver can restart) */
			int escape_angle = (int)angle + 180;
			escape_angle %= 360;

			if (escape_angle > (ship->angle + 15))
				turn_ship(ship, 0);
			else if (escape_angle < (ship->angle - 15))
				turn_ship(ship, 1);
		}
	}
}

/* flies in the direction opposite that of the target */
void maneuver_fly_away(struct _ship *ship) {
	float angle;

	if (!ship)
		return;
	if (!ship->target) {
#ifndef NDEBUG
		printf("maneuver_fly_away called with no target\n");
#endif
		return;
	}

	angle = get_angle_to(ship->world_x, ship->world_y, ship->target->offender->world_x, ship->target->offender->world_y);

	/* get the opposite angle */
	angle = (float)((int)(angle + 180) % 360);

	if (angle > (ship->angle + 15))
		turn_ship(ship, 0);
	else if (angle < (ship->angle - 15))
		turn_ship(ship, 1);

	ship->accel++;
}
