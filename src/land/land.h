#ifndef LAND_H
#define LAND_H

#include "outfit/outfit.h"
#include "sprite/planet.h"
#include "sprite/model.h"

struct _planet_features {
	unsigned char outfit, shipyard, employment, bar;
	model_t *ships_available[MAX_MODELS];
	outfit_item *outfits_available[MAX_OUTFITS];
	int num_ships, num_outfits;
} planet_features;

typedef enum _land_options {DEPART, SUMMARY, OUTFIT, SHIPYARD, EMPLOYMENT, BAR} land_options;

void toggle_land(void);
void land_on(struct _planet *planet);

#endif /* LAND_H */
