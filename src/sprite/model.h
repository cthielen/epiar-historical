#ifndef _MODEL_H_
#define _MODEL_H_

#include "includes.h"

#define MAX_MODELS           35
#define MAX_WEAPON_SLOTS      8

#define MAX_CACHED_ROTATIONS 36

enum MODEL_TYPE {MODEL_LIGHT, MODEL_MEDIUM, MODEL_HEAVY, MODEL_CARGO, MODEL_CAPITOL};

struct _weapon_mount {
	struct _weapon *weapon;
	short int x, y;
	short int angle, range;
	int max_ammo, ammo;
	Uint32 time;
};

typedef struct _weapon_mount weapon_mount_t;

typedef struct _model {
	char *name;
	SDL_Surface *image;
	SDL_Surface *cached[MAX_CACHED_ROTATIONS];
	/* everytime a cached is used or made, this Uint32 is set to the current time + 10 seconds, if after that time it isnt updated, we free all the caches - we also onnly cache as needed, if we see a ship rotate half way and fly away, we'd only rotate that much */
	Uint32 cache_expiration;
	char *wireframe, *comm_front;
	struct _engine *default_engine;
	struct _shield *default_shield;
	weapon_mount_t *default_mounts[MAX_WEAPON_SLOTS];
	int cargo;
	int hull_life;
	int str;
	int radius;
	long int price;
	int total_fuel;
	enum MODEL_TYPE class;
} model_t;

extern model_t *models[MAX_MODELS];

extern int num_models;

int load_models_eaf(FILE *eaf, char *filename);
int unload_models(void);
model_t *get_model_pointer(char *name);
char *model_get_manufacturer(model_t *model);
char *model_get_description(model_t *model);
void free_weapon_mount(weapon_mount_t *weapon_mount);
weapon_mount_t *create_weapon_mount(void);
weapon_mount_t *copy_weapon_mount(weapon_mount_t *mount);
void clean_models(void);

#endif /* _MODEL_H_ */
