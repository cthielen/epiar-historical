#include "force/force.h"
#include "game/update.h"
#include "system/math.h"

#define MAX_FORCES 3

#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

struct {
	f_type type;
	int x, y;
	float strength;
	int dist_sqrd;
	Uint32 lifetime_left;
} forces[MAX_FORCES];

int num_forces = 0;

void init_force(void) {
	int i;

	for (i = 0; i < MAX_FORCES; i++) {
		forces[i].lifetime_left = 0;
	}

	num_forces = 0;
}

void deinit_force(void) {
	num_forces = 0;
}

/* function which updates forces and applies changed coordinates and momentums to ships affected by the forces */
/* NOTE: this function does a little too much math and should be optimizied, luckily, it shouldnt occur too often */
void update_force(void) {
	int i;

	for (i = 0; i < MAX_FORCES; i++) {
		if (forces[i].lifetime_left != 0) {
			if (((signed)forces[i].lifetime_left - (signed)loop_length) <= 0) {
				forces[i].lifetime_left = 0;
			} else {
				int dist = get_distance_from_player_sqrd(forces[i].x, forces[i].y);
				int true_dist = sqrt(dist);

				forces[i].lifetime_left -= loop_length;

				if (dist < forces[i].dist_sqrd) {
					float theta;
					float mag;

					/* get player's angle to the explosion */
					theta = atan(((float)-forces[i].y - -player.ship->world_y) / ((float)forces[i].x - player.ship->world_x));
					/* reverse the angle (so the angle is away, the way to push) */
					theta += M_PI;

					/* check quadrants */
					if ((forces[i].x - player.ship->world_x) < 0)
					  theta += M_PI;

					mag = forces[i].strength * (1.0f - (true_dist / sqrt(forces[i].dist_sqrd)));

					/* mag is in pixels per second but we're doing updates faster than that, so convert from seconds */
					mag *= (float)loop_length / 1000.0f;

					player.ship->momentum_x += cos(theta) * mag;
					player.ship->momentum_y += -sin(theta) * mag;
				}
			}
		}
	}
}

void new_force(f_type type, int x, int y, float strength, short int dist, Uint32 lifetime) {
	int i;

	for (i = 0; i < MAX_FORCES; i++) {
		if (!forces[i].lifetime_left) {
			forces[i].type = type;
			forces[i].x = x;
			forces[i].y = y;
			forces[i].strength = strength;
			forces[i].dist_sqrd = dist * dist;
			forces[i].lifetime_left = lifetime;
			return;
		}
	}
}
