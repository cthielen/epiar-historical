#ifndef H_PLANET
#define H_PLANET

#define MAX_PLANETS             50
#define MAX_TRACK               20
#define MAX_UPGRADES_PER_PLANET 25

#include "alliances/alliances.h"
#include "includes.h"
#include "sprite/model.h"
#include "sprite/upgrade.h"

typedef enum {PLANET, STATION} cbody_land_type; /* celestrial body land type */
typedef enum {STANDARD, RACING} cbody_comm_type;

struct _planet {
	char *name;
	char *surface;
	SDL_Surface *image;
	int world_x, world_y;
	int screen_x, screen_y;
	struct _alliance *alliance;
	struct _ship *offenders[MAX_TRACK]; /* a list of all offenders to a system */
	cbody_land_type land_type;
	unsigned char landable;
	short int num_defenders;
	model_t *defender_model;
	cbody_comm_type comm_type;
	unsigned char revealed;
	struct _upgrade *upgrades[MAX_UPGRADES_PER_PLANET];
	int traffic; /* average traffic when player arrives */
	int battle_chance; /* chance of battle when player arrives */
};

extern struct _planet *planets[MAX_PLANETS];

int load_planets_eaf(FILE *eaf, char *filename);
void unload_planets(void);
struct _planet *get_planet_pointer(char *name);
int get_closest_planet(int x, int y);
char *get_outfits(char *planet);
char *get_planet_description(char *planet);

extern int num_planets;

#endif /* H_PLANET */
