#include "alliances/alliances.h"
#include "asteroid/asteroid.h"
#include "com_defs.h"
#include "game/scenario.h"
#include "includes.h"
#include "sprite/planet.h"
#include "sprite/sprite.h"
#include "system/eaf.h"
#include "system/esf.h"
#include "system/math.h"
#include "system/path.h"
#include "system/video/video.h"

int num_planets = 0;

static struct _planet *create_default_planet(void);
static void free_planet(struct _planet *planet);

static int add_planet(struct _planet *planet) {
	if (num_planets < MAX_PLANETS) {
		int i;
		planets[num_planets] = planet;

		planet->revealed = 0;

		/* spawn the planet's defenders, give them their dest points and homeworld */
		if (!skip_ship_load) {
			for (i = 0; i < planet->num_defenders; i++) {
				struct _ship *defender = create_ship();
				char name[40] = {0};

				sprintf(name, "%s Defender #%d", planet->name, i);

				defender->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
				memset(defender->name, 0, sizeof(char) * (strlen(name) + 1));
				strcpy(defender->name, name);
				defender->model = planet->defender_model;
				defender->alliance = planet->alliance;
				defender->class = (char *)malloc(sizeof(char) * (strlen("Defender") + 1));
				memset(defender->class, 0, sizeof(char) * (strlen("Defender") + 1));
				strcpy(defender->class, "Defender");
				defender->initial_x = (int)((planet->world_x - 500) + (rand() % 1000));
				defender->initial_y = (int)((planet->world_y - 500) + (rand() % 1000));
				add_ship(defender);
				/* set the four dest points to be a box around the planet */
				defender->ai.dest_point[0][0] = planet->world_x - 1000;
				defender->ai.dest_point[0][1] = planet->world_y - 1000;
				defender->ai.dest_point[1][0] = planet->world_x + 1000;
				defender->ai.dest_point[1][1] = planet->world_y - 1000;
				defender->ai.dest_point[2][0] = planet->world_x + 1000;
				defender->ai.dest_point[2][1] = planet->world_y + 1000;
				defender->ai.dest_point[3][0] = planet->world_x - 1000;
				defender->ai.dest_point[3][1] = planet->world_y + 1000;
				defender->ai.dest_heading = 1;
				defender->ai.home = planets[num_planets];
			}
			/* set the offender list to nothing */
			for (i = 0; i < MAX_TRACK; i++) {
				planets[num_planets]->offenders[i] = NULL;
			}
		}

		num_planets++;

		return (0);
	} else {
		fprintf(stdout, "Too many planets, cannot add anymore.\n");
	}

	return (-1);
}

int load_planets_eaf(FILE *eaf, char *filename) {
	parsed_file *planets_esf = NULL;
	int i, j, k, l;

	planets_esf = esf_new_handle();
	assert(planets_esf);

	if (esf_set_filter(planets_esf, "planet") != 0) {
		printf("Couldn't set parser filter for planets\n");
		return (-1);
	}

	if (esf_parse_file_eaf(eaf, planets_esf, filename) != 0) {
		printf("Couldn't parse file \"%s\"\n", filename);
		return (-1);
	}

	for (i = 0; i < planets_esf->num_items; i++) {
		struct _planet *planet = create_default_planet();
		assert(planet);
		planet->traffic = 0;
		planet->battle_chance = 0; /* defaults */

		for (j = 0; j < planets_esf->items[i].num_keys; j++) {
			char *name = planets_esf->items[i].keys[j].name;

			if (!strcmp(name, "Name")) {
				char *text = planets_esf->items[i].keys[j].value.cp;

				planet->name = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(planet->name, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(planet->name, text);
			} else if (!strcmp(name, "Surface")) {
				char *text = planets_esf->items[i].keys[j].value.cp;

				planet->surface = (char *)malloc(sizeof(char) * (strlen(text) + 1));
				memset(planet->surface, 0, sizeof(char) * (strlen(text) + 1));
				strcpy(planet->surface, text);
			} else if (!strcmp(name, "Image")) {
				char *text = planets_esf->items[i].keys[j].value.cp;

				planet->image = load_image_eaf(eaf, text, BLUE_COLORKEY);
				if (planet->image == NULL) {
					planet->image = load_image_eaf(main_eaf, text, BLUE_COLORKEY);

					if (planet->image == NULL) {
						printf("Could not load planet image\n");
					}
				}
			} else if (!strcmp(name, "Defender Model")) {
				char *text = planets_esf->items[i].keys[j].value.cp;

				planet->defender_model = get_model_pointer(text);
				assert(planet->defender_model);
			} else if (!strcmp(name, "Alliance")) {
				char *text = planets_esf->items[i].keys[j].value.cp;

				planet->alliance = get_alliance_pointer(text);

				if (!planet->alliance) {
					planet->alliance = get_alliance_pointer("Independent");
					assert(planet->alliance);
				}
			} else if (!strcmp(name, "X")) {
				int x = planets_esf->items[i].keys[j].value.i;

				planet->world_x = x;
			} else if (!strcmp(name, "Y")) {
				int y = planets_esf->items[i].keys[j].value.i;

				planet->world_y = y;
			} else if (!strcmp(name, "Traffic")) {
				planet->traffic = planets_esf->items[i].keys[j].value.i;
			} else if (!strcmp(name, "Battle Chance")) {
				planet->battle_chance = planets_esf->items[i].keys[j].value.i;
			} else if (!strcmp(name, "Landable")) {
				int landable = planets_esf->items[i].keys[j].value.i;

				planet->landable = (unsigned char)landable;
			} else if (!strcmp(name, "Defenders")) {
				int num_defenders = planets_esf->items[i].keys[j].value.i;

				planet->num_defenders = num_defenders;
			}
		}

		if (add_planet(planet) != 0) {
			free_planet(planet);
			break;
		}
	}

	if (esf_close_handle(planets_esf) != 0) {
		printf("Couldn't close parser handle for planets\n");
		return (-1);
	}

	return (0);
}

void unload_planets(void) {
	int i;

	for (i = 0; i < num_planets; i++)
		free_planet(planets[i]);

	num_planets = 0;
}

/* function to get a planet pointer based on its name */
struct _planet *get_planet_pointer(char *name) {
	int i;

	if (!name)
		return (NULL);

	for (i = 0; i < num_planets; i++) {
		if (!strcmp(planets[i]->name, name))
			return (planets[i]);
	}

	return (NULL);
}

static struct _planet *create_default_planet(void) {
	struct _planet *planet = NULL;

	planet = (struct _planet *)malloc(sizeof(struct _planet));
	assert(planet != NULL);
	planet->name = 0;
	planet->surface = 0;
	planet->alliance = NULL;
	planet->world_x = 0;
	planet->world_y = 0;
	planet->num_defenders = 0;
	planet->land_type = PLANET;
	planet->comm_type = STANDARD;
	planet->landable = 1;
	planet->image = NULL;
	planet->num_defenders = 0;
	planet->defender_model = models[0];

	return (planet);
}

static void free_planet(struct _planet *planet) {
	assert(planet->image != NULL);
	SDL_FreeSurface(planet->image);
	planet->image = NULL;
	free(planet->name);
	free(planet->surface);
	free(planet);
	planet = NULL;
}

/* returns the closest planet to those coords, -1 on error */
int get_closest_planet(int x, int y) {
	int closest = 0;
	int i;
	float a, dist;

	if (num_planets == 0)
		return (-1);

	dist = get_distance_sqrd(x, y, planets[0]->world_x, planets[0]->world_y);

	for (i = 0; i < num_planets; i++) {
		if ((a = get_distance_sqrd(x, y, planets[i]->world_x, planets[i]->world_y)) < dist) {
			dist = a;
			closest = i;
		}
	}

	return (closest);
}

/* returns a \n separated list of the names of the outfits available on planet 'planet' */
char *get_outfits(char *planet) {
	parsed_file *planets_esf = NULL;
	char *items = NULL;
	int i, j, k, l;
	unsigned char found_planet = 0;

	if ((planets_esf = esf_new_handle()) == NULL) {
		printf("Couldn't create parser handle\n");
		return (NULL);
	}

	if (esf_set_filter(planets_esf, "planet") != 0) {
		printf("Couldn't set parser filter\n");
		esf_close_handle(planets_esf);
		return (NULL);
	}

	if (esf_parse_file_eaf(loaded_eaf, planets_esf, "planets.esf") != 0) {
		printf("Error while parsing file\n");
		esf_close_handle(planets_esf);
		return (NULL);
	}

	for (i = 0; i < planets_esf->num_items; i++) {
		found_planet = 0;

		/* parse subitems only (we only care about outfits, which are subitems) */
		for (j = 0; j < planets_esf->items[i].num_subitems; j++) {
			for (l = 0; l < planets_esf->items[i].num_keys; l++) {
				char *name = planets_esf->items[i].keys[l].name;

				if (!strcmp(name, "Name")) {
					char *pname = planets_esf->items[i].keys[l].value.cp;

					if (!strcmp(pname, planet))
						found_planet = 1;
				}
			}
			for (k = 0; k < planets_esf->items[i].subitems[j].num_keys; k++) {
				char *name = planets_esf->items[i].subitems[j].keys[k].name;

				if (!strcmp(name, "Outfit Item") && found_planet) {
					char *new_outfit = NULL;
					char *text = planets_esf->items[i].subitems[j].keys[k].value.cp;

					/* construct a new string with the old data plus this, free the old string, and make this the old string */
					if (items) {
						new_outfit = (char *)malloc(sizeof(char) * (strlen(items) + strlen(text) + 2));
						memset(new_outfit, 0, sizeof(char) * (strlen(items) + strlen(text) + 2));
						strcpy(new_outfit, items);
					} else {
						new_outfit = (char *)malloc(sizeof(char) * (strlen(text) + 2));
						memset(new_outfit, 0, sizeof(char) * (strlen(text) + 2));
					}
					strcat(new_outfit, text);
					strcat(new_outfit, "\n");

					free(items);
					items = new_outfit;
				}
			}
		}
	}

	if (esf_close_handle(planets_esf) != 0) {
		printf("Couldn't close parser handle\n");
		return (NULL);
	}

	if (items)
		return (items);

	/* didn't find any items (or maybe the planet), so check the main eaf */
	if ((planets_esf = esf_new_handle()) == NULL) {
		printf("Couldn't create parser handle\n");
		return (NULL);
	}

	if (esf_set_filter(planets_esf, "planet") != 0) {
		printf("Couldn't set parser filter\n");
		esf_close_handle(planets_esf);
		return (NULL);
	}

	if (esf_parse_file_eaf(main_eaf, planets_esf, "planets.esf") != 0) {
		printf("Error while parsing file\n");
		esf_close_handle(planets_esf);
		return (NULL);
	}

	for (i = 0; i < planets_esf->num_items; i++) {
		found_planet = 0;

		/* parse subitems only (we only care about outfits, which are subitems) */
		for (j = 0; j < planets_esf->items[i].num_subitems; j++) {
			for (l = 0; l < planets_esf->items[i].num_keys; l++) {
				char *name = planets_esf->items[i].keys[l].name;

				if (!strcmp(name, "Name")) {
					char *pname = planets_esf->items[i].keys[l].value.cp;

					if (!strcmp(pname, planet))
						found_planet = 1;
				}
			}
			for (k = 0; k < planets_esf->items[i].subitems[j].num_keys; k++) {
				char *name = planets_esf->items[i].subitems[j].keys[k].name;

				if (!strcmp(name, "Outfit Item") && found_planet) {
					char *new_outfit = NULL;
					char *text = planets_esf->items[i].subitems[j].keys[k].value.cp;

					/* construct a new string with the old data plus this, free the old string, and make this the old string */
					if (items) {
						new_outfit = (char *)malloc(sizeof(char) * (strlen(items) + strlen(text) + 2));
						memset(new_outfit, 0, sizeof(char) * (strlen(items) + strlen(text) + 2));
						strcpy(new_outfit, items);
					} else {
						new_outfit = (char *)malloc(sizeof(char) * (strlen(text) + 2));
						memset(new_outfit, 0, sizeof(char) * (strlen(text) + 2));
					}
					strcat(new_outfit, text);
					strcat(new_outfit, "\n");

					free(items);
					items = new_outfit;
				}
			}
		}
	}

	if (esf_close_handle(planets_esf) != 0) {
		printf("Couldn't close parser handle\n");
		return (NULL);
	}

	if (items)
		return (items);

	return (NULL);
}

char *get_planet_description(char *planet) {
	char *desc = NULL;
	parsed_file *planets_esf = NULL;
	int i, j;
	unsigned char found_planet = 0;
	
	if (!planet)
		return (NULL);
	
	planets_esf = esf_new_handle();
	
	if (esf_set_filter(planets_esf, "planet") != 0) {
		printf("Could not set filter\n");
		return (NULL);
	}
	
	if (esf_parse_file_eaf(loaded_eaf, planets_esf, "planets.esf") != 0) {
		printf("Couldn't parse \"planets.esf\" in loaded scenario.\n");
	} else {
		for (i = 0; i < planets_esf->num_items; i++) {
			found_planet = 0; /* reset per item */
			for (j = 0; j < planets_esf->items[i].num_keys; j++) {
				if (!strcmp(planets_esf->items[i].keys[j].name, "Name")) {
					if (!strcmp(planets_esf->items[i].keys[j].value.cp, planet))
						found_planet = 1;
				}
				if (!strcmp(planets_esf->items[i].keys[j].name, "Description")) {
					if (found_planet) {
						desc = (char *)malloc(sizeof(char) * (strlen(planets_esf->items[i].keys[j].value.cp) + 1));
						memset(desc, 0, sizeof(char) * (strlen(planets_esf->items[i].keys[j].value.cp) + 1));
						strcpy(desc, planets_esf->items[i].keys[j].value.cp);
						esf_close_handle(planets_esf);
						return (desc);
					}
				}
			}
		}
	}
	
	esf_close_handle(planets_esf);

#ifndef NDEBUG	
	printf("couldnt find description in loaded scenario, giving up\n");
#endif

	return (NULL);
}
