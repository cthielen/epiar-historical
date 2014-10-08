#ifndef _TYPE_H_
#define _TYPE_H_

#include "includes.h"

#define MAX_TYPES            25
#define MAX_WEAPON_SLOTS      8

enum CLASS_TYPE {LIGHT, MEDIUM, HEAVY, CARGO, CAPITOL};

struct _weapon_mount {
	struct _weapon *weapon;
	short int x, y;
	short int angle, range;
	short int default_quantity; /* used by HUD code as maximum quantity, might wanna respect that in other areas as well */
	unsigned char auto_aim;
};

struct _type {
	char *name;
	SDL_Surface *image;
	char *wireframe;
	char *comm_front;
	struct _engine *engine;
	struct _shield *shield;
	struct _weapon_mount *default_mounts[MAX_WEAPON_SLOTS];
	int mass;
	int cargo;
	int hull_life;
	int str;
	int radius;
	long int price;
	int total_fuel;
	enum CLASS_TYPE class_type;
} *types[MAX_TYPES];

extern int num_types;

int load_types_eaf(FILE *eaf, char *filename);
int unload_types(void);
struct _type *get_type_pointer(char *name);
char *get_manufacturer(struct _type *type);
char *get_description(struct _type *type);
void free_weapon_mount(struct _weapon_mount *weapon_mount);
struct _weapon_mount *create_weapon_mount(void);

#endif /* _TYPE_H_ */
