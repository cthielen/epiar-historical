#include "audio/audio.h"
#include "com_defs.h"
#include "game/scenario.h"
#include "gui/gui.h"
#include "hud/hud.h"
#include "includes.h"
#include "land/land.h"
#include "land/land_dlg.h"
#include "missions/missions.h"
#include "outfit/outfit.h"
#include "sprite/planet.h"
#include "sprite/player.h"
#include "sprite/model.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/math.h"
#include "system/path.h"
#include "system/video/video.h"

#define MAX_DIST 175000 /* how close you have to be to select a planet */

/* the following four functions are used to know whether or not a button should be displayed for a particular planet */
static void discern_planet_features(char *planet);
static void do_planet_hud_message(void);

void land_on(struct _planet *planet) {
	assert(planet != NULL);
	
	mission_player_landed(planet);
	
	discern_planet_features(planet->name);
	
	do_landing_dialog(planet);
}

void toggle_land(void) {
	struct _planet *planet = NULL;
	float planet_dist = 0.0; /* used to know how close the planet found is during comparison */
	int which = -1;
	
	/* if we're over a planet, land on it and deselect it, otherwise do the normal thing of selecting/unselecting the nearest */
	if (player.selected_planet) {
		if (player.ship->world_x > player.selected_planet->world_x)
			if (player.ship->world_x < (player.selected_planet->world_x + player.selected_planet->image->w))
				if (player.ship->world_y > player.selected_planet->world_y)
					if (player.ship->world_y < (player.selected_planet->world_y + player.selected_planet->image->h)) {
						if (player.selected_planet->landable) {
							stop_audio();
							land_on(player.selected_planet);
							resume_audio();
							erase_boxed_selection(player.selected_planet->screen_x, player.selected_planet->screen_y, player.selected_planet->image->w, player.selected_planet->image->h);
							player.selected_planet = NULL;
						} else {
							hud_message("You cannot land on that.", 3500);
						}
						return;
					}
	}
	
	if (player.selected_planet) {
		/* a planet is selected, so unselect it */
		erase_boxed_selection(player.selected_planet->screen_x, player.selected_planet->screen_y, player.selected_planet->image->w, player.selected_planet->image->h);
		player.selected_planet = NULL;
	} else {
		int i;
		float j;
		
		planet_dist = 999999999.999f;
		player.selected_planet = NULL;
		
		/* go through and find a planet within range. if multiple in range, use closest */
		for (i = 0; i < num_planets; i++) {
			/* w/h / 2 because we want dist from the center of the planet (noticeable when two planets are close together) */
			j = get_distance(player.ship->world_x, player.ship->world_y, (float)planets[i]->world_x + ((float)planets[i]->image->w / 2.0f), (float)planets[i]->world_y + ((float)planets[i]->image->h / 2.0f));
			if ((j < planet_dist) && (j < (float)MAX_DIST)) {
				planet_dist = j;
				player.selected_planet = planets[i];
			}
		}
		
		/* if there's no planet in range */
		if (!player.selected_planet)
			hud_message("No planet in range.", 3500);
		else
			do_planet_hud_message();
	}
}

static void discern_planet_features(char *planet) {
	int i, j, k, l, cur;
	parsed_file *planets_esf = NULL;
	unsigned char found_planet = 0;
	char *items = NULL;
	char temp[60] = {0};
	int already_parsed_ships = 0;
	
	planet_features.outfit = 0;
	planet_features.shipyard = 0;
	planet_features.employment = 0;
	planet_features.bar = 0;
	planet_features.num_ships = 0;
	planet_features.num_outfits = 0;
	
	/* clear the ships_available variable */
	for (i = 0; i < MAX_MODELS; i++) {
		planet_features.ships_available[i] = NULL;
		planet_features.outfits_available[i] = NULL;
	}
	
	if ((planets_esf = esf_new_handle()) == NULL) {
		printf("Couldn't create parser handle\n");
		return;
	}
	
	if (esf_set_filter(planets_esf, "planet") != 0) {
		printf("Couldn't set filter\n");
		esf_close_handle(planets_esf);
		return;
	}
	
	if (esf_parse_file_eaf(loaded_eaf, planets_esf, "planets.esf") != 0) {
		printf("Couldn't parse \"planets.esf\"\n");
		esf_close_handle(planets_esf);		
	} else {
		/* run through the parsed information */
		
		for (i = 0; i < planets_esf->num_items; i++) {
			found_planet = 0;
			
			for (j = 0; j < planets_esf->items[i].num_keys; j++) {
				char *name = planets_esf->items[i].keys[j].name;
				
				if (!strcmp(name, "Name")) {
					char *planet_name = planets_esf->items[i].keys[j].value.cp;
					
					if (!strcmp(planet, planet_name)) {
						found_planet = 1;
						already_parsed_ships = 1;
					}
				}
			}
			
			/* the subitems is where most of the info we want is */
			for (k = 0; k < planets_esf->items[i].num_subitems; k++) {
				for (l = 0; l < planets_esf->items[i].subitems[k].num_keys; l++) {
					char *name = planets_esf->items[i].subitems[k].keys[l].name;
					
					if (!strcmp(name, "Shipyard Item") && found_planet) {
						char *ship = planets_esf->items[i].subitems[k].keys[l].value.cp;
						
						planet_features.shipyard = 1;
						
						planet_features.ships_available[planet_features.num_ships] = get_model_pointer(ship);
						assert(planet_features.ships_available[planet_features.num_ships]);
						planet_features.num_ships++;
					}
				}
			}
		}
	}
	
	if (esf_close_handle(planets_esf) != 0) {
		printf("Could not close parser handle\n");
		return;
	}
	
	/* if planet wasnt found, look through the main.eaf file for it */
	if (!already_parsed_ships) {
		if ((planets_esf = esf_new_handle()) == NULL) {
			printf("Couldn't create parser handle\n");
			return;
		}
		
		if (esf_set_filter(planets_esf, "planet") != 0) {
			printf("Couldn't set filter\n");
			esf_close_handle(planets_esf);
			return;
		}
		
		if (esf_parse_file_eaf(main_eaf, planets_esf, "planets.esf") != 0) {
			printf("Couldn't parse \"planets.esf\"\n");
			esf_close_handle(planets_esf);		
		} else {
			/* run through the parsed information */
			
			for (i = 0; i < planets_esf->num_items; i++) {
				found_planet = 0;
				
				for (j = 0; j < planets_esf->items[i].num_keys; j++) {
					char *name = planets_esf->items[i].keys[j].name;
					
					if (!strcmp(name, "Name")) {
						char *planet_name = planets_esf->items[i].keys[j].value.cp;
						
						if (!strcmp(planet, planet_name))
							found_planet = 1;
					}
				}
				
				/* the subitems is where most of the info we want is */
				for (k = 0; k < planets_esf->items[i].num_subitems; k++) {
					for (l = 0; l < planets_esf->items[i].subitems[k].num_keys; l++) {
						char *name = planets_esf->items[i].subitems[k].keys[l].name;
						
						if (!strcmp(name, "Shipyard Item") && found_planet) {
							char *ship = planets_esf->items[i].subitems[k].keys[l].value.cp;
							
							planet_features.shipyard = 1;
							
							planet_features.ships_available[planet_features.num_ships] = get_model_pointer(ship);
							assert(planet_features.ships_available[planet_features.num_ships]);
							planet_features.num_ships++;
						}
					}
				}
			}
		}
		
		if (esf_close_handle(planets_esf) != 0) {
			printf("Could not close parser handle\n");
			return;
		}
	}
	
	/* now look for outfit items */
	items = get_outfits(planet);
	
	memset(temp, 0, sizeof(char) * 60);
	cur = 0;
	
	if (items) {
		/* +1 because we need to see that last '\n' */
		for (i = 0; i < (signed)strlen(items) + 1; i++) {
			if (items[i] != '\n') {
				temp[cur] = items[i];
				cur++;
			} else {
				planet_features.outfits_available[planet_features.num_outfits] = get_outfit_pointer(temp);
				planet_features.num_outfits++;
				memset(temp, 0, sizeof(char) * 60);
				cur = 0;
			}
		}
		free(items);
	}
}

/* sends a message to the hud like, "you're cleared to land" or something */
static void do_planet_hud_message(void) {
	char msg[80];
	int which = rand() % 5;
	
	if (!player.selected_planet->landable) {
		sprintf(msg, "You may not dock at %s, pilot.", player.selected_planet->name);
	} else {
		if (which == 0)
			sprintf(msg, "%s traffic control receieves you, you're cleared to land.", player.selected_planet->name);
		else if (which == 1)
			sprintf(msg, "%s docking control has you, landing clearance confirmed.", player.selected_planet->name);
		else if (which == 2)
			sprintf(msg, "You are cleared to land, pilot. Welcome to %s.", player.selected_planet->name);
		else if (which == 3)
			sprintf(msg, "Landing confirmed. Welcome to %s, pilot.", player.selected_planet->name);
		else
			sprintf(msg, "Your docking code is confirmed. Welcome to %s.", player.selected_planet->name);
	}
	
	hud_message(msg, 4000);
}
