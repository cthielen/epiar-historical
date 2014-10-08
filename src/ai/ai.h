#ifndef H_AI
#define H_AI

#include "ai/maneuvers.h"
#include "alliances/alliances.h"
#include "sprite/ship.h"

/* main ai functions */
void update_ai(void);

/* general ai functions */
unsigned char slow_down(struct _ship *ship);
unsigned char ai_turn_towards(struct _ship *ship, int x, int y);
void cap_momentum(struct _ship *ship, float value);
unsigned char near_point(struct _ship *ship, int x, int y);
void add_offender(struct _ship *ship, struct _ship *offender);
void goto_dest_point(struct _ship *ship, float speed); /* ship is who, speed is % of max speed (1 = max) */
struct _planet *get_friendly_planet(struct _alliance *alliance);
struct _planet *get_random_planet(void);
void ship_was_fired_upon(struct _ship *ship);
float get_distance_to_closest_person_sqrd(struct _ship *ship);

/* general targetting functions */
struct _target *acquire_target(struct _ship *who, unsigned char closest, struct _ship *target_him);
void loose_target(struct _target *target);
void ai_ship_destroyed(struct _ship *ship);

#endif /* H_AI */
