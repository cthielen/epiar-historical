#ifndef _H_WEAPON_
#define _H_WEAPON_

#include "includes.h"

#define MAX_WEAPONS     15
#define MAX_AMMO         5

struct _weapon {
	char *name;
	SDL_Surface *ammo_image;
	int lifetime; /* time it takes to expire in milliseconds */
	int recharge; /* shots per second */
	float velocity; /* pixels per second */
	int strength; /* how much is taken away from the shields each time */
	unsigned char uses_ammo;
	int initial_ammo;
	unsigned char homing;
	char *accepted_ammo;
};

extern struct _weapon *weapons[MAX_WEAPONS];

struct _ammo {
	char *name;
	char *adds_to;
	int quantity;
};

extern struct _ammo *ammos[MAX_AMMO];

extern int num_weapons;
extern int num_ammo;

int load_weapons_eaf(FILE *eaf, char *filename);
int unload_weapons(void);
struct _weapon *get_weapon_pointer(char *name);
struct _ammo *get_ammo_pointer(char *name);
int equip_weapon_player(struct _weapon *weap);

#endif /* _H_WEAPON_ */
