#include "ai/ai.h"
#include "ai/defender.h"
#include "sprite/fire.h"
#include "system/math.h"

void update_explorer(struct _ship *ship) {
  unsigned char in_battle = 0;
  
  if (!ship)
    return;
  
  if (ship->offenders[0])
    in_battle = 1;
  
  if (in_battle) {
    if (!ship->target) {
      int i;
      
      /* find a non-allied target from list of offenders */
      for (i = 0; i < MAX_OFFENDERS; i++) {
	if (ship->offenders[i]) {
	  if (ship->offenders[i]->alliance != ship->alliance) {
	    /* found a ship that is an offender and not from our alliance */
	    ship->target = acquire_target(ship, 0, ship->offenders[i]);
	    assert(ship->target);
	    assert(ship->target->offender);
	    break;
	  }
	}
      }
    }
    
    if (ship->target) {
      assert(ship->target);
      assert(ship->target->offender);
      /* face the offender and if facing then fire */
      if (ai_turn_towards(ship, ship->target->offender->world_x, ship->target->offender->world_y)) {
	if (ship->w_slot[0] != -1)
	  fire(ship, 0);
	else if (ship->w_slot[1] != -1)
	  fire(ship, 1);
      }
      
      /* if we're too far away then get closer */
      if (get_distance_sqrd(ship->world_x, ship->world_y, ship->target->offender->world_x, ship->target->offender->world_y) > 1000000)
	ship->accel += 1;
    }
  } else {
    int dist;
    
    /* explorers fly from planet to planet */
    if (ship->destination == NULL)
      ship->destination = get_random_planet();
    
    assert(ship->destination != NULL);
    
    dist = get_distance_sqrd(ship->world_x, ship->world_y, ship->destination->world_x, ship->destination->world_y);
    
    /* face the planet and fly towards it */
    ai_turn_towards(ship, ship->destination->world_x, ship->destination->world_y);
    ship->accel++;
    
    if (dist < 10000)
      ship->destination = NULL; /* get a new one */
    
    if (dist < 1000000)
      cap_momentum(ship, 0.45);
  }
}
