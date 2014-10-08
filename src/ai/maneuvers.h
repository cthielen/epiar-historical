#ifndef H_MANEUVERS
#define H_MANEUVERS

#include "ai/ai.h"
#include "sprite/ship.h"

/* a.i. tactic that turns the ship so that the nearest weapon mount faces the target and then fires that mount */
void maneuver_nearest_mount(struct _ship *ship);
void maneuver_fly_by(struct _ship *ship);
void maneuver_fly_away(struct _ship *ship);

#endif /* H_MANEUVERS */
