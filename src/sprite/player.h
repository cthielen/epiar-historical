#ifndef PLAYER_H
#define PLAYER_H

#include "outfit/outfit.h"
#include "sprite/gate.h"
#include "sprite/sprite.h"
#include "sprite/target.h"
#include "sprite/weapon.h"

struct _player {
	struct _ship *ship;
  
	/* player specific stuff */
	struct _jump *jump;
	struct _targets target; /* this is the cycle target player's target */
  
	struct _planet *selected_planet;
	int credits;
	int free_mass;

	/* purchased outfits */
	outfit_item *upgrades[MAX_OUTFITS];
} player;

#endif
