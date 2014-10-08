#include "ai/ai.h"
#include "ai/gate_defender.h"
#include "sprite/gate.h"

void update_gate_defender(struct _ship *ship) {
	if(!ship)
		return;

	/* gate defenders simply patrol for now */
	goto_dest_point(ship, 0.5);
	if (near_point(ship, ship->ai.dest_point[ship->ai.dest_heading][0], ship->ai.dest_point[ship->ai.dest_heading][1])) {
		ship->ai.dest_heading++;
		if (ship->ai.dest_heading > 3)
			ship->ai.dest_heading = 0;
	}
}
