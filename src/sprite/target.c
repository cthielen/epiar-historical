#include "comm/comm.h"
#include "game/update.h"
#include "includes.h"
#include "sprite/gate.h"
#include "sprite/sprite.h"
#include "sprite/target.h"

struct _targets targets[MAX_TARGETS]; /* holds information for target cycling */
static int target_selected = -1; /* used for target cycling */

/* contructs list of closest hailable objects */
void update_targets(void) {
	int n, m, k, j, i;
	int length;
	int num_targets = 0;
	struct _targets t;
	
	length = num_ships + num_gates;
	
	/* first, throw everything into the list in whatever order and record distance from player (what we're ordering by) */
	/* add ships */
	for (j = 0; j < MAX_SHIPS; j++) {
		if (ships[j] && (current_time >= ships[j]->creation_delay)) {
			targets[num_targets].target = ships[j];
			targets[num_targets].type = TARGET_SHIP;
			targets[num_targets].dist = get_distance_from_player_sqrd(ships[j]->world_x, ships[j]->world_y);
			num_targets++;
		}
	}
	/* add gates */
	for (j = 0; j < num_gates; j++) {
		targets[num_targets].target = gates[j];
		targets[num_targets].type = TARGET_GATE;
		targets[num_targets].dist = get_distance_from_player_sqrd(gates[j]->world_x, gates[j]->world_y);
		num_targets++;
	}
	/* fill the rest with null */
	for (; num_targets < MAX_TARGETS; num_targets++) {
		targets[num_targets].target = NULL;
		targets[num_targets].type = TARGET_NONE;
		targets[num_targets].dist = 999999999.99f;
	}
	
	n = length;
	n = n - 1;
	m = n;
	
	do {
		m = (int)(m/2);
		if (m == 0) return;
		k = n - m;
		j = 0;
		do {
			i = j;
			do {
				if (targets[i].dist > targets[i+m].dist) {
					/* record item in t */
					t.target = targets[i].target;
					t.type = targets[i].type;
					t.dist = targets[i].dist;
					/* and swap */
					targets[i].target = targets[i+m].target;
					targets[i].type = targets[i+m].type;
					targets[i].dist = targets[i+m].dist;
					targets[i+m].target = t.target;
					targets[i+m].type = t.type;
					targets[i+m].dist = t.dist;
					i=i-m;
				} else
					break;
			} while (!(i < 0));
			j = j + 1;
		} while (!(j > k));
	} while (j > k);
}

void reset_target_cycle(void) {
	target_selected = -1;
}

void cycle_targets(void) {
	static Uint32 last_cycle = 0;
	extern Uint32 current_time;

	if (current_time < (last_cycle + 85)) {
		return; /* if hasnt been a half second, so ignore this request */
	} else {
		last_cycle = current_time; /* we're gonna do the cycling, so record this time for later checking */
	}
	
	target_selected++;
	
	if ((target_selected > MAX_TARGETS) || (target_selected > (num_ships + num_gates)) || (targets[target_selected].type == TARGET_NONE) || (targets[target_selected].dist > COMM_DIST_SQRD)) {
		target_selected = -1;
	}
	
	if (target_selected == -1) {
		player.target.target = NULL;
		player.target.type = TARGET_NONE;
		player.target.dist = 0.0f;
	} else {
		player.target.target = targets[target_selected].target;
		player.target.type = targets[target_selected].type;
		player.target.dist = targets[target_selected].dist;
	}

	if(player.target.type == TARGET_SHIP) {
		struct _ship *ship = (struct _ship *)player.target.target;

		if(ship->status & SHIP_CLOAKING) {
			last_cycle -= 85;
			cycle_targets(); /* can't target a cloaked ship */
		}
	}
}

void set_target_nearest(void) {
	player.target = targets[0];
}
